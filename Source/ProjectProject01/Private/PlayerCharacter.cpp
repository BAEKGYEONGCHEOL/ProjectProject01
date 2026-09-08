// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

    // 디버그 탑뷰 카메라
    DebugCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Debug Camera"));
    DebugCamera->SetupAttachment(RootComponent);

    DebugCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 5000.0f));
    DebugCamera->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));

    // 게임 시작 때는 원래 Camera 를 사용하고 디버그 카메라는 꺼두기!
    DebugCamera->SetAutoActivate(false);
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 현재 플레이어가 소유한 컨트롤러를 가져온다.
	// APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
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

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bDebugMode)
	{
		UpdateDebugAim();
	}
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

        EnhancedInputComponent->BindAction(IA_DebugCamera, ETriggerEvent::Started, this, &APlayerCharacter::ToggleDebugCamera);
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

    if (bDebugMode)
	{
		// 탑뷰 화면 기준 이동
		const FVector Forward = DebugCamera->GetUpVector().GetSafeNormal2D();
		const FVector Right = DebugCamera->GetRightVector().GetSafeNormal2D();

		AddMovementInput(Forward, MovementVector.X);
		AddMovementInput(Right, MovementVector.Y);

		return;
	}

	AddMovementInput(GetActorForwardVector(), MovementVector.X);
	AddMovementInput(GetActorRightVector(), MovementVector.Y);
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookVector = Value.Get<FVector2D>();

    // 탑뷰에서는 일반 마우스 카메라 회전 금지
	if (bDebugMode)
	{
		return;
	}

	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);
}

void APlayerCharacter::ToggleDebugCamera()
{
    bDebugMode = !bDebugMode;

    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (PlayerController == nullptr)
    {
        return;
    }

    if (bDebugMode)
    {
        Camera->Deactivate();
		DebugCamera->Activate();

		PlayerController->bShowMouseCursor = true;
		PlayerController->SetInputMode(FInputModeGameAndUI());
    }
    else
    {
        DebugCamera->Deactivate();
		Camera->Activate();

		PlayerController->bShowMouseCursor = false;
		PlayerController->SetInputMode(FInputModeGameOnly());
    }
}

void APlayerCharacter::UpdateDebugAim()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController == nullptr)
	{
		return;
	}

	FVector WorldLocation;
	FVector WorldDirection;

	if (!PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
	{
		return;
	}

	if (FMath::Abs(WorldDirection.Z) < 0.001f)
	{
		return;
	}

	// 캐릭터 높이와 만나는 지점 계산
	const float Distance = (GetActorLocation().Z - WorldLocation.Z) / WorldDirection.Z;
	const FVector MouseWorldLocation = WorldLocation + WorldDirection * Distance;
    FVector Direction = MouseWorldLocation - GetActorLocation();

	Direction.Z = 0.0f;

	if (Direction.IsNearlyZero())
	{
		return;
	}

	SetActorRotation(FRotator(0.0f, Direction.Rotation().Yaw, 0.0f));
}