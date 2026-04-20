#include "Combat/CombatSubsystem.h"
#include "Units/Unit.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Player/TFTPlayerState.h"

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
        if (Unit) Unit->Destroy();
    }
    EnemyUnits.Empty();
}

// -------------------------------------------------------
// Phase Control
// -------------------------------------------------------

void UCombatSubsystem::StartPrepPhase()
{
    // Reset all player units to full HP for the new round
    for (AUnit* Unit : PlayerUnits)
    {
        if (Unit) Unit->ResetForNewRound();
    }

    // Set state
    CurrentPhase      = EGamePhase::Prep;
    PrepTimeRemaining = PrepDuration;

    // Stop the combat tick if it was still running
    GetWorld()->GetTimerManager().ClearTimer(CombatTick);

    // Notify listeners (UI updates to show prep screen)
    OnPhaseChanged.Broadcast(EGamePhase::Prep);

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

    CurrentPhase = EGamePhase::Combat;
    OnPhaseChanged.Broadcast(EGamePhase::Combat);

    // Start the fight loop — CombatUpdate fires every 0.1s
    GetWorld()->GetTimerManager().SetTimer(CombatTick, [this]()
    {
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

    // Otherwise fight is still going — do nothing
}

void UCombatSubsystem::EndCombat(bool bPlayerWon)
{
    // Stop the fight loop
    GetWorld()->GetTimerManager().ClearTimer(CombatTick);

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

    // Wait 3 seconds on the result screen then start next prep phase
    GetWorld()->GetTimerManager().SetTimer(
        PrepCountdown,
        this,
        &UCombatSubsystem::StartPrepPhase,
        3.f,
        false
    );
}