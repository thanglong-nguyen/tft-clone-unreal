#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Combat/CombatSubsystem.h"
#include "UI/TFTHUDWidget.h"
#include "Combat/BattlefieldActor.h"
#include "TFTGameMode.generated.h"

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
    ATFTPlayerState* PS;
    
    UPROPERTY()
    ABattlefieldActor* Battlefield;
    
    UPROPERTY()
    UBoardWidget* BoardWidget;
    
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

private:
    UPROPERTY()
    UTFTHUDWidget* HUDWidget = nullptr;

    UFUNCTION()
    void StartGame();

    void SetupShop();

    UFUNCTION()
    void OnPhaseChanged(EGamePhase NewPhase);
};