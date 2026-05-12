#include "TFTGameMode.h"
#include "Shop/ShopSubsystem.h"
#include "Combat/CombatSubsystem.h"
#include "Player/TFTPlayerState.h"
#include "Units/Unit.h"
#include "Units/UnitDataAsset.h"
#include "Blueprint/UserWidget.h"
#include "Combat/BattlefieldActor.h"

void ATFTGameMode::BeginPlay()
{
    Super::BeginPlay();

    FTimerHandle StartHandle;
    GetWorld()->GetTimerManager().SetTimer(
        StartHandle,
        this,
        &ATFTGameMode::StartGame,
        0.5f,
        false
    );
}

void ATFTGameMode::StartGame()
{
    UE_LOG(LogTemp, Log, TEXT("TFTGameMode: StartGame called"));
    
    SetupShop();
    
    if (UCombatSubsystem* Combat = GetGameInstance()->GetSubsystem<UCombatSubsystem>())
    {
        Combat->OnPhaseChanged.AddDynamic(this, &ATFTGameMode::OnPhaseChanged);
        Combat->StartPrepPhase();
    }

    if (HUDWidgetClass)
    {
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            HUDWidget = CreateWidget<UTFTHUDWidget>(PC, HUDWidgetClass);
            if (HUDWidget)
            {
                HUDWidget->AddToPlayerScreen();
                HUDWidget->SetVisibility(ESlateVisibility::Visible);
                UE_LOG(LogTemp, Log, TEXT("HUD added to viewport"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("HUD widget failed to create"));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("HUDWidgetClass is null"));
    }

    FActorSpawnParameters Params;
    ABattlefieldActor* Battlefield = GetWorld()->SpawnActor<ABattlefieldActor>(
        ABattlefieldActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);

    // Give it to the combat subsystem
    if (UCombatSubsystem* Combat = GetGameInstance()->GetSubsystem<UCombatSubsystem>())
        Combat->Battlefield = Battlefield;
    
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (ATFTPlayerState* PS = PC->GetPlayerState<ATFTPlayerState>())
            PS->Battlefield = Battlefield;
    }

    
}

void ATFTGameMode::SetupShop()
{
    UShopSubsystem* Shop = GetGameInstance()->GetSubsystem<UShopSubsystem>();
    if (!Shop) return;

    for (UUnitDataAsset* Unit : AvailableUnits)
    {
        if (Unit) Shop->AllUnits.Add(Unit);
    }
    
    Shop->FreeRefresh(1); 
}

void ATFTGameMode::OnPhaseChanged(EGamePhase NewPhase)
{
    if (NewPhase == EGamePhase::Combat)
        SpawnEnemiesForRound();
}

void ATFTGameMode::SpawnEnemiesForRound()
{
    UCombatSubsystem* Combat = GetGameInstance()->GetSubsystem<UCombatSubsystem>();
    if (!Combat || EnemyUnitPool.IsEmpty()) return;

    for (int32 i = 0; i < EnemyUnitPool.Num(); i++)
    {
        UUnitDataAsset* EnemyData = EnemyUnitPool[i];
        if (!EnemyData) continue;

        FVector Location = FVector(500.f, i * 200.f, 0.f);
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AUnit* Enemy = GetWorld()->SpawnActor<AUnit>(
            AUnit::StaticClass(), Location, FRotator::ZeroRotator, Params);

        if (!Enemy) continue;

        Enemy->DataAsset = EnemyData;
        Enemy->InitFromDataAsset();
        Combat->RegisterEnemyUnit(Enemy);
    }

    UE_LOG(LogTemp, Log, TEXT("Spawned %d enemies for round %d"),
        EnemyUnitPool.Num(), Combat->GetCurrentRound());
}