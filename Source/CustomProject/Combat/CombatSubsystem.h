#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Units/Unit.h"
#include "CombatSubsystem.generated.h"

class ATFTGameMode;
// Represents the current phase of the game loop
UENUM(BlueprintType)
enum class EGamePhase : uint8
{
    Prep,    // Player arranges units on the board
    Combat,  // Units auto-battle
    Result   // Round over — showing outcome before next prep
};

// Fired when the game phase changes — UI listens to this
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChanged, EGamePhase, NewPhase);

// Fired when the round number increments
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundChanged, int32, NewRound);

// Fired when combat ends — true if player won, false if lost
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatEnded, bool, bPlayerWon);

UCLASS()
class CUSTOMPROJECT_API UCombatSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    
    UPROPERTY()
    ATFTGameMode* TFTGameMode;
    
    // Current phase of the game loop
    EGamePhase CurrentPhase;

    // -------------------------------------------------------
    // Phase Control
    // -------------------------------------------------------

    // Begins the prep phase — resets units, starts countdown timer
    UFUNCTION(BlueprintCallable, Category="Combat")
    void StartPrepPhase();

    // Begins the combat phase — starts the fight loop
    UFUNCTION(BlueprintCallable, Category="Combat")
    void StartCombatPhase();

    // Returns the current game phase
    EGamePhase GetCurrentPhase() const { return CurrentPhase; }

    // Returns the current round number
    int32 GetCurrentRound() const { return CurrentRound; }

    // Returns seconds remaining in the prep phase
    float GetPrepTimeRemaining() const { return PrepTimeRemaining; }
    
    // Returns seconds remaining in the prep phase
    float GetCombatTimeRemaining() const { return CombatTimeRemaining; }
    
    // Returns seconds remaining in the prep phase
    float GetResultTimeRemaining() const { return ResultTimeRemaining; }

    // -------------------------------------------------------
    // Unit Registration
    // Called when units are placed on the board
    // -------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category="Combat")
    void RegisterPlayerUnit(AUnit* Unit);

    UFUNCTION(BlueprintCallable, Category="Combat")
    void RegisterEnemyUnit(AUnit* Unit);

    // Destroys all enemy actors and clears the enemy array
    void ClearEnemyUnits();

    // -------------------------------------------------------
    // Config
    // -------------------------------------------------------

    // How long the prep phase lasts in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
    float PrepDuration = 15.f;
    
    // How long the prep phase lasts in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
    float CombatDuration = 30.f;
    
    // How long the prep phase lasts in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
    float ResultDuration = 5.f;

    // -------------------------------------------------------
    // Delegates — bind to these to react to game events
    // -------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category="Combat")
    FOnPhaseChanged OnPhaseChanged;

    UPROPERTY(BlueprintAssignable, Category="Combat")
    FOnRoundChanged OnRoundChanged;

    UPROPERTY(BlueprintAssignable, Category="Combat")
    FOnCombatEnded OnCombatEnded;

private:
    // -------------------------------------------------------
    // State
    // -------------------------------------------------------

    // Which round we are on
    int32 CurrentRound;

    // Seconds remaining in prep phase
    float PrepTimeRemaining = 0.f;
    
    // Seconds remaining in combat phase
    float CombatTimeRemaining = 0.f;
    
    // Seconds remaining in result phase
    float ResultTimeRemaining = 0.f;  

    // Units currently registered for combat
    UPROPERTY()
    TArray<AUnit*> PlayerUnits;

    UPROPERTY()
    TArray<AUnit*> EnemyUnits;

    // Timer handles — used to start/stop/clear timers
    FTimerHandle PrepCountdown; // counts down prep phase
    FTimerHandle CombatTick;    // drives the fight loop at 0.1s intervals
    FTimerHandle ResultCountdown; 

    // -------------------------------------------------------
    // Internal Functions
    // -------------------------------------------------------

    // Called every 0.1s during combat — drives unit AI
    void CombatUpdate();

    // Checks if one side has been wiped — triggers EndCombat
    void CheckRoundEnd();

    // Cleans up after combat — grants rewards, starts next prep
    void EndCombat(bool bPlayerWon);
};