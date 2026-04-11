#include "Units/Unit.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"

AUnit::AUnit()
{
    PrimaryActorTick.bCanEverTick = true;

    // AI controller so we can use MoveToLocation
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AUnit::BeginPlay()
{
    Super::BeginPlay();
    CurrentHP = MaxHP;
}

void AUnit::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AUnit::InitFromDataAsset()
{
    if (!DataAsset) return;

    UnitName    = DataAsset->UnitName;
    Cost        = DataAsset->Cost;
    MaxHP       = DataAsset->BaseHP;
    CurrentHP   = DataAsset->BaseHP;
    AttackDamage= DataAsset->BaseAttack;
    Armor       = DataAsset->BaseArmor;
    AttackRange = DataAsset->AttackRange;
    AttackSpeed = DataAsset->AttackSpeed;

    GetCharacterMovement()->MaxWalkSpeed = DataAsset->MoveSpeed;

    if (DataAsset->Mesh)
    {
        GetMesh()->SetSkeletalMesh(DataAsset->Mesh);
    }
}

void AUnit::CombatTick(float DeltaTime)
{
    if (IsDead()) return;

    // Tick down attack cooldown
    if (AttackCooldown > 0.f)
    {
        AttackCooldown -= DeltaTime;
    }

    if (!CurrentTarget || CurrentTarget->IsDead())
    {
        SetActorRotation(FRotator::ZeroRotator);
        CurrentTarget = nullptr;
        CurrentState  = EUnitState::Idle;
        return;
    }

    if (IsEnemyInRange())
    {
        // Stop moving, face target, attack
        if (AAIController* AI = Cast<AAIController>(GetController()))
        {
            AI->StopMovement();
        }
        CurrentState = EUnitState::Attacking;
        TryAttack();
    }
    else
    {
        // Chase target
        CurrentState = EUnitState::Moving;
        MoveToTarget();
    }
}

void AUnit::FindAndSetTarget(const TArray<AUnit*>& EnemyUnits)
{
    AUnit* Closest    = nullptr;
    float  BestDistSq = FLT_MAX;

    for (AUnit* Enemy : EnemyUnits)
    {
        if (!Enemy || Enemy->IsDead()) continue;

        float DistSq = FVector::DistSquared(GetActorLocation(), Enemy->GetActorLocation());
        if (DistSq < BestDistSq)
        {
            BestDistSq = DistSq;
            Closest    = Enemy;
        }
    }

    CurrentTarget = Closest;
}

void AUnit::MoveToTarget()
{
    if (!CurrentTarget) return;

    if (AAIController* AI = Cast<AAIController>(GetController()))
    {
        AI->MoveToActor(CurrentTarget, AttackRange * 0.9f);
    }
}

bool AUnit::IsEnemyInRange() const
{
    if (!CurrentTarget) return false;
    float DistSq = FVector::DistSquared(GetActorLocation(), CurrentTarget->GetActorLocation());
    return DistSq <= (AttackRange * AttackRange);
}

void AUnit::TryAttack()
{
    if (AttackCooldown > 0.f) return;

    if (CurrentTarget && !CurrentTarget->IsDead())
    {
        CurrentTarget->ApplyDamage(AttackDamage);
        AttackCooldown = 1.f / AttackSpeed; // reset cooldown
    }
}

void AUnit::ApplyDamage(float RawDamage)
{
    if (IsDead()) return;

    // Simple flat armor reduction
    float Reduced = RawDamage * (100.f / (100.f + Armor));
    CurrentHP     = FMath::Max(0.f, CurrentHP - Reduced);

    if (CurrentHP <= 0.f)
    {
        Die();
    }
}

void AUnit::Die()
{
    CurrentState  = EUnitState::Dead;
    CurrentTarget = nullptr;

    if (AAIController* AI = Cast<AAIController>(GetController()))
    {
        AI->StopMovement();
    }

    // Disable collision so other units don't interact with the corpse
    SetActorEnableCollision(false);

    // For now just hide it — later you'll play a death animation then destroy
    SetActorHiddenInGame(true);
}