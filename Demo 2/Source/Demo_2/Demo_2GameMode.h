// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Demo_2GameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActiveLanguageChanged);

UCLASS(minimalapi)
class ADemo_2GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADemo_2GameMode();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 ActiveLanguageIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TArray<FString> AvailableLanguages;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Data")
	FOnActiveLanguageChanged OnActiveLanguageChanged;

	UFUNCTION(BlueprintCallable)
	FString GetActiveLanguageString();

	UFUNCTION(BlueprintCallable)
	FText GetActiveLanguageText();
};
