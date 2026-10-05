// Copyright Mohammed Kenawy. All Rights Reserved.


#include "NPC.h"
#include "Demo_2/PlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "MKScriptPlay.h"

// Sets default values
ANPC::ANPC()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	InteractCollision = CreateDefaultSubobject<USphereComponent>(TEXT("InteractCollision"));
	InteractCollision->SetupAttachment(RootComponent);
	InteractCollision->InitSphereRadius(200.0f);

	InteractWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractWidget"));
	InteractWidget->SetupAttachment(RootComponent);
	InteractWidget->SetVisibility(false);

	bCanShowInteractWidget = true;

	InteractCollision->OnComponentBeginOverlap.AddDynamic(this, &ANPC::OnNPCCollisionBeginOverlap);
	InteractCollision->OnComponentEndOverlap.AddDynamic(this, &ANPC::OnNPCCollisionEndOverlap);
}

// Called when the game starts or when spawned
void ANPC::BeginPlay()
{
	Super::BeginPlay();
	
}

void ANPC::OnNPCCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bCanShowInteractWidget)
		return;

	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor))
	{
		InteractWidget->SetVisibility(true);
		PlayerCharacter->Merchant = this;
		UE_LOG(LogTemp, Warning, TEXT("We have the player"));
	}
}

void ANPC::OnNPCCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!bCanShowInteractWidget)
		return;

	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor))
	{
		InteractWidget->SetVisibility(false);
		PlayerCharacter->Merchant = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("We lost the player"));
	}
}

// Called every frame
void ANPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ANPC::PrepareForDialogue()
{
	bCanShowInteractWidget = false;
	InteractWidget->SetVisibility(false);
	PurchaseScriptPlay->ResetData();
}

void ANPC::PrepareForGameplay()
{
	bCanShowInteractWidget = true;

	TArray<AActor*> Actors;
	InteractCollision->GetOverlappingActors(Actors, APlayerCharacter::StaticClass());

	if (Actors.IsEmpty())
		return;

	InteractWidget->SetVisibility(true);

	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Actors[0]))
		PlayerCharacter->Merchant = this;
}

