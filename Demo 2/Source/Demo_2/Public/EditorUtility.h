// Copyright Mohammed Kenawy. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/RichTextBlock.h"
#include "EditorUtility.generated.h"

UCLASS()
class UDataTableActions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static void CreateRichTextStyleRow(UDataTable* DataTable, UObject* Font, FName StyleName, const FString& FontTypeface, float FontSize, bool bIsUnderline, bool bIsStrikethrough, FLinearColor FontColor, const FSlateBrush& Stroke = FSlateBrush())
	{
		FRichTextStyleRow Style;
		Style.TextStyle = FTextBlockStyle()
			.SetFont(FSlateFontInfo(Font, FontSize))
			.SetColorAndOpacity(FSlateColor(FontColor));

		Style.TextStyle.Font.TypefaceFontName = *FontTypeface;

		if (bIsUnderline)
			Style.TextStyle.SetUnderlineBrush(Stroke);
		if (bIsStrikethrough)
			Style.TextStyle.SetStrikeBrush(Stroke);

		DataTable->AddRow(StyleName, Style);
	}
};
