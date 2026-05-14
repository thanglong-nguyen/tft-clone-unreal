#include "UI/BoardWidget.h"
#include "UI/SynergyCard.h"
#include "BenchUnitCard.h"
#include "TFTGameMode.h"
#include "Traits/TraitSubsystem.h"
#include "Shop/ShopSubsystem.h"
#include "Player/TFTPlayerState.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"

void UBoardWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BindDelegates();
}

void UBoardWidget::BindDelegates()
{
    if (!TFTGameMode) return;

    // Refresh synergies when traits change
    if (UTraitSubsystem* Traits = GetGameInstance()->GetSubsystem<UTraitSubsystem>())
        Traits->OnTraitsUpdated.AddDynamic(this, &UBoardWidget::HandleTraitsUpdated);

    // Refresh bench when a unit is bought
    if (TFTGameMode->ShopSS)
        TFTGameMode->ShopSS->OnUnitPurchased.AddDynamic(
            this, &UBoardWidget::HandleUnitPurchased);
}

void UBoardWidget::RefreshSynergies()
{
    if (!SynergyContainer || !SynergyCardClass) return;

    UTraitSubsystem* Traits = GetGameInstance()->GetSubsystem<UTraitSubsystem>();
    if (!Traits) return;

    // Clear existing cards
    SynergyContainer->ClearChildren();

    // Add a card for every trait the player has at least 1 unit in
    for (const FActiveTraitStatus& Status : Traits->GetAllTraitStatuses())
    {
        if (Status.CurrentCount == 0) continue; // skip traits with no units

        USynergyCard* Card = CreateWidget<USynergyCard>(
            this, SynergyCardClass);
        if (!Card) continue;

        Card->SetData(
            Status.DisplayName,
            Status.CurrentCount,
            Status.NextThreshold,
            Status.bIsActive
        );

        SynergyContainer->AddChild(Card);
    }
}

void UBoardWidget::RefreshBench()
{
    if (!BenchContainer || !BenchUnitCardClass || !TFTGameMode->PS) return;

    BenchContainer->ClearChildren();

    for (AUnit* Unit : TFTGameMode->PS->BenchUnits)
    {
        if (!Unit) continue;

        UBenchUnitCard* Card = CreateWidget<UBenchUnitCard>(
            this, BenchUnitCardClass);
        if (!Card) continue;
        
        Card->SetUnit(Unit);
        BenchContainer->AddChild(Card);
    }
}

void UBoardWidget::HandleTraitsUpdated()
{
    RefreshSynergies();
}

void UBoardWidget::HandleUnitPurchased(AUnit* NewUnit)
{
    RefreshBench();
    RefreshSynergies();
}