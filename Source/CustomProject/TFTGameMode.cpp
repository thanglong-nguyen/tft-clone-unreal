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
    // Setup data before caching subsystems so pool is ready
    SetupShop();
    SetupBenchPositions();

    // Cache all subsystems once — everything else accesses them through TFTGameMode
    UGameInstance* GI = GetGameInstance();
    CombatSS = GI->GetSubsystem<UCombatSubsystem>();
    ShopSS   = GI->GetSubsystem<UShopSubsystem>();
    TraitSS  = GI->GetSubsystem<UTraitSubsystem>();

    // Spawn the battlefield actor at world origin
    FActorSpawnParameters Params;
    Battlefield = GetWorld()->SpawnActor<ABattlefieldActor>(
        ABattlefieldActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);

    // Cache player state and pipe this back so PS can access all systems
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        PS = PC->GetPlayerState<ATFTPlayerState>();

    if (PS)
    {
        PS->TFTGameMode = this;
        PS->AddGold(10); // starting gold
    }

    // Pipe TFTGameMode into all subsystems
    if (TraitSS) TraitSS->TFTGameMode = this;
    if (ShopSS)  ShopSS->TFTGameMode  = this;

    if (CombatSS)
    {
        CombatSS->TFTGameMode = this;
        CombatSS->OnPhaseChanged.AddDynamic(this, &ATFTGameMode::OnPhaseChanged);
        CombatSS->StartPrepPhase();
    }

    // Create board widget — hidden by default, toggled by player
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

    // Create HUD — always visible
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

    // Register all available unit data assets with the shop pool
    for (UUnitDataAsset* Unit : AvailableUnits)
        if (Unit) Shop->AllUnits.Add(Unit);

    // Generate the first shop for free
    Shop->FreeRefresh(1);
}

void ATFTGameMode::SetupBenchPositions()
{
    int Slots = 5;
    BenchSlots.Init(false, Slots); // all slots start empty
    float Spacing = 200.f;
    float HalfCell = Spacing * 0.5f;
    UWorld* World = GetWorld();

    // Draw vertical dividers between bench slots
    for (int32 i = 0; i <= Slots; i++)
    {
        FVector Start = BenchOrigin + FVector(-HalfCell, i * Spacing - HalfCell, 1.f);
        FVector End   = BenchOrigin + FVector( HalfCell, i * Spacing - HalfCell, 1.f);
        DrawDebugLine(World, Start, End, FColor::Yellow, true, -1.f, 0, 2.f);
    }

    // Draw top and bottom border lines
    FVector TopLeft     = BenchOrigin + FVector( HalfCell, -HalfCell,                   1.f);
    FVector TopRight    = BenchOrigin + FVector( HalfCell,  Slots * Spacing - HalfCell, 1.f);
    FVector BottomLeft  = BenchOrigin + FVector(-HalfCell, -HalfCell,                   1.f);
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
    return -1; // bench full
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
    // -1 key means each call adds a new message rather than replacing the previous one
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, Duration, Color.ToFColor(true), Message);
}

void ATFTGameMode::OnPhaseChanged(EGamePhase NewPhase)
{
    switch (NewPhase)
    {
        case EGamePhase::Prep:
            // Check merges for all units in case new copies were acquired last round
            for (AUnit* Unit : PS->BoardUnits)
                PS->CheckForMerge(Unit->UnitName);
            for (AUnit* Unit : PS->BenchUnits)
                PS->CheckForMerge(Unit->UnitName);
            break;

        case EGamePhase::Combat:
        {
            if (!CombatSS || !Battlefield || !PS) return;

            // Register all board units with combat subsystem
            for (AUnit* Unit : PS->BoardUnits)
                CombatSS->RegisterPlayerUnit(Unit);

            // Auto-fill remaining board slots from bench before combat starts
            while (PS->CanPlaceOnBoard() && PS->BenchUnits.Num() > 0)
            {
                for (AUnit* Unit : PS->BenchUnits)
                {
                    PS->TryAutoPlace(Unit);
                    if (PS->BoardUnits.Contains(Unit))
                        CombatSS->RegisterPlayerUnit(Unit);
                }
            }

            // Spawn enemies and assign them to grid cells
            for (int32 i = 0; i < EnemyUnitPool.Num(); i++)
            {
                UUnitDataAsset* Data = EnemyUnitPool[i];
                if (!Data) continue;

                int32 Col, Row;
                if (!Battlefield->GetNextFreeEnemyCell(Col, Row)) continue;

                FActorSpawnParameters Params;
                Params.SpawnCollisionHandlingOverride =
                    ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

                // Spawn off screen — CombatSubsystem moves them to grid on combat start
                AUnit* Enemy = GetWorld()->SpawnActor<AUnit>(
                    AUnit::StaticClass(), FVector(0.f, 0.f, -1000.f),
                    FRotator::ZeroRotator, Params);

                if (!Enemy) continue;

                Enemy->DataAsset       = Data;
                Enemy->StateWidgetClass = StateWidgetClass;
                Enemy->InitFromDataAsset();
                Enemy->SetStateWidget();

                // Red health bar to distinguish enemies from player units
                if (Enemy->StateWidget)
                    Enemy->StateWidget->HealthBar->SetFillColorAndOpacity(FLinearColor::Red);

                Enemy->GridCol = Col;
                Enemy->GridRow = Row;
                Battlefield->OccupyEnemyCell(Col, Row);
                CombatSS->RegisterEnemyUnit(Enemy);
            }
            break;
        }

        case EGamePhase::Result:
            break;
    }
}