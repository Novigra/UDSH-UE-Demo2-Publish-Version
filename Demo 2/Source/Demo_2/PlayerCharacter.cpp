// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "NPC.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// APlayerCharacter

APlayerCharacter::APlayerCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	PreviousInputMode.Push(EInputMode::InputModeGameOnly);

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void APlayerCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	AddMappingContext(DefaultMappingContext);
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Default

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);

		// Talk To
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &APlayerCharacter::TalkTo);

		// Pause Game
		EnhancedInputComponent->BindAction(PauseGameAction, ETriggerEvent::Triggered, this, &APlayerCharacter::PauseGame);

		
		// Dialogue

		// Pause Dialogue
		EnhancedInputComponent->BindAction(PauseDialogueAction, ETriggerEvent::Triggered, this, &APlayerCharacter::PauseDialogue);


		// UI

		// Return
		EnhancedInputComponent->BindAction(ReturnGameAction, ETriggerEvent::Triggered, this, &APlayerCharacter::ReturnGame);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APlayerCharacter::TalkTo(const FInputActionValue& Value)
{
	if (!Merchant)
		return;

	if (ANPC* NPC = Cast<ANPC>(Merchant))
	{
		UUserWidget* DialogueWidget = GetWidget("DialogueWidget");
		AddWidgetToViewport(DialogueWidget);
		
		SwitchMappingContext(ActiveMappingContext, DialogueMappingContext);

		if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
		{
			FInputModeGameAndUI InputModeGameAndUI;
			InputModeGameAndUI.SetWidgetToFocus(DialogueWidget->TakeWidget());
			InputModeGameAndUI.SetHideCursorDuringCapture(false);

			PlayerController->SetInputMode(InputModeGameAndUI);
			PlayerController->SetShowMouseCursor(true);
		}

		NPC->PrepareForDialogue();
		OnPlayMerchantSequence.Broadcast();
	}
}

void APlayerCharacter::PauseDialogue(const FInputActionValue& Value)
{
	CurrentActiveSequence->Pause();
	PreviousInputMode.Push(EInputMode::InputModeGameAndUI);

	StartPauseWidget();
}

void APlayerCharacter::PauseGame(const FInputActionValue& Value)
{
	PreviousInputMode.Push(EInputMode::InputModeGameOnly);

	StartPauseWidget();
}

void APlayerCharacter::StartPauseWidget()
{
	UUserWidget* PauseGameWidget = GetWidget("PauseGameWidget");
	AddWidgetToViewport(PauseGameWidget);

	SwitchMappingContext(ActiveMappingContext, UIMappingContext);

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		FInputModeGameAndUI InputModeGameAndUI;
		InputModeGameAndUI.SetWidgetToFocus(PauseGameWidget->TakeWidget());
		InputModeGameAndUI.SetHideCursorDuringCapture(false);

		PlayerController->SetInputMode(InputModeGameAndUI);
		PlayerController->SetShowMouseCursor(true);
	}
}

void APlayerCharacter::ReturnGame(const FInputActionValue& Value)
{
	RemoveWidget(1);

	SwitchMappingContext(ActiveMappingContext, PreviousMappingContext);

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		EInputMode InputMode = PreviousInputMode.Pop();

		if (InputMode == EInputMode::InputModeGameOnly)
		{
			PlayerController->SetInputMode(FInputModeGameOnly());
			PlayerController->SetShowMouseCursor(false);
		}
		else if (InputMode == EInputMode::InputModeGameAndUI)
		{
			FInputModeGameAndUI InputModeGameAndUI;
			InputModeGameAndUI.SetWidgetToFocus(WidgetsStack[WidgetsStack.Num() - 1]->TakeWidget()); // WidgetsStack can't be empty at this point
			InputModeGameAndUI.SetHideCursorDuringCapture(false);

			PlayerController->SetInputMode(InputModeGameAndUI);
			PlayerController->SetShowMouseCursor(true);

			if (CurrentActiveSequence)
				CurrentActiveSequence->Play();
		}
	}
}

void APlayerCharacter::SwitchMappingContext(UInputMappingContext* OldInputMappingContext, UInputMappingContext* NewInputMappingContext)
{
	RemoveMappingContext(OldInputMappingContext);
	AddMappingContext(NewInputMappingContext);
}

void APlayerCharacter::AddMappingContext(UInputMappingContext* InputMappingContext)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
			ActiveMappingContext = InputMappingContext;
		}
	}
}

void APlayerCharacter::RemoveMappingContext(UInputMappingContext* InputMappingContext)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(InputMappingContext);

			PreviousMappingContext = ActiveMappingContext;
			ActiveMappingContext = nullptr;
		}
	}
}

void APlayerCharacter::AddWidgetToViewport(UUserWidget* TargetWidget)
{
	TargetWidget->AddToViewport();
	WidgetsStack.Push(TargetWidget);
}

void APlayerCharacter::RemoveWidget(int32 Level)
{
	for (int i = 0; i < Level; ++i)
	{
		if (WidgetsStack.IsEmpty())
			break;

		UUserWidget* Widget = WidgetsStack.Pop();
		Widget->RemoveFromParent();
	}
}

void APlayerCharacter::StartStore()
{
	UUserWidget* StoreWidget = GetWidget("StoreWidget");
	AddWidgetToViewport(StoreWidget); // Btw, it's not safe to call the function before checking if the variable is valid. However, I want hard crash if it's nullptr

	PreviousInputMode.Push(EInputMode::InputModeGameAndUI);

	SwitchMappingContext(ActiveMappingContext, UIMappingContext);

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		FInputModeGameAndUI InputModeGameAndUI;
		InputModeGameAndUI.SetWidgetToFocus(StoreWidget->TakeWidget());
		InputModeGameAndUI.SetHideCursorDuringCapture(false);

		PlayerController->SetInputMode(InputModeGameAndUI);
		PlayerController->SetShowMouseCursor(true);
	}
}

void APlayerCharacter::PlayNextSequence(ULevelSequence* NextSequence, bool bIsLoopingSequence, int32 NumberOfLoops)
{
	if (CurrentActiveSequence)
	{
		OnStopSequence.Broadcast();
		CurrentActiveSequence->Stop();
	}

	if (!NextSequence)
		return;

	// Don't know yet what I can do with level sequence actor so we will just pass a temp var for now
	ALevelSequenceActor* Temp;
	CurrentActiveSequence = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), NextSequence, FMovieSceneSequencePlaybackSettings(), Temp);

	if (bIsLoopingSequence)
		CurrentActiveSequence->PlayLooping(NumberOfLoops);
	else
		CurrentActiveSequence->Play();
}

void APlayerCharacter::StopDialogue()
{
	if (CurrentActiveSequence)
	{
		OnStopSequence.Broadcast();
		CurrentActiveSequence->Stop();
	}

	RemoveWidget(1);

	SwitchMappingContext(ActiveMappingContext, DefaultMappingContext);

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
	}

	if (ANPC* NPC = Cast<ANPC>(Merchant))
	{
		Merchant = nullptr;
		NPC->PrepareForGameplay();
	}
}

UUserWidget* APlayerCharacter::GetLastActiveWidget()
{
	if (WidgetsStack.IsEmpty())
		return nullptr;

	return WidgetsStack[WidgetsStack.Num() - 1];
}

UUserWidget* APlayerCharacter::GetWidget(const FName& WidgetName)
{
	if (TObjectPtr<UUserWidget>* FoundWidget = AvailableWidgets.Find(WidgetName))
		return *FoundWidget;

	return nullptr;
}
