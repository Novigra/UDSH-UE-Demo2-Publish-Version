// Copyright Epic Games, Inc. All Rights Reserved.

#include "Demo_2GameMode.h"
#include "PlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"

ADemo_2GameMode::ADemo_2GameMode()
{
	// set default pawn class to our Blueprinted character
	/*static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}*/
}

FString ADemo_2GameMode::GetActiveLanguageString()
{
	if (AvailableLanguages.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode: AvailableLanguages array is empty"))
		return FString();
	}
	else if (ActiveLanguageIndex < 0 || ActiveLanguageIndex >= AvailableLanguages.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode: ActiveLanguageIndex is out of AvailableLanguages array range"))
		return FString();
	}

	return AvailableLanguages[ActiveLanguageIndex];
}

FText ADemo_2GameMode::GetActiveLanguageText()
{
	if (AvailableLanguages.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode: AvailableLanguages array is empty"))
			return FText();
	}
	else if (ActiveLanguageIndex < 0 || ActiveLanguageIndex >= AvailableLanguages.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode: ActiveLanguageIndex is out of AvailableLanguages array range"))
			return FText();
	}

	return FText::FromString(AvailableLanguages[ActiveLanguageIndex]);
}
