#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Combat/CombatSubsystem.h"
#include "UI/TFTHUDWidget.h"
#include "TFTGameMode.generated.h"

UCLASS()
class CUSTOMPROJECT_API ATFTGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    // Assign WBP_HUD here in BP_TFTGameMode
    UPROPERTY(EditAnywhere, Category="UI")
    TSubclassOf<UTFTHUDWidget> HUDWidgetClass;

    // All unit data assets available in the shop pool
    UPROPERTY(EditAnywhere, Category="Setup")
    TArray<UUnitDataAsset*> AvailableUnits;

    // Enemy units spawned each combat round
    UPROPERTY(EditAnywhere, Category="Setup")
    TArray<UUnitDataAsset*> EnemyUnitPool;

private:
    UPROPERTY()
    UTFTHUDWidget* HUDWidget = nullptr;
    
    UPROPERTY()
    UCombatSubsystem* CombatSS;

    UPROPERTY()
    UShopSubsystem* ShopSS;

    UPROPERTY()
    ATFTPlayerState* PS;

    UFUNCTION()
    void StartGame();

    void SetupShop();

    UFUNCTION()
    void OnPhaseChanged(EGamePhase NewPhase);
};