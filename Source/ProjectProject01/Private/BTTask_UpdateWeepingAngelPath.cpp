// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_UpdateWeepingAngelPath.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"

#include "WeepingAngelCharacter.h"
#include "WeepingAngelPath.h"

UBTTask_UpdateWeepingAngelPath::UBTTask_UpdateWeepingAngelPath()
{
	NodeName = TEXT("Update Current Weeping Angel Path");
}

EBTNodeResult::Type UBTTask_UpdateWeepingAngelPath::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

    // Blackboard 가져오기
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!IsValid(Blackboard))
    {
        return EBTNodeResult::Failed;
    }

    // AI Controller 가져오기
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!IsValid(AIController))
    {
        return EBTNodeResult::Failed;
    }

    // AI Controller가 조종하는 천사 가져오기
    AWeepingAngelCharacter* Angel = Cast<AWeepingAngelCharacter>(AIController->GetPawn());
    if (!IsValid(Angel))
    {
        return EBTNodeResult::Failed;
    }

    // Blackboard에서 다음 Path 가져오기
    AWeepingAngelPath* NextPath = Cast<AWeepingAngelPath>(Blackboard->GetValueAsObject(TEXT("NextPath")));
    if (!IsValid(NextPath))
    {
        return EBTNodeResult::Failed;
    }

    const bool bApproachSegment = Angel->IsFollowingApproachSegment();
    const FVector MoveDestination = Blackboard->GetValueAsVector(TEXT("NextPathLocation"));

    // MoveTo can report success at the end of a partial path.
    // Match the current BT MoveTo acceptance radius (100 cm + agent radius).
    UPathFollowingComponent* Following = AIController->GetPathFollowingComponent();
    if (Blackboard->GetValueAsBool(TEXT("CanDirectChase")) || !IsValid(Following) ||
        !Blackboard->IsVectorValueSet(TEXT("NextPathLocation")) ||
        !Following->HasReached(MoveDestination, EPathFollowingReachMode::OverlapAgent, 100.0f) ||
        (!bApproachSegment && !Following->HasReached(NextPath->GetAngelPathLocation(),
            EPathFollowingReachMode::OverlapAgent, 100.0f)))
    {
        UE_LOG(LogTemp, Warning, TEXT("Ignoring unreached corridor: %s -> %s"),
            *Angel->GetName(), *NextPath->GetName());
        Angel->SetFollowingApproachSegment(false);
        Blackboard->ClearValue(TEXT("NextPath"));
        Blackboard->ClearValue(TEXT("NextPathLocation"));
        return EBTNodeResult::Failed;
    }

    Angel->SetFollowingApproachSegment(false);
    if (!bApproachSegment)
    {
        AWeepingAngelPath* PreviousPath = Angel->GetCurrentPath();
        Angel->SetCurrentPath(NextPath);
        Blackboard->SetValueAsObject(TEXT("CurrentPath"), NextPath);
        if (NextPath == Angel->GetAssignedApproachPath())
        {
            Angel->SetApproachPathReached(true);
        }
        UE_LOG(LogTemp, Log, TEXT("CurrentPath Updated: %s -> %s"),
            *GetNameSafe(PreviousPath), *NextPath->GetName());
    }
    // An approach segment advances along NavMesh, without claiming a different corridor was reached.

    // 도착한 NextPath는 더 이상 다음 목적지가 아님
    Blackboard->ClearValue(TEXT("NextPath"));
    Blackboard->ClearValue(TEXT("NextPathLocation"));

    return EBTNodeResult::Succeeded;
}