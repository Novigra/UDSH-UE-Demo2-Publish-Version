// Copyright Epic Games, Inc. All Rights Reserved.

/*
*	I highly highly recommend you don't replicate my code if you're working on a serious project. 
	I wrote it like this because I was interested in the logic more than the architecture, and I wanted 
	to finish the demos quickly. This. sloppiness has an advantage. You don't have to think about 
	architecture, and the code is super straight forward.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Blueprint/UserWidget.h"
#include "MovieSceneSequencePlayer.h"
#include "PlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class ULevelSequence;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayMerchantSequence);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStopSequence);

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

USTRUCT(BlueprintType)
struct FItem
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gameplay)
	FName ItemTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gameplay)
	bool bHasItem;
};

UENUM()
enum class EInputMode : uint8
{
	InputModeUIOnly,
	InputModeGameOnly,
	InputModeGameAndUI,
};

UCLASS(config=Game)
class APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;
	
	/** MappingContext */
	UPROPERTY(BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> ActiveMappingContext;

	UPROPERTY(BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> PreviousMappingContext;

	// Input mode before UI
	UPROPERTY(BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TArray<EInputMode> PreviousInputMode;

	// Default Input

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	/** Interact Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> InteractAction;

	/** Pause Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> PauseGameAction;

	// Dialogue Input

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DialogueMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> PauseDialogueAction;

	// UI Input

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> UIMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ReturnGameAction;

	// Widget
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gameplay, meta = (AllowPrivateAccess = "true"))
	TMap<FName, TObjectPtr<UUserWidget>> AvailableWidgets;

	UPROPERTY(BlueprintReadWrite, Category = Gameplay, meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UUserWidget>> WidgetsStack;

	// Sequencer
	UPROPERTY(BlueprintReadWrite, Category = Gameplay, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMovieSceneSequencePlayer> CurrentActiveSequence;

	// Gameplay
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay, meta = (AllowPrivateAccess = "true"))
	FItem ShieldItem; // We will use this to test what if there's shield dialogue option, but the player doesn't have one

	UPROPERTY(BlueprintAssignable, meta = (AllowPrivateAccess = "true"))
	FOnPlayMerchantSequence OnPlayMerchantSequence;

	UPROPERTY(BlueprintAssignable, meta = (AllowPrivateAccess = "true"))
	FOnStopSequence OnStopSequence;

public:
	APlayerCharacter();
	
	// Obviously in a serious project you don't handle interaction like this, however, we will keep it simple because we only have one NPC
	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	TObjectPtr<ACharacter> Merchant;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void TalkTo(const FInputActionValue& Value);

	void PauseDialogue(const FInputActionValue& Value);

	void PauseGame(const FInputActionValue& Value);

	void StartPauseWidget();

	void ReturnGame(const FInputActionValue& Value);

	void SwitchMappingContext(UInputMappingContext* OldInputMappingContext, UInputMappingContext* NewInputMappingContext);

	void AddMappingContext(UInputMappingContext* InputMappingContext);

	void RemoveMappingContext(UInputMappingContext* InputMappingContext);

	void AddWidgetToViewport(UUserWidget* TargetWidget);

	// Level is how many widgets you wanna remove.
	// This function works, but I recommend you have a complex identification system for your widgets, so when you go back a step, you should get
	// extra details that will dictate how the rest of the systems should react. You can check Demo_1 to see it in action. It's super simple though, so take it as inspiration
	void RemoveWidget(int32 Level);

protected:

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintCallable)
	void StartStore();

	UFUNCTION(BlueprintCallable)
	void PlayNextSequence(ULevelSequence* NextSequence, bool bIsLoopingSequence, int32 NumberOfLoops = -1);

	UFUNCTION(BlueprintCallable)
	void StopDialogue();

	UFUNCTION(BlueprintCallable)
	UUserWidget* GetLastActiveWidget();

	UUserWidget* GetWidget(const FName& WidgetName);
};

