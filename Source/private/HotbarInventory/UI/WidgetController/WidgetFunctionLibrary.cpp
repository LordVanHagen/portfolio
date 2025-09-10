// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/WidgetFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "ActionGameCharacter.h"
#include "Public/UI/HUD/AGHUD.h"
#include "Public/UI/WidgetController/ActionWidgetController.h"
#include "AbilitySystem/AttributeSets/AG_AttributeSetBase.h"
#include "GameFramework/PlayerState.h"

UInventoryWidgetController* UWidgetFunctionLibrary::GetInventoryWidgetController(const UObject* WorldContextObject)
{
	// Can be called, because we only intend to use this inside Widgets, whom are all local
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (AAGHUD* AGHUD = Cast<AAGHUD>(PC->GetHUD()))
		{
			APlayerState* PS = PC->GetPlayerState<APlayerState>();
			AActionGameCharacter* Character = Cast<AActionGameCharacter>(PC->GetPawn());
			check(Character);
			UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
			UAttributeSet* AS = Cast<UAttributeSet>(Character->GetAttributeSet());
			UInventoryComponent* IC = Character->GetInventoryComponent();
			const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS, IC);
			return AGHUD->GetInventoryWidgetController(WidgetControllerParams);
		}
	}

	return nullptr;
}
