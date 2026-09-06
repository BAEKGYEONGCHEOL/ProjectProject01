// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PlayerCharacter.generated.h"

class UInputAction;
class UInputMappingContext;

UCLASS()
class PROJECTPROJECT01_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
#if WITH_EDITORONLY_DATA
    // PIE 표시 화면만 탑뷰로 전환한다. AI 시야 카메라는 변경하지 않는다.
    UPROPERTY(VisibleInstanceOnly, Transient, Category = "Debug|Top View")
    bool bUseTopViewInPIE = false;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Player_DebugCamera;

    UPROPERTY(VisibleAnywhere, Category = "Debug|Top View")
    TObjectPtr<class UCameraComponent> DebugTopViewCamera;
#endif

#if WITH_EDITOR
    friend class FProjectProject01TopViewExtension;
    void ToggleDebugCamera();
    TSharedPtr<class FProjectProject01TopViewExtension, ESPMode::ThreadSafe> TopViewExtension;
#endif


	UPROPERTY(VisibleAnywhere, Category = "Component")
	class USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, Category = "Component")
	class UCameraComponent* Camera;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* IMC_PlayerInput;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Move;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Look;

	// 입력 이벤트 발생 시 실행할 함수
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
};