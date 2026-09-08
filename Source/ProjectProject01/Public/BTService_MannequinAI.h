// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_MannequinAI.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPROJECT01_API UBTService_MannequinAI : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTService_MannequinAI();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	// 플레이어의 화면에 Mannequin 이 확인이 되었는지 여부를 저장한다.
	bool PlayerSeeMannequin;

	// Mannequin 이 플레이어를 발견했는지 저장한다.
	bool MannequinSeePlayer;
};
