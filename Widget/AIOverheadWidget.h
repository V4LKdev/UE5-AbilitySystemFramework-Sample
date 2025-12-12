// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIAssessment/Component/AIStealthComponent.h"
#include "Blueprint/UserWidget.h"
#include "AIOverheadWidget.generated.h"

enum class EStealthState : uint8;
class UIsekaiUIBridge;
/**
 * 
 */
UCLASS()
class AIASSESSMENT_API UAIOverheadWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void InitFromBridge(UIsekaiUIBridge* InUIBridge);
	
protected:
	UPROPERTY()
	TObjectPtr<UIsekaiUIBridge> UIBridge;

	UFUNCTION(BlueprintImplementableEvent, Category="Isekai|UI")
	void OnHealthAttributeChanged(float NewValue, float MaxValue);
	
	UFUNCTION(BlueprintImplementableEvent, Category="Isekai|UI")
	void OnStealthStateDataChanged(const FStealthStateData& NewData);
	
	UFUNCTION(BlueprintPure, Category="Isekai|UI")
	float GetHealthNormalized() const;

};
