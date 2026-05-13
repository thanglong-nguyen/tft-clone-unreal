#pragma once
#include "CoreMinimal.h"
#include "UnitDataAsset.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Character.h"
#include "UI/UnitStateWidget.h"
#include "Unit.generated.h"

// Represents what the unit is currently doing
// Used by animation blueprint and combat logic
UENUM(BlueprintType)
enum class EUnitState : uint8
{
    Idle,       // Standing still, no target
    Moving,     // Walking toward a target
    Attacking,  // In range, dealing damage
    Dead        // HP reached 0
};

UCLASS()
class CUSTOMPROJECT_API AUnit : public ACharacter
{
    GENERATED_BODY()

public:
    AUnit();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // -------------------------------------------------------
    // Data Asset
    // Assign in editor to define this unit's stats and traits
    // -------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unit")
    TObjectPtr<UUnitDataAsset> DataAsset;

    // Loads all stats and mesh from the assigned DataAsset
    // Called automatically in BeginPlay and ResetForNewRound
    void InitFromDataAsset();

    // -------------------------------------------------------
    // Identity
    // -------------------------------------------------------

    // Display name — matches DataAsset.UnitName
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unit")
    FName UnitName;

    // 1 = base, 2 = two copies merged, 3 = three copies merged
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unit")
    int32 StarLevel = 1;

    // Shop cost tier (1-5) — used by shop pool and rarity table
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unit")
    int32 Cost = 1;

    // -------------------------------------------------------
    // Stats
    // Base values loaded from DataAsset, modified by traits
    // -------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
    float MaxHP = 500.f;

    UPROPERTY(BlueprintReadOnly, Category="Stats")
    float CurrentHP;

    // Damage dealt per attack before armor reduction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
    float AttackDamage = 50.f;

    // Distance in cm at which this unit can attack
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
    float AttackRange = 200.f;

    // How many times per second this unit attacks
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
    float AttackSpeed = 1.0f;

    // Reduces incoming damage — formula: Damage * (100 / (100 + Armor))
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
    float Armor = 20.f;

    // -------------------------------------------------------
    // State
    // -------------------------------------------------------
    
    UPROPERTY(EditAnywhere)
    UWidgetComponent* StateWidgetComp;
    
    UPROPERTY(EditAnywhere, Category="UI")
    TSubclassOf<UUnitStateWidget> StateWidgetClass;
	
    UPROPERTY(EditAnywhere)
    UUnitStateWidget* StateWidget;

    // What the unit is currently doing — read by animation BP
    UPROPERTY(BlueprintReadOnly, Category="Combat")
    EUnitState CurrentState = EUnitState::Idle;

    // The unit this one is currently targeting
    UPROPERTY(BlueprintReadOnly, Category="Combat")
    TObjectPtr<AUnit> CurrentTarget;
    
    
    // Stores which grid cell this unit occupies
    // -1 means not on the grid
    UPROPERTY()
    int32 GridCol = -1;

    UPROPERTY()
    int32 GridRow = -1;

    // -------------------------------------------------------
    // Combat Interface
    // Called by CombatSubsystem each tick
    // -------------------------------------------------------

    // Main combat loop — handles targeting, movement, attacking
    void CombatTick(float DeltaTime);

    // Receives damage after armor reduction
    void ApplyDamage(float RawDamage);

    // Finds the nearest living unit in the given array and sets it as target
    void FindAndSetTarget(const TArray<AUnit*>& EnemyUnits);

    // Moves toward CurrentTarget using AI navigation
    void MoveToTarget();

    // Fires an attack if cooldown has expired
    void TryAttack();

    // Returns true if CurrentTarget is within attack range
    bool IsEnemyInRange() const;

    // Hides the unit and sets state to Dead
    void Die();

    // Returns true if unit state is Dead
    bool IsDead() const { return CurrentState == EUnitState::Dead; }

    // Resets HP, state and visibility for the next round
    void ResetForNewRound();

private:
    // Seconds remaining before this unit can attack again
    // Counts down each CombatTick, reset to 1/AttackSpeed after each attack
    float AttackCooldown = 0.f;
};