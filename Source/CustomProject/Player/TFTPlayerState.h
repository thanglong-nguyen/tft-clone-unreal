#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TFTPlayerState.generated.h"

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
    TArray<int32> XPThresholds = {2, 6, 10, 16, 22, 34, 46, 60};
    
    UFUNCTION(BlueprintCallable) 
    int32 GetBoardCapacity() const;
    
    UFUNCTION(BlueprintCallable)
    int32 GetXPToNextLevel() const;
    
    void AddXP(int32 amount);
    
    bool BuyXP();
    

    // --- Board & Bench ---
    UPROPERTY(BlueprintReadOnly, Category="Board")
    TArray<TObjectPtr<class AUnit>> BoardUnits;

    UPROPERTY(BlueprintReadOnly, Category="Board")
    TArray<TObjectPtr<class AUnit>> BenchUnits;

    bool CanPlaceOnBoard() const;
    
    bool MoveToBoard(AUnit* Unit);
    
    void MoveToBench(AUnit* Unit);

    // --- Gold ---
    UPROPERTY(BlueprintReadOnly, Category="Economy")
    int32 Gold = 0;
    
    void AddGold(int32 Amount);
    
    bool SpendGold(int32 Amount);

    // --- Delegates ---
    UPROPERTY(BlueprintAssignable)
    FOnLevelUp OnLevelUp;

private:
    static constexpr int32 MaxLevel  = 8;
    static constexpr int32 GoldForXP = 4;
    void CheckLevelUp();
};