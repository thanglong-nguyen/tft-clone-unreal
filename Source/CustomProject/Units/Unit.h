#pragma once
#include "CoreMinimal.h"
#include "UnitDataAsset.h"
#include "GameFramework/Character.h"
#include "Unit.generated.h"

UENUM(BlueprintType)
enum class EUnitState : uint8
{
    Idle,
    Moving,
    Attacking,
    Dead
};

UCLASS()
class CUSTOMPROJECT_API AUnit : public ACharacter
{
    GENERATED_BODY()

public:
    AUnit();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unit")
    TObjectPtr<UUnitDataAsset> DataAsset;
    
    void InitFromDataAsset();

    // --- Identity ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unit")
    FName UnitName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unit")
    int32 StarLevel = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Unit")
    int32 Cost = 1;

    // --- Stats ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
    float MaxHP = 500.f;

    UPROPERTY(BlueprintReadOnly, Category="Stats")
    float CurrentHP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
    float AttackDamage = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
    float AttackRange = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
    float AttackSpeed = 1.0f; // attacks per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
    float Armor = 20.f;

    // --- State ---
    UPROPERTY(BlueprintReadOnly, Category="Combat")
    EUnitState CurrentState = EUnitState::Idle;

    UPROPERTY(BlueprintReadOnly, Category="Combat")
    TObjectPtr<AUnit> CurrentTarget;

    // --- Combat ---
    void ApplyDamage(float RawDamage);
    void FindAndSetTarget(const TArray<AUnit*>& EnemyUnits);
    void MoveToTarget();
    void TryAttack();
    bool IsEnemyInRange() const;
    void Die();

    bool IsDead() const { return CurrentState == EUnitState::Dead; }
    
    void ResetForNewRound();

    // Called every frame by CombatSubsystem during combat phase
    void CombatTick(float DeltaTime);

private:
    float AttackCooldown = 0.f; // counts down to 0 before next attack
};