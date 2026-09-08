// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_MannequinAI.h"

UBTService_MannequinAI::UBTService_MannequinAI()
{
	NodeName = TEXT("Mannequin AI Behavior");

    Interval = 0.0f;
    RandomDeviation = 0.0f;
}

void UBTService_MannequinAI::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    
}