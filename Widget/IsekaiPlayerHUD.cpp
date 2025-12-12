// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "IsekaiPlayerHUD.h"

#include "AIAssessment/Component/IsekaiUIBridge.h"

void UIsekaiPlayerHUD::InitFromBridge(UIsekaiUIBridge* InUIBridge)
{
	if (!InUIBridge)
	{
		return;
	}
	
	UIBridge = InUIBridge;
	
	// Bind delegates
	UIBridge->OnHealthChanged.AddDynamic(this, &UIsekaiPlayerHUD::OnHealthAttributeChanged);
	UIBridge->OnStaminaChanged.AddDynamic(this, &UIsekaiPlayerHUD::OnStaminaAttributeChanged);
	UIBridge->OnExhaustedChanged.AddDynamic(this, &UIsekaiPlayerHUD::OnExhaustedStateChanged);
	UIBridge->OnSprintingChanged.AddDynamic(this, &UIsekaiPlayerHUD::OnSprintingStateChanged);
	
	// Initial update
	OnHealthAttributeChanged(UIBridge->GetHealth(), UIBridge->GetMaxHealth());
	OnStaminaAttributeChanged(UIBridge->GetStamina(), UIBridge->GetMaxStamina());
	OnExhaustedStateChanged(UIBridge->IsExhausted());
	OnSprintingStateChanged(UIBridge->IsSprinting());
}

float UIsekaiPlayerHUD::GetHealthNormalized() const
{
	return UIBridge->GetHealthNormalized();
}

float UIsekaiPlayerHUD::GetStaminaNormalized() const
{
	return UIBridge->GetStaminaNormalized();
}
