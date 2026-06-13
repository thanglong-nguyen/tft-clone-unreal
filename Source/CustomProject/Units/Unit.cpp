#include "Units/Unit.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

AUnit::AUnit()
{
    PrimaryActorTick.bCanEverTick = true;

    // Spawn an AI controller automatically so we can call MoveToActor
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    
    GetCharacterMovement()->bUseRVOAvoidance = true;
    GetCharacterMovement()->AvoidanceWeight = 0.5f;
    GetCharacterMovement()->SetAvoidanceEnabled(true);

    // Position the mesh inside the capsule correctly
    GetMesh()->SetupAttachment(GetCapsuleComponent());
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
    
    StateWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("StateWidget"));
    StateWidgetComp->AttachToComponent(RootComponent, 
        FAttachmentTransformRules::KeepRelativeTransform);
    StateWidgetComp->SetWidgetSpace(EWidgetSpace::Screen); // always faces camera
    StateWidgetComp->SetDrawSize(FVector2D(150.f, 40.f));
    
    StateWidgetComp->SetupAttachment(GetMesh(), FName("head"));

    // Add a small upward offset so it sits *above* the skull, not inside it
    StateWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 30.f)); 
}

void AUnit::BeginPlay()
{
    Super::BeginPlay();

    // Load stats and mesh from the assigned data asset
    if (DataAsset)
    {
        InitFromDataAsset();
    }

    // Fallback HP in case no data asset is assigned
    CurrentHP = MaxHP;
}

void AUnit::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AUnit::InitFromDataAsset()
{
    if (!DataAsset) return;

    UnitName = DataAsset->UnitName;
    Cost     = DataAsset->Cost;

    // Load stats from the correct star tier — fallback to tier 1 if missing
    const FUnitStarTier* Tier = DataAsset->GetStarTier(StarLevel);
    if (!Tier) Tier = DataAsset->GetStarTier(1);
    if (!Tier) return;

    MaxHP        = Tier->BaseHP;
    CurrentHP    = Tier->BaseHP;
    AttackDamage = Tier->BaseAttack;
    Armor        = Tier->BaseArmor;
    AttackRange  = Tier->AttackRange;
    AttackSpeed  = Tier->AttackSpeed;

    GetCharacterMovement()->MaxWalkSpeed = DataAsset->MoveSpeed;

    if (DataAsset->Mesh && GetMesh())
    {
        GetMesh()->SetSkeletalMesh(DataAsset->Mesh);
        GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
        GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
    }
}

void AUnit::SetStateWidget()
{
    if (StateWidgetComp && StateWidgetClass)
    {
        StateWidgetComp->SetWidgetClass(StateWidgetClass);
        
        StateWidget = Cast<UUnitStateWidget>(StateWidgetComp->GetUserWidgetObject());
        
        if (StateWidget)
        {
            StateWidget->Owner = this;
            StateWidget->UpdateValues();
        }
    }
}

// -------------------------------------------------------
// Combat Loop
// -------------------------------------------------------

void AUnit::CombatTick(float DeltaTime)
{
    if (IsDead()) return;

    // Count down attack cooldown each tick
    if (AttackCooldown > 0.f)
    {
        AttackCooldown -= DeltaTime;
    }

    // If target is gone or dead, go idle and wait for retargeting
    if (!CurrentTarget || CurrentTarget->IsDead())
    {
        SetActorRotation(FRotator::ZeroRotator);
        CurrentTarget = nullptr;
        CurrentState  = EUnitState::Idle;
        return;
    }

    if (IsEnemyInRange())
    {
        // In range — stop moving and attack
        if (AAIController* AI = Cast<AAIController>(GetController()))
        {
            AI->StopMovement();
        }
        CurrentState = EUnitState::Attacking;
        TryAttack();
    }
    else
    {
        // Out of range — chase the target
        CurrentState = EUnitState::Moving;
        MoveToTarget();
    }
    
    if (StateWidget)
        StateWidget->UpdateValues();
}

void AUnit::FindAndSetTarget(const TArray<AUnit*>& EnemyUnits)
{
    AUnit* Closest    = nullptr;
    float  BestDistSq = FLT_MAX;

    // Find the nearest living enemy
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
        // Stop slightly before reaching attack range
        AI->MoveToActor(CurrentTarget, AttackRange * 0.5f);
    }
}

bool AUnit::IsEnemyInRange() const
{
    if (!CurrentTarget) return false;

    float DistSq = FVector::DistSquared(
        GetActorLocation(),
        CurrentTarget->GetActorLocation()
    );

    // Add tolerance buffer to account for navmesh imprecision
    float RangeWithTolerance = AttackRange + 50.f;
    return DistSq <= (RangeWithTolerance * RangeWithTolerance);
}

void AUnit::TryAttack()
{
    // Wait for cooldown before attacking again
    if (AttackCooldown > 0.f) return;

    if (CurrentTarget && !CurrentTarget->IsDead())
    {
        CurrentTarget->ApplyDamage(AttackDamage);

        // Reset cooldown based on attack speed
        // AttackSpeed = 1.0 → 1 attack/sec, 2.0 → 2 attacks/sec
        AttackCooldown = 1.f / AttackSpeed;
    }
}

void AUnit::ApplyDamage(float RawDamage)
{
    if (IsDead()) return;

    // Flat armor damage reduction formula
    // Higher armor = less damage taken, but never 0
    float Reduced = RawDamage * (100.f / (100.f + Armor));
    CurrentHP     = FMath::Max(0.f, CurrentHP - Reduced);

    if (CurrentHP <= 0.f)
    {
        Die();
    }
}

void AUnit::Die()
{
    CurrentState   = EUnitState::Dead;
    CurrentTarget  = nullptr;
    CurrentHP      = 0.f;
    AttackCooldown = 999.f; // prevent any pending attacks from firing

    if (AAIController* AI = Cast<AAIController>(GetController()))
    {
        AI->StopMovement();
    }

    // Hide the unit — it stays in memory for potential respawn next round
    SetActorEnableCollision(false);
    SetActorHiddenInGame(true);
}

void AUnit::ResetForNewRound()
{
    // Restore base stats from data asset (clears any leftover damage)
    InitFromDataAsset();

    // Reset combat state
    CurrentState   = EUnitState::Idle;
    CurrentTarget  = nullptr;
    AttackCooldown = 0.f;

    // Make the unit visible and collidable again
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    
    if (StateWidget)
        StateWidget->UpdateValues();
}