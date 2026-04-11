#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Units/Unit.h"
#include "TATFTPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelUp, int32, NewLevel, int32, BoardCapacity);

UCLASS()
class CUSTOMPROJECT_API ATFTPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    ATFTPlayerState();

    // --- Level & XP ---
    UPROPERTY(BlueprintReadOnly, Category="Level")
    int32 PlayerLevel = 1;

    UPROPERTY(BlueprintReadOnly, Category="Level")
    int32 CurrentXP = 0;

    // XP required to reach each level
    // Index 0 = XP to reach level 2, index 1 = XP to reach level 3 etc
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
    TArray<int32> XPThresholds = {2, 6, 10, 16, 22, 34, 46, 60};

    // Board capacity = PlayerLevel (max 8)
    UFUNCTION(BlueprintCallable, Category="Level")
    int32 GetBoardCapacity() const { return FMath::Min(PlayerLevel, 8); }

    UFUNCTION(BlueprintCallable, Category="Level")
    int32 GetXPToNextLevel() const;

    // Add XP — automatically levels up if threshold met
    UFUNCTION(BlueprintCallable, Category="Level")
    void AddXP(int32 Amount);

    // Buy XP with gold (4 gold = 4 XP, like TFT)
    UFUNCTION(BlueprintCallable, Category="Level")
    bool BuyXP();

    // --- Board & Bench ---
    UPROPERTY(BlueprintReadOnly, Category="Board")
    TArray<TObjectPtr<AUnit>> BoardUnits;

    UPROPERTY(BlueprintReadOnly, Category="Board")
    TArray<TObjectPtr<AUnit>> BenchUnits;

    UFUNCTION(BlueprintCallable, Category="Board")
    bool CanPlaceOnBoard() const;

    UFUNCTION(BlueprintCallable, Category="Board")
    bool MoveToBoard(AUnit* Unit);

    UFUNCTION(BlueprintCallable, Category="Board")
    void MoveToBench(AUnit* Unit);

    // --- Gold (moved here from shop for cleaner access) ---
    UPROPERTY(BlueprintReadOnly, Category="Economy")
    int32 Gold = 0;

    UFUNCTION(BlueprintCallable, Category="Economy")
    void AddGold(int32 Amount);

    UFUNCTION(BlueprintCallable, Category="Economy")
    bool SpendGold(int32 Amount);

    // --- Delegates ---
    UPROPERTY(BlueprintAssignable)
    FOnLevelUp OnLevelUp;

private:
    void CheckLevelUp();

    // XP given per round automatically
    static constexpr int32 XPPerRound = 2;
    // Cost to manually buy XP
    static constexpr int32 BuyXPCost  = 4;
    // XP gained per manual buy
    static constexpr int32 BuyXPAmount = 4;
};