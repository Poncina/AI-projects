#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FishingPhysicsComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLineTensionChanged, float, NormalizedTension, float, FFBNewtonMeters);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLineBroke);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CATCHMATE_API UFishingPhysicsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFishingPhysicsComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Proprietà configurabili da Blueprint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Physics")
	float MaxLineTension; // Tensione massima del filo prima che si rompa (in Newton)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Physics")
	float BaseDragCoefficient; // Coefficiente di attrito base per la frizione

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Physics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RodFlexibility; // Flessibilità della canna (0.0 = rigida, 1.0 = molto flessibile)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Physics")
	float MaxReelSpeedEffect; // Effetto massimo della velocità di recupero sulla tensione

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Physics")
	float FishStaminaEffect; // Quanto la stamina del pesce influisce sulla tensione

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Physics")
	float FishAggressionEffect; // Quanto l'aggressione del pesce influisce sulla tensione

	// Input dal gioco
	UPROPERTY(BlueprintReadWrite, Category = "Fishing Physics")
	float CurrentReelSpeed; // Velocità attuale di recupero (0.0 - 1.0)

	UPROPERTY(BlueprintReadWrite, Category = "Fishing Physics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CurrentDragSetting; // Impostazione attuale della frizione (0.0 = aperta, 1.0 = chiusa)

	UPROPERTY(BlueprintReadWrite, Category = "Fishing Physics")
	float CurrentFishStamina; // Stamina attuale del pesce (0.0 - 1.0)

	UPROPERTY(BlueprintReadWrite, Category = "Fishing Physics")
	bool bIsFishAttacking; // Il pesce sta attaccando/strattonando

	UPROPERTY(BlueprintReadWrite, Category = "Fishing Physics")
	float CurrentRodFlexion; // Flessione attuale della canna (output, non input)

	// Output
	UPROPERTY(BlueprintReadOnly, Category = "Fishing Physics")
	float CurrentLineTension; // Tensione attuale del filo (in Newton)

	UPROPERTY(BlueprintReadOnly, Category = "Fishing Physics")
	float NormalizedLineTension; // Tensione normalizzata del filo (0.0 - 1.0)

	UPROPERTY(BlueprintReadOnly, Category = "Fishing Physics")
	float FFBOutputNewtonMeters; // Forza di Feedback in Newton-metri

	UPROPERTY(BlueprintReadOnly, Category = "Fishing Physics")
	bool bLineBroke; // Il filo si è rotto

	// Eventi
	UPROPERTY(BlueprintAssignable, Category = "Fishing Physics Events")
	FOnLineTensionChanged OnLineTensionChanged;

	UPROPERTY(BlueprintAssignable, Category = "Fishing Physics Events")
	FOnLineBroke OnLineBroke;

private:
	void CalculateLineTension(float DeltaTime);
	void ApplyRodFlexion();
	void CheckLineBreak();

	float InternalLineTension; // Tensione interna prima dell'ammortizzazione della canna
	float LastTensionPeak; // Ultimo picco di tensione per la flessione della canna
	float RodFlexionVelocity; // Velocità di flessione della canna

	const float FFB_CONVERSION_FACTOR = 0.5f; // Fattore di conversione arbitrario per FFB
	const float FISH_JERK_MAGNITUDE = 10.0f; // Magnitudo delle strattonate del pesce
	const float FISH_RUN_MAGNITUDE = 20.0f; // Magnitudo della fuga del pesce
};
