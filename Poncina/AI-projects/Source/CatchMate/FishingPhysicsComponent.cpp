#include "FishingPhysicsComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UFishingPhysicsComponent::UFishingPhysicsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Inizializzazione delle proprietà di default
	MaxLineTension = 100.0f; // Esempio: 100 Newton
	BaseDragCoefficient = 50.0f; // Esempio: 50 N per unità di frizione
	RodFlexibility = 0.5f; // A metà tra rigido e flessibile
	MaxReelSpeedEffect = 20.0f; // 20N max dalla velocità di recupero
	FishStaminaEffect = 30.0f; // 30N max dalla stamina del pesce
	FishAggressionEffect = 40.0f; // 40N max dall'aggressione del pesce

	CurrentReelSpeed = 0.0f;
	CurrentDragSetting = 0.5f;
	CurrentFishStamina = 1.0f;
	bIsFishAttacking = false;

	CurrentLineTension = 0.0f;
	NormalizedLineTension = 0.0f;
	FFBOutputNewtonMeters = 0.0f;
	bLineBroke = false;

	InternalLineTension = 0.0f;
	LastTensionPeak = 0.0f;
	RodFlexionVelocity = 0.0f;
}

// Called when the game starts
void UFishingPhysicsComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

// Called every frame
void UFishingPhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bLineBroke) return; // Se il filo è rotto, non calcolare più

	CalculateLineTension(DeltaTime);
	ApplyRodFlexion();
	CheckLineBreak();

	// Aggiorna FFB Output
	FFBOutputNewtonMeters = CurrentLineTension * FFB_CONVERSION_FACTOR;

	// Invia eventi
	OnLineTensionChanged.Broadcast(NormalizedLineTension, FFBOutputNewtonMeters);
}

void UFishingPhysicsComponent::CalculateLineTension(float DeltaTime)
{
	float RawTension = 0.0f;

	// 1. Tensione da velocità di recupero e frizione
	// Maggiore è la velocità di recupero, maggiore la tensione, ma la frizione la riduce
	float ReelTension = CurrentReelSpeed * MaxReelSpeedEffect;
	
	// La frizione agisce come un limite o una resistenza. Più è chiusa (verso 1.0), più resistenza offre.
	// Una frizione completamente aperta (0.0) permette al filo di scorrere liberamente, riducendo la tensione.
	// Una frizione completamente chiusa (1.0) blocca il filo, trasferendo tutta la tensione.
	float DragResistance = CurrentDragSetting * BaseDragCoefficient;

	// Se la ReelTension supera la DragResistance, parte dell'energia viene dissipata dalla frizione.
	// La tensione effettiva dovuta al recupero sarà limitata dalla DragResistance.
	RawTension += FMath::Min(ReelTension, DragResistance);

	// 2. Comportamento e stamina del pesce
	float FishTension = 0.0f;

	// Effetto della stamina: un pesce stanco (Stamina bassa) tira meno.
	FishTension += CurrentFishStamina * FishStaminaEffect;

	// Strattonate casuali del pesce (se sta attaccando)
	if (bIsFishAttacking)
	{
		// Aggiunge una componente di rumore/strattonata quando il pesce attacca
		FishTension += FMath::RandRange(-FISH_JERK_MAGNITUDE, FISH_JERK_MAGNITUDE);
	}

	// Fuga del pesce: un comportamento più persistente e forte
	// Potrebbe essere attivato da un altro parametro o basato sulla logica AI del pesce
	// Per ora, lo simulo con una probabilità o una condizione.
	// Esempio: se il pesce è molto aggressivo e non è stanco, tenta una fuga
	// if (bIsFishAggressive && CurrentFishStamina > 0.5f) // Esempio di condizione
	// {
	//	FishTension += FISH_RUN_MAGNITUDE;
	// }

	RawTension += FishTension;

	// Aggiorna la tensione interna prima della flessione della canna
	InternalLineTension = RawTension;
}

void UFishingPhysicsComponent::ApplyRodFlexion()
{
	// La flessione della canna ammortizza i picchi di tensione.
	// Una canna più flessibile (RodFlexibility alta) riduce maggiormente la tensione istantanea.
	// Calcoliamo la differenza tra la tensione interna e quella attuale (ammortizzata).

	float TargetRodFlexion = FMath::GetMappedRangeValueClamped(
		TRange<float>(0.0f, MaxLineTension),
		TRange<float>(0.0f, 1.0f),
		InternalLineTension
	);

	// Applica la flessibilità per ridurre la tensione percepita.
	// La tensione finale è una versione smussata della tensione interna.
	// Usiamo un FInterpTo per simulare l'ammortizzazione.

	float LerpSpeed = FMath::Lerp(5.0f, 50.0f, RodFlexibility); // Canna flessibile reagisce più lentamente ai picchi

	CurrentRodFlexion = FMath::FInterpTo(CurrentRodFlexion, TargetRodFlexion, GetWorld()->GetDeltaSeconds(), LerpSpeed);

	// La tensione effettiva sul filo è influenzata dalla flessione della canna.
	// Una flessione maggiore significa che la canna sta assorbendo parte della tensione.
	// Mappiamo la flessione della canna indietro a una riduzione della tensione.
	CurrentLineTension = FMath::GetMappedRangeValueClamped(
		TRange<float>(0.0f, 1.0f),
		TRange<float>(0.0f, InternalLineTension),
		1.0f - CurrentRodFlexion // Maggiore flessione (vicino a 1.0) -> minore tensione rimanente
	);

	// La tensione normalizzata è sempre rispetto alla tensione massima.
	NormalizedLineTension = CurrentLineTension / MaxLineTension;

	// Assicurati che la tensione non sia negativa
	CurrentLineTension = FMath::Max(0.0f, CurrentLineTension);
}

void UFishingPhysicsComponent::CheckLineBreak()
{
	// Condizione di rottura del filo:
	// La tensione supera il valore massimo E la frizione è troppo chiusa (ad esempio, > 0.9)

	if (CurrentLineTension > MaxLineTension && CurrentDragSetting > 0.9f)
	{
		bLineBroke = true;
		OnLineBroke.Broadcast();
		UE_LOG(LogTemp, Warning, TEXT("Il filo si è rotto!"));
	}
}
