#include "Combat/CombatSubsystem.h"
#include "Units/Unit.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Player/TATFTPlayerState.h"

void UCombatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    CurrentPhase = EGamePhase::Prep;
    CurrentRound = 1;
}

void UCombatSubsystem::RegisterPlayerUnit(AUnit* Unit)
{
    if (Unit && !PlayerUnits.Contains(Unit))
        PlayerUnits.Add(Unit);
}

void UCombatSubsystem::UnregisterPlayerUnit(AUnit* Unit)
{
    PlayerUnits.Remove(Unit);
}

void UCombatSubsystem::RegisterEnemyUnit(AUnit* Unit)
{
    if (Unit && !EnemyUnits.Contains(Unit))
        EnemyUnits.Add(Unit);
}

void UCombatSubsystem::ClearEnemyUnits()
{
    for (AUnit* Enemy : EnemyUnits)
    {
        if (Enemy) Enemy->Destroy();
    }
    EnemyUnits.Empty();
}

void UCombatSubsystem::StartPrepPhase()
{
    CurrentPhase     = EGamePhase::Prep;
    PrepTimeRemaining = PrepDuration;

    // Stop combat tick if running
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CombatTickHandle);

        // Count down prep timer
        World->GetTimerManager().SetTimer(
            PrepTimerHandle,
            [this]()
            {
                PrepTimeRemaining -= 1.f;
                if (PrepTimeRemaining <= 0.f)
                {
                    StartCombatPhase();
                }
            },
            1.f,   // fires every 1 second
            true   // looping
        );
    }

    OnPhaseChanged.Broadcast(EGamePhase::Prep);

    UE_LOG(LogTemp, Log, TEXT("--- Prep Phase Started | Round %d ---"), CurrentRound);
}

void UCombatSubsystem::StartCombatPhase()
{
    // Stop prep timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PrepTimerHandle);
    }

    CurrentPhase = EGamePhase::Combat;
    OnPhaseChanged.Broadcast(EGamePhase::Combat);

    // Give every unit a target before the first tick
    for (AUnit* PlayerUnit : PlayerUnits)
    {
        if (PlayerUnit && !PlayerUnit->IsDead())
        {
            TArray<AUnit*> Enemies;
            for (auto& E : EnemyUnits) 
                if (E) Enemies.Add(E);
            PlayerUnit->FindAndSetTarget(Enemies);
        }
    }

    for (AUnit* EnemyUnit : EnemyUnits)
    {
        if (EnemyUnit && !EnemyUnit->IsDead())
        {
            TArray<AUnit*> Players;
            for (auto& P : PlayerUnits) 
                if (P) Players.Add(P);
            EnemyUnit->FindAndSetTarget(Players);
        }
    }

    // Start combat tick — runs every 0.1s
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            CombatTickHandle,
            this,
            &UCombatSubsystem::CombatUpdate,
            0.1f,  // tick every 100ms
            true   // looping
        );
    }

    UE_LOG(LogTemp, Log, TEXT("--- Combat Phase Started | Round %d ---"), CurrentRound);
}

void UCombatSubsystem::CombatUpdate()
{
    if (CurrentPhase != EGamePhase::Combat) return;

    CleanDeadUnits();

    // Tick all living player units
    for (AUnit* Unit : PlayerUnits)
    {
        if (Unit && !Unit->IsDead())
        {
            // Retarget if current target is dead
            if (!Unit->CurrentTarget || Unit->CurrentTarget->IsDead())
            {
                TArray<AUnit*> Enemies;
                for (auto& E : EnemyUnits) 
                    if (E && !E->IsDead()) Enemies.Add(E);
                Unit->FindAndSetTarget(Enemies);
            }
            Unit->CombatTick(0.1f);
        }
    }

    // Tick all living enemy units
    for (AUnit* Unit : EnemyUnits)
    {
        if (Unit && !Unit->IsDead())
        {
            if (!Unit->CurrentTarget || Unit->CurrentTarget->IsDead())
            {
                TArray<AUnit*> Players;
                for (auto& P : PlayerUnits) 
                    if (P && !P->IsDead()) Players.Add(P);
                Unit->FindAndSetTarget(Players);
            }
            Unit->CombatTick(0.1f);
        }
    }

    CheckRoundEnd();
}

void UCombatSubsystem::CheckRoundEnd()
{
    // Count living units on each side
    int32 LivingPlayers = 0;
    int32 LivingEnemies = 0;

    for (AUnit* U : PlayerUnits)
        if (U && !U->IsDead()) LivingPlayers++;

    for (AUnit* U : EnemyUnits)
        if (U && !U->IsDead()) LivingEnemies++;

    if (LivingPlayers == 0 && LivingEnemies == 0)
    {
        EndCombat(false); // draw — treat as loss
    }
    else if (LivingEnemies == 0)
    {
        EndCombat(true);  // player wins
    }
    else if (LivingPlayers == 0)
    {
        EndCombat(false); // player loses
    }
}

void UCombatSubsystem::EndCombat(bool bPlayerWon)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CombatTickHandle);
    }

    CurrentPhase = EGamePhase::Result;
    OnPhaseChanged.Broadcast(EGamePhase::Result);
    OnCombatEnded.Broadcast(bPlayerWon);

    UE_LOG(LogTemp, Log, TEXT("--- Combat Ended | Player %s | Round %d ---"),
        bPlayerWon ? TEXT("WON") : TEXT("LOST"), CurrentRound);

    // Advance round and restart prep after 3 seconds
    CurrentRound++;
    OnRoundChanged.Broadcast(CurrentRound);
    
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (ATFTPlayerState* PS = PC->GetPlayerState<ATFTPlayerState>())
        {
            PS->AddXP(2); // 2 XP per round automatically
            
            // Also grant round income gold
            int32 RoundIncome = 5; // base income
            PS->AddGold(RoundIncome);

            UE_LOG(LogTemp, Log, TEXT("Granted 2 XP and %d gold"), RoundIncome);
        }
    }   

    ClearEnemyUnits();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PrepTimerHandle,
            this,
            &UCombatSubsystem::StartPrepPhase,
            3.f,   // 3 second result screen
            false  // not looping
        );
    }
}

void UCombatSubsystem::CleanDeadUnits()
{
    PlayerUnits.RemoveAll([](const TObjectPtr<AUnit>& U)
    {
        return !U || U->IsDead();
    });

    EnemyUnits.RemoveAll([](const TObjectPtr<AUnit>& U)
    {
        return !U || U->IsDead();
    });
}