// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/World.h"

#if WITH_EDITOR
#include "InputAction.h"
#include "UObject/ConstructorHelpers.h"
#include "SceneView.h"
#include "SceneViewExtension.h"

// SetupView는 GameThread에서 렌더링용 View에만 적용된다.
// SetupViewPoint/SetupViewProjectionMatrix를 변경하지 않아 AI의 투영은 유지된다.
class FProjectProject01TopViewExtension final : public FWorldSceneViewExtension
{
public:
    FProjectProject01TopViewExtension(const FAutoRegister& AutoRegister, APlayerCharacter* InPlayer)
        : FWorldSceneViewExtension(AutoRegister, InPlayer->GetWorld()), Player(InPlayer)
    {
    }

    virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override
    {
        APlayerCharacter* Owner = Player.Get();
        if (!IsValid(Owner) || !Owner->IsLocallyControlled() ||
            InView.bIsSceneCapture || InView.ViewActor.ActorUniqueId != Owner->GetUniqueID())
        {
            return;
        }

        UWorld* PlayerWorld = Owner->GetWorld();
        UCameraComponent* DebugCamera = Owner->DebugTopViewCamera.Get();
        const bool bUseTopView = IsValid(PlayerWorld) && PlayerWorld->WorldType == EWorldType::PIE &&
            Owner->bUseTopViewInPIE && IsValid(DebugCamera);
        if (bUseTopView != bWasTopView)
        {
            InView.bCameraCut = true;
            bWasTopView = bUseTopView;
        }
        if (!bUseTopView)
        {
            return;
        }

        // 렌즈/FOV는 기존 화면 설정을 유지하고 관찰 위치와 방향만 교체한다.
        InView.ViewLocation = DebugCamera->GetComponentLocation();
        InView.ViewRotation = DebugCamera->GetComponentRotation();
        InView.UpdateViewMatrix();
        InView.CullingOrigin = InView.ViewLocation;
        InView.bHasNearClippingPlane =
            InView.ViewMatrices.GetWorldToClip().GetFrustumNearPlane(InView.NearClippingPlane);
    }

private:
    // 프레임 끝까지 렌더러가 확장을 보관해도 Pawn 수명을 연장하지 않는다.
    TWeakObjectPtr<APlayerCharacter> Player;
    bool bWasTopView = false;
};
#endif



// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

#if WITH_EDITORONLY_DATA
    static ConstructorHelpers::FObjectFinder<UInputAction> DebugCameraAction(
        TEXT("/Game/MyProject/Input/IA_Player_DebugCamera.IA_Player_DebugCamera"));
    if (DebugCameraAction.Succeeded())
    {
        IA_Player_DebugCamera = DebugCameraAction.Object;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Missing input action: /Game/MyProject/Input/IA_Player_DebugCamera"));
    }

    DebugTopViewCamera = CreateEditorOnlyDefaultSubobject<UCameraComponent>(TEXT("Debug Top View Camera"));
    if (DebugTopViewCamera)
    {
        DebugTopViewCamera->SetupAttachment(RootComponent);
        DebugTopViewCamera->SetAutoActivate(false);
        DebugTopViewCamera->bUsePawnControlRotation = false;
        DebugTopViewCamera->SetAbsolute(false, true, false);
        DebugTopViewCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 4000.0f));
        DebugTopViewCamera->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
    }
#endif
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

#if WITH_EDITOR
    // PIE를 시작할 때마다 반드시 기본 플레이어 화면에서 시작한다.
    bUseTopViewInPIE = false;
    if (IsValid(GetWorld()) && GetWorld()->WorldType == EWorldType::PIE)
    {
        if (IsValid(DebugTopViewCamera))
        {
            // Blueprint 설정으로 Auto Activate가 바뀌어도 원래 카메라를 유지한다.
            DebugTopViewCamera->Deactivate();
            TopViewExtension = FSceneViewExtensions::NewExtension<FProjectProject01TopViewExtension>(this);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("PIE top view unavailable: missing debug camera on %s."), *GetName());
        }
    }
#endif
	
	// 현재 플레이어가 소유한 컨트롤러를 가져온다.
	//APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	// 이보다 현재 캐릭터가 가지고 있는 컨트롤러를 가져오는 게 좋다.
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	
	// 만일, 플레이어 컨트롤러 변수에 값이 들어 있다면...
	if (PlayerController != nullptr)
	{
		// 플레이어 컨트롤러로부터 입력 서브 시스템 정보를 가져온다.
		UEnhancedInputLocalPlayerSubsystem* Subsystem = 
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

		if (Subsystem != nullptr)
		{
			// 입력 서브 시스템에 IMC 파일 변수를 연결한다.
			Subsystem->AddMappingContext(IMC_PlayerInput, 0);
		}
	}
}

void APlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if WITH_EDITOR
    TopViewExtension.Reset();
#endif
    Super::EndPlay(EndPlayReason);
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (EnhancedInputComponent != nullptr)
	{
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		//EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Completed, this, &APlayerCharacter::Move);
		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);

#if WITH_EDITOR
        if (IsValid(GetWorld()) && GetWorld()->WorldType == EWorldType::PIE)
        {
            if (IsValid(IA_Player_DebugCamera))
            {
                // Started는 누르기 시작할 때 한 번만 발생한다.
                EnhancedInputComponent->BindAction(
                    IA_Player_DebugCamera.Get(), ETriggerEvent::Started, this, &APlayerCharacter::ToggleDebugCamera);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Debug camera input not bound: IA_Player_DebugCamera is missing on %s."), *GetName());
            }
        }
#endif
		//EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Completed, this, &APlayerCharacter::Look);
	}
}

#if WITH_EDITOR
void APlayerCharacter::ToggleDebugCamera()
{
    if (!IsValid(GetWorld()) || GetWorld()->WorldType != EWorldType::PIE || !IsLocallyControlled())
    {
        return;
    }
    if (!IsValid(DebugTopViewCamera) || !TopViewExtension.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Debug camera toggle unavailable on %s."), *GetName());
        return;
    }

    // 표시 화면만 토글하며 원래 플레이어 카메라와 AI 판정은 유지한다.
    bUseTopViewInPIE = !bUseTopViewInPIE;
}
#endif

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	AddMovementInput(GetActorForwardVector(), MovementVector.X);
	AddMovementInput(GetActorRightVector(), MovementVector.Y);
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);
}

