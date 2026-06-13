#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Combat/CombatSubsystem.h"
#include "UI/TFTHUDWidget.h"
#include "Combat/BattlefieldActor.h"
#include "TFTGameMode.generated.h"

class UTraitSubsystem;
class UBoardWidget;

// Central hub of the game that owns and pipes all systems together.
// All subsystems, UI, and player state are cached here on start.
// Every other class accesses what it needs through TFTGameMode.
UCLASS()
class CUSTOMPROJECT_API ATFTGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    // -------------------------------------------------------
    // Cached System References — set once in StartGame
    // -------------------------------------------------------

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

    // -------------------------------------------------------
    // Bench Slot Tracking
    // Index = slot number, value = occupied
    // -------------------------------------------------------

    TArray<bool> BenchSlots;

    // World origin of the bench — slots extend along Y axis from here
    FVector BenchOrigin = FVector(-1200.f, -500.f, 0.f);

    // Returns index of the first free bench slot, -1 if full
    int32 GetNextFreeBenchSlot() const;

    // Returns world position of a bench slot by index
    FVector GetBenchPosition(int32 Index) const;

    void OccupyBenchSlot(int32 Index);
    void FreeBenchSlot(int32 Index);

    // -------------------------------------------------------
    // Blueprint Assignable Classes — set in BP_TFTGameMode_New
    // -------------------------------------------------------

    UPROPERTY(EditAnywhere, Category="UI")
    TSubclassOf<UBoardWidget> BoardWidgetClass;

    UPROPERTY(EditAnywhere, Category="UI")
    TSubclassOf<UUnitStateWidget> StateWidgetClass;

    UPROPERTY(EditAnywhere, Category="UI")
    TSubclassOf<UTFTHUDWidget> HUDWidgetClass;

    // Units available in the shop pool, add more units here. 
    UPROPERTY(EditAnywhere, Category="Setup")
    TArray<UUnitDataAsset*> AvailableUnits;

    // Units spawned as enemies each combat round, add more enemies here.
    UPROPERTY(EditAnywhere, Category="Setup")
    TArray<UUnitDataAsset*> EnemyUnitPool;

    // Display a message on screen for the given duration
    UFUNCTION(BlueprintCallable)
    void ShowMessage(const FString& Message, float Duration = 2.f,
        FLinearColor Color = FLinearColor::White);

private:
    // Initialises all systems and spawns the game world
    UFUNCTION()
    void StartGame();

    // Populates the shop pool and generates the first shop
    void SetupShop();

    // Initialises bench slot tracking and draws debug grid lines
    void SetupBenchPositions();

    // Responds to phase changes — handles merges, enemy spawning, player registration
    UFUNCTION()
    void OnPhaseChanged(EGamePhase NewPhase);
};