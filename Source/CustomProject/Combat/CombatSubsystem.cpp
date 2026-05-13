#include "Combat/CombatSubsystem.h"
#include "Units/Unit.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Player/TFTPlayerState.h"
#include "TFTGameMode.h"

void UCombatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    CurrentPhase = EGamePhase::Prep;
    CurrentRound = 1;
}

// -------------------------------------------------------
// Unit Registration
// -------------------------------------------------------

void UCombatSubsystem::RegisterPlayerUnit(AUnit* Unit)
{
    // Ignore null units or duplicates
    if (!Unit || PlayerUnits.Contains(Unit)) return;
    PlayerUnits.Add(Unit);
}

void UCombatSubsystem::RegisterEnemyUnit(AUnit* Unit)
{
    if (!Unit || EnemyUnits.Contains(Unit)) return;
    EnemyUnits.Add(Unit);
}

void UCombatSubsystem::ClearEnemyUnits()
{
    // Destroy each enemy actor then empty the array
    for (AUnit* Unit : EnemyUnits)
    {
        if (!Unit) continue;
        if (TFTGameMode->Battlefield && Unit->GridCol >= 0)
            TFTGameMode->Battlefield->FreeEnemyCell(Unit->GridCol, Unit->GridRow);
        Unit->Destroy();
    }
    EnemyUnits.Empty();
}

// -------------------------------------------------------
// Phase Control
// -------------------------------------------------------

void UCombatSubsystem::StartPrepPhase()
{
    // Clear all timers to prevent conflicts
    GetWorld()->GetTimerManager().ClearTimer(ResultCountdown);
    GetWorld()->GetTimerManager().ClearTimer(CombatTick);
    GetWorld()->GetTimerManager().ClearTimer(PrepCountdown);
    
    // Set state
    CurrentPhase      = EGamePhase::Prep;
    PrepTimeRemaining = PrepDuration;
    
    // Notify listeners (UI updates to show prep screen)
    OnPhaseChanged.Broadcast(EGamePhase::Prep);
    
    // Reset all player units to full HP for the new round
    for (AUnit* Unit : PlayerUnits)
    {
        if (!Unit) continue;

        Unit->ResetForNewRound();

        if (TFTGameMode->Battlefield && Unit->GridCol >= 0)
        {
            FVector Position = TFTGameMode->Battlefield->GetPlayerCellPosition(
                Unit->GridCol, Unit->GridRow);
            Unit->SetActorLocation(Position);
        }
    }

    // Count down every second — when it hits 0 start combat
    GetWorld()->GetTimerManager().SetTimer(PrepCountdown, [this]()
    {
        PrepTimeRemaining -= 1.f;

        if (PrepTimeRemaining <= 0.f)
        {
            StartCombatPhase();
        }

    }, 1.f, true);
}

void UCombatSubsystem::StartCombatPhase()
{
    // Stop the prep countdown
    GetWorld()->GetTimerManager().ClearTimer(PrepCountdown);
    
    CombatTimeRemaining = CombatDuration;

    CurrentPhase = EGamePhase::Combat;
    OnPhaseChanged.Broadcast(EGamePhase::Combat);
    
    // Move enemies from off-screen to their grid positions
    for (AUnit* Unit : EnemyUnits)
    {
        if (!Unit) continue;
        if (TFTGameMode->Battlefield && Unit->GridCol >= 0)
        {
            Unit->SetActorLocation(
                TFTGameMode->Battlefield->GetEnemyCellPosition(Unit->GridCol, Unit->GridRow));
        }
    }

    // Start the fight loop — CombatUpdate fires every 0.1s
    GetWorld()->GetTimerManager().SetTimer(CombatTick, [this]()
    {
        CombatTimeRemaining -= 0.1f;
        CombatUpdate();

    }, 0.1f, true);
}

// -------------------------------------------------------
// Fight Loop
// -------------------------------------------------------

void UCombatSubsystem::CombatUpdate()
{
    // Safety check — should always be Combat here but just in case
    if (CurrentPhase != EGamePhase::Combat) return;

    // Tick all living player units
    for (AUnit* Unit : PlayerUnits)
    {
        if (!Unit || Unit->IsDead()) continue;

        // If no target, find the nearest enemy
        if (Unit->CurrentTarget == nullptr)
        {
            Unit->FindAndSetTarget(EnemyUnits);
        }

        // Run movement and attack logic
        Unit->CombatTick(0.1f);
    }

    // Tick all living enemy units
    for (AUnit* Unit : EnemyUnits)
    {
        if (!Unit || Unit->IsDead()) continue;

        if (Unit->CurrentTarget == nullptr)
        {
            Unit->FindAndSetTarget(PlayerUnits);
        }

        Unit->CombatTick(0.1f);
    }

    // Check if the round is over after each tick
    CheckRoundEnd();
}

void UCombatSubsystem::CheckRoundEnd()
{
    // Count how many units are still alive on each side
    int32 LivingPlayers = 0;
    int32 LivingEnemies = 0;

    for (AUnit* Unit : PlayerUnits)
        if (Unit && !Unit->IsDead()) LivingPlayers++;

    for (AUnit* Unit : EnemyUnits)
        if (Unit && !Unit->IsDead()) LivingEnemies++;

    // Both sides wiped — treat as a loss
    if (LivingPlayers == 0 && LivingEnemies == 0)
        EndCombat(false);

    // All enemies dead — player wins
    else if (LivingEnemies == 0)
        EndCombat(true);

    // All player units dead — player loses
    else if (LivingPlayers == 0)
        EndCombat(false);
    
    else if (CombatTimeRemaining<=0)
        EndCombat(false);

    // Otherwise fight is still going — do nothing
}

void UCombatSubsystem::EndCombat(bool bPlayerWon)
{
    // Stop the fight loop
    GetWorld()->GetTimerManager().ClearTimer(CombatTick);
    
    ResultTimeRemaining = ResultDuration;

    // Update phase and notify listeners
    CurrentPhase = EGamePhase::Result;
    OnPhaseChanged.Broadcast(EGamePhase::Result);
    OnCombatEnded.Broadcast(bPlayerWon);

    // Advance round counter
    CurrentRound++;
    OnRoundChanged.Broadcast(CurrentRound);

    // Clean up enemies — new ones spawn next combat phase
    ClearEnemyUnits();

    // Grant rewards to the player
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (ATFTPlayerState* PS = PC->GetPlayerState<ATFTPlayerState>())
        {
            PS->AddGold(10);
            PS->AddXP(4);
        }
    }
    
    GetWorld()->GetTimerManager().SetTimer( ResultCountdown, [this]()
    {
        ResultTimeRemaining -= 1.f;

        if (ResultTimeRemaining <= 0.f)
        {
            StartPrepPhase();
        }

    }, 1.f, true);
}