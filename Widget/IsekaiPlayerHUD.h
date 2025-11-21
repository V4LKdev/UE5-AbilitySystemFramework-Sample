// Copyright (c) 2025 V4LKdev and Vlad. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "IsekaiPlayerHUD.generated.h"

/**
 * Base HUD widget for the local player
 * Binds to UIBridge to show player info
 */
UCLASS()
class AIASSESSMENT_API UIsekaiPlayerHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="Isekai|UI")
	void InitFromBridge(class UIsekaiUIBridge* InUIBridge);
	
protected:
	UPROPERTY()
	TObjectPtr<UIsekaiUIBridge> UIBridge;
	
	UFUNCTION(BlueprintImplementableEvent, Category="Isekai|UI")
	void OnHealthAttributeChanged(float NewValue, float MaxValue);

	UFUNCTION(BlueprintImplementableEvent, Category="Isekai|UI")
	void OnStaminaAttributeChanged(float NewValue, float MaxValue);

	UFUNCTION(BlueprintImplementableEvent, Category="Isekai|UI")
	void OnExhaustedStateChanged(bool bNewValue);

	UFUNCTION(BlueprintImplementableEvent, Category="Isekai|UI")
	void OnSprintingStateChanged(bool bNewValue);
	
	UFUNCTION(BlueprintPure, Category="Isekai|UI")
	float GetHealthNormalized() const;
	UFUNCTION(BlueprintPure, Category="Isekai|UI")
	float GetStaminaNormalized() const;
};
