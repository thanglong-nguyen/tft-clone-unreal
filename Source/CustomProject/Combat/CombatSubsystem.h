#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CombatSubsystem.generated.h"

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
    Prep,    // Player arranges units
    Combat,  // Units fight
    Result   // Round over, show outcome
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChanged, EGamePhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundChanged, int32, RoundNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatEnded, bool, bPlayerWon);

UCLASS()
class CUSTOMPROJECT_API UCombatSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // --- Phase Control ---
    UFUNCTION(BlueprintCallable, Category="Combat")
    void StartPrepPhase();

    UFUNCTION(BlueprintCallable, Category="Combat")
    void StartCombatPhase();

    UFUNCTION(BlueprintCallable, Category="Combat")
    EGamePhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintCallable, Category="Combat")
    int32 GetCurrentRound() const { return CurrentRound; }

    // --- Unit Registration ---
    // Call these when units are placed/removed from board
    UFUNCTION(BlueprintCallable, Category="Combat")
    void RegisterPlayerUnit(AUnit* Unit);

    UFUNCTION(BlueprintCallable, Category="Combat")
    void UnregisterPlayerUnit(AUnit* Unit);

    // Enemy units — spawned automatically each round
    UFUNCTION(BlueprintCallable, Category="Combat")
    void RegisterEnemyUnit(AUnit* Unit);

    void ClearEnemyUnits();

    // --- Delegates ---
    UPROPERTY(BlueprintAssignable)
    FOnPhaseChanged OnPhaseChanged;

    UPROPERTY(BlueprintAssignable)
    FOnRoundChanged OnRoundChanged;

    UPROPERTY(BlueprintAssignable)
    FOnCombatEnded OnCombatEnded;

    // --- Prep Timer ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
    float PrepDuration = 30.f; // seconds per prep phase

    UFUNCTION(BlueprintCallable, Category="Combat")
    float GetPrepTimeRemaining() const { return PrepTimeRemaining; }

private:
    EGamePhase CurrentPhase  = EGamePhase::Prep;
    int32      CurrentRound  = 1;
    float      PrepTimeRemaining = 0.f;

    UPROPERTY()
    TArray<TObjectPtr<AUnit>> PlayerUnits;

    UPROPERTY()
    TArray<TObjectPtr<AUnit>> EnemyUnits;

    FTimerHandle PrepTimerHandle;
    FTimerHandle CombatTickHandle;

    void CombatUpdate();
    void CheckRoundEnd();
    void EndCombat(bool bPlayerWon);

    // Removes dead units from arrays
    void CleanDeadUnits();
};