#include "TFTGameMode.h"
#include "Shop/ShopSubsystem.h"
#include "Combat/CombatSubsystem.h"
#include "Player/TFTPlayerState.h"
#include "Units/Unit.h"
#include "Units/UnitDataAsset.h"
#include "Blueprint/UserWidget.h"
#include "Combat/BattlefieldActor.h"
#include "Traits/TraitSubsystem.h"
#include "UI/BoardWidget.h"

void ATFTGameMode::BeginPlay()
{
    Super::BeginPlay();
    StartGame();
}

void ATFTGameMode::StartGame()
{
    
    SetupShop();
    SetupBenchPositions();
    
    // Cache subsystem and player state references once
    // All update functions use these cached pointers
    UGameInstance* GI = GetGameInstance();
    CombatSS = GI->GetSubsystem<UCombatSubsystem>();
    ShopSS   = GI->GetSubsystem<UShopSubsystem>();
    TraitSS = GetGameInstance()->GetSubsystem<UTraitSubsystem>();
    
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
    
    if (TraitSS)
    {
        TraitSS->TFTGameMode = this;
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
    
    if (BoardWidgetClass)
    {
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            BoardWidget = CreateWidget<UBoardWidget>(PC, BoardWidgetClass);
            if (BoardWidget)
            {
                BoardWidget->TFTGameMode = this;
                BoardWidget->AddToPlayerScreen();
                BoardWidget->SetVisibility(ESlateVisibility::Hidden);
            }
        }
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
            }
        }
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

void ATFTGameMode::SetupBenchPositions()
{
    int Slots = 5;
    BenchSlots.Init(false, Slots);
    float Spacing = 200.f;

    // Draw bench grid lines
    UWorld* World = GetWorld();
    float HalfCell = Spacing * 0.5f;

    for (int32 i = 0; i <= Slots; i++)
    {
        // Vertical lines — separating each bench slot
        FVector Start = BenchOrigin + FVector(-HalfCell, i * Spacing - HalfCell, 1.f);
        FVector End   = BenchOrigin + FVector(HalfCell,  i * Spacing - HalfCell, 1.f);
        DrawDebugLine(World, Start, End, FColor::Yellow, true, -1.f, 0, 2.f);
    }

    // Top and bottom horizontal lines
    FVector TopLeft     = BenchOrigin + FVector(HalfCell,  -HalfCell,            1.f);
    FVector TopRight    = BenchOrigin + FVector(HalfCell,   Slots * Spacing - HalfCell, 1.f);
    FVector BottomLeft  = BenchOrigin + FVector(-HalfCell, -HalfCell,            1.f);
    FVector BottomRight = BenchOrigin + FVector(-HalfCell,  Slots * Spacing - HalfCell, 1.f);

    DrawDebugLine(World, TopLeft,    TopRight,    FColor::Yellow, true, -1.f, 0, 2.f);
    DrawDebugLine(World, BottomLeft, BottomRight, FColor::Yellow, true, -1.f, 0, 2.f);
}

FVector ATFTGameMode::GetBenchPosition(int32 Index) const
{
    return BenchOrigin + FVector(0.f, Index * 200.f, 0.f);
}

int32 ATFTGameMode::GetNextFreeBenchSlot() const
{
    for (int32 i = 0; i < BenchSlots.Num(); i++)
        if (!BenchSlots[i]) return i;
    return -1;
}

void ATFTGameMode::OccupyBenchSlot(int32 Index)
{
    if (BenchSlots.IsValidIndex(Index))
        BenchSlots[Index] = true;
}

void ATFTGameMode::FreeBenchSlot(int32 Index)
{
    if (BenchSlots.IsValidIndex(Index))
        BenchSlots[Index] = false;
}

void ATFTGameMode::ShowMessage(const FString& Message, float Duration, FLinearColor Color)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,        // key — -1 means new message each time
            Duration,  // how long it stays
            Color.ToFColor(true),
            Message
        );
    }
}

void ATFTGameMode::OnPhaseChanged(EGamePhase NewPhase)
{
    
    switch (NewPhase)
    {
        case EGamePhase::Prep:
        
            for (AUnit* Unit: PS->BoardUnits)
            {
                PS->CheckForMerge(Unit->UnitName);
            }
        
            for (AUnit* Unit: PS->BenchUnits)
            {
                PS->CheckForMerge(Unit->UnitName);
            }
        
            break;
        
        
        case EGamePhase::Combat:
        
            if (!CombatSS || !Battlefield) return;
        
            if (!PS) return;
        
            for (AUnit* Unit : PS->BoardUnits)
            {
                CombatSS->RegisterPlayerUnit(Unit);
            }
        
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
                Enemy->SetStateWidget();
                
                if (Enemy->StateWidget)
                {
                    FLinearColor HealthColor = FLinearColor::Red;
                    Enemy->StateWidget->HealthBar->SetFillColorAndOpacity(HealthColor);
                }
                
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
