// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.


#include "AIOverheadWidget.h"

#include "AIAssessment/Component/AIStealthComponent.h"
#include "AIAssessment/Component/IsekaiUIBridge.h"

void UAIOverheadWidget::InitFromBridge(UIsekaiUIBridge* InUIBridge)
{
	if (!InUIBridge)
	{
		return;
	}
	
	UIBridge = InUIBridge;
	
	// Bind delegates
	UIBridge->OnHealthChanged.AddDynamic(this, &ThisClass::OnHealthAttributeChanged);
	UIBridge->OnStealthStateDataChanged.AddDynamic(this, &UAIOverheadWidget::OnStealthStateDataChanged);
}

float UAIOverheadWidget::GetHealthNormalized() const
{
	return UIBridge->GetHealthNormalized();
}
