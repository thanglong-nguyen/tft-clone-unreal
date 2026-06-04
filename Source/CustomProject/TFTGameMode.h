#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Combat/CombatSubsystem.h"
#include "UI/TFTHUDWidget.h"
#include "Combat/BattlefieldActor.h"
#include "TFTGameMode.generated.h"

class UTraitSubsystem;
class UBoardWidget;

UCLASS()
class CUSTOMPROJECT_API ATFTGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    UPROPERTY()
    UCombatSubsystem* CombatSS;

    UPROPERTY()
    UShopSubsystem* ShopSS;
    
    UPROPERTY()
    UTraitSubsystem* TraitSS; 

    UPROPERTY()
    ATFTPlayerState* PS;
    
    UPROPERTY()
    ABattlefieldActor* Battlefield;
    
    UPROPERTY()
    UBoardWidget* BoardWidget;
    
    UPROPERTY()
    UTFTHUDWidget* HUDWidget;

    TArray<bool> BenchSlots;
    
    UPROPERTY(EditAnywhere, Category="UI")
    TSubclassOf<UBoardWidget> BoardWidgetClass;
    
    UPROPERTY(EditAnywhere, Category="UI")
    TSubclassOf<UUnitStateWidget> StateWidgetClass;
    
    // Assign WBP_HUD here in BP_TFTGameMode
    UPROPERTY(EditAnywhere, Category="UI")
    TSubclassOf<UTFTHUDWidget> HUDWidgetClass;

    // All unit data assets available in the shop pool
    UPROPERTY(EditAnywhere, Category="Setup")
    TArray<UUnitDataAsset*> AvailableUnits;

    // Enemy units spawned each combat round
    UPROPERTY(EditAnywhere, Category="Setup")
    TArray<UUnitDataAsset*> EnemyUnitPool;
    
    int32 GetNextFreeBenchSlot() const; 
    
    FVector GetBenchPosition(int32 Index) const;
    
    void OccupyBenchSlot(int32 Index);
    
    void FreeBenchSlot(int32 Index);
    
    UFUNCTION(BlueprintCallable)
    void ShowMessage(const FString& Message, float Duration = 2.f, 
        FLinearColor Color = FLinearColor::White);
    
    FVector BenchOrigin = FVector(-1200.f, -500.f, 0.f);
private:

    UFUNCTION()
    void StartGame();

    void SetupShop();
    
    // Generates bench slot positions relative to battlefield
    void SetupBenchPositions();

    UFUNCTION()
    void OnPhaseChanged(EGamePhase NewPhase);
};