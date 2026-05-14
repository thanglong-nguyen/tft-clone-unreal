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
    
    // Cache subsystem and player state references once
    // All update functions use these cached pointers
    UGameInstance* GI = GetGameInstance();
    CombatSS = GI->GetSubsystem<UCombatSubsystem>();
    ShopSS   = GI->GetSubsystem<UShopSubsystem>();
    
    FActorSpawnParameters Params;
    Battlefield = GetWorld()->SpawnActor<ABattlefieldActor>(
        ABattlefieldActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
    
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        PS = PC->GetPlayerState<ATFTPlayerState>();
    
    if (PS)
    {
        PS->TFTGameMode = this;
        PS->AddGold(10);
    }
    
    
    if (ShopSS)
    {
        ShopSS->TFTGameMode = this;
    }
    
    if (CombatSS)
    {
        CombatSS->TFTGameMode = this;
        CombatSS->OnPhaseChanged.AddDynamic(this, &ATFTGameMode::OnPhaseChanged);
        CombatSS->StartPrepPhase();
    }

    if (HUDWidgetClass)
    {
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            HUDWidget = CreateWidget<UTFTHUDWidget>(PC, HUDWidgetClass);
            if (HUDWidget)
            {
                HUDWidget->TFTGameMode = this;
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
    
    switch (NewPhase)
    {
        case EGamePhase::Prep:
            for (AUnit* Unit : PS->BoardUnits)
            {
                CombatSS->RegisterPlayerUnit(Unit);
            }
            break;
        
        
        case EGamePhase::Combat:
        
            if (!CombatSS || !Battlefield) return;
        
            if (!PS) return;
        
            while (PS->CanPlaceOnBoard() && PS->BenchUnits.Num() > 0)
            {
                for (AUnit* Unit : PS->BenchUnits)
                {
                    PS->TryAutoPlace(Unit);
                    if (PS->BoardUnits.Contains(Unit))
                    {
                        CombatSS->RegisterPlayerUnit(Unit);
                    }
                }
            }

            // Spawn enemies from pool
            for (int32 i = 0; i < EnemyUnitPool.Num(); i++)
            {
                UUnitDataAsset* Data = EnemyUnitPool[i];
                if (!Data) continue;

                int32 Col, Row;
                if (!Battlefield->GetNextFreeEnemyCell(Col, Row)) continue;

                FActorSpawnParameters Params;
                Params.SpawnCollisionHandlingOverride =
                    ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

                // Spawn off screen — CombatSubsystem will move them on combat start
                AUnit* Enemy = GetWorld()->SpawnActor<AUnit>(
                    AUnit::StaticClass(), FVector(0.f, 0.f, -1000.f),
                    FRotator::ZeroRotator, Params);

                if (!Enemy) continue;

                Enemy->DataAsset = Data;
                Enemy->StateWidgetClass = StateWidgetClass;
                Enemy->InitFromDataAsset();
                Enemy->GridCol = Col;
                Enemy->GridRow = Row;
                Battlefield->OccupyEnemyCell(Col, Row);
                CombatSS->RegisterEnemyUnit(Enemy);
            }
            break;
        
        case EGamePhase::Result:
            break;
    }
}
