// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "CatAirBalloonPawn.h"
#include "CatAirBalloon.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ChildActorComponent.h"

#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Engine/StaticMesh.h"

ACatAirBalloonPawn::ACatAirBalloonPawn()
{

	/*
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> PlaneMesh;
		FConstructorStatics()
			: PlaneMesh(TEXT("/Game/Flying/Meshes/UFO.UFO"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	// Create static mesh component
	PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh0"));
	PlaneMesh->SetStaticMesh(ConstructorStatics.PlaneMesh.Get());	// Set static mesh
	RootComponent = PlaneMesh;
	*/

	// Must be stored to a local variable with VisibleAnywhere set in order to be editable in the BP.
	// OK, fine.
	MyRootComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Root0"));
	RootComponent = MyRootComponent;

	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->SetupAttachment(RootComponent);	// Attach SpringArm to RootComponent
	SpringArm->TargetArmLength = 160.0f; // The camera follows at this distance behind the character	
	SpringArm->SocketOffset = FVector(0.f,0.f,60.f);
	SpringArm->bEnableCameraLag = false;	// Do not allow camera to lag
	SpringArm->CameraLagSpeed = 15.f;

	// Create camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);	// Attach the camera
	Camera->bUsePawnControlRotation = false; // Don't rotate camera with controller

	// Set handling parameters
	Acceleration = 500.f;
	TurnSpeed = 50.f;
	MaxSpeed = 4000.f;
	MinSpeed = 500.f;
	CurrentForwardSpeed = 1000.f;

	// UI
	MaxHP = 100;
	HP = MaxHP;
	HPPercentage = 1.0f;

	AirTime = 0.0f;

	CatCount = 0;
	GoldCount = 10000;
	MaxHotAir = 100;
	HotAir = MaxHotAir;
	DistanceTravelled = 0;

	CatMultiplier = -100;
	GoldMultiplier = 100;
	DistanceMultiplier = 100;
	AirTimeMultiplier = 1000;
}


void ACatAirBalloonPawn::BeginPlay()
{
	Super::BeginPlay();


	StartLocation = GetActorLocation();

	if (GetWorld()) {
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ACatAirBalloonPawn::TimerExpired, 9999999.0f);
	}
}

void ACatAirBalloonPawn::Tick(float DeltaSeconds)
{
	// Descend at a calculated rate and move forward at a constant rate
	float DescentSpeed = (MaxHotAir - HotAir) * .5f + CatCount * 10.0f + GoldCount * .1f;
	
	const FVector LocalMove = FVector(CurrentForwardSpeed * DeltaSeconds, 0.f, -DescentSpeed * DeltaSeconds);

	// Move plan forwards (with sweep so we stop when we collide with things)
	AddActorLocalOffset(LocalMove, true);

	// Calculate change in rotation this frame
	FRotator DeltaRotation(0,0,0);
	DeltaRotation.Pitch = CurrentPitchSpeed * DeltaSeconds;
	DeltaRotation.Yaw = CurrentYawSpeed * DeltaSeconds;
	DeltaRotation.Roll = CurrentRollSpeed * DeltaSeconds;

	// Rotate plane
	AddActorLocalRotation(DeltaRotation);

	FVector Location = GetActorLocation();
	CurrentHeight = Location.Z / 100.0f;
	DistanceTravelled = FVector::Dist(Location, StartLocation) / 100.0f;

	if (Location.Z <= 2.0f) {

		APlayerController* const Controller = Cast<APlayerController>(GEngine->GetFirstLocalPlayerController(GetWorld()));
		if (Controller) {
			print(TEXT("Landed"));
			GameOver();
			BP_DidLandOnGround();
		}
	}

	// Call any parent class Tick implementation
	Super::Tick(DeltaSeconds);
}

void ACatAirBalloonPawn::TimerExpired()
{
	// Hack
}

void ACatAirBalloonPawn::GameOver()
{
	if (GetWorld()) {
		AirTime = GetWorld()->GetTimerManager().GetTimerElapsed(TimerHandle);
	}

	UpdateFinalScore();

}

void ACatAirBalloonPawn::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// Deflect along the surface when we collide.
	FRotator CurrentRotation = GetActorRotation();
	SetActorRotation(FQuat::Slerp(CurrentRotation.Quaternion(), HitNormal.ToOrientationQuat(), 0.025f));

	if (Other->ActorHasTag(TEXT("Floor"))) {

		// TODO: Stop collisions and let things bouce before ending game?

		print(TEXT("Landed"));
		GameOver();
		BP_DidLandOnGround();
	}
	else {
		HP--;
		HPPercentage = HP / MaxHP;

		BP_CollidingWithWall();
	}

}


void ACatAirBalloonPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    // Check if PlayerInputComponent is valid (not NULL)
	check(PlayerInputComponent);

	// Bind our control axis' to callback functions
	PlayerInputComponent->BindAction("GoUp", EInputEvent::IE_Pressed, this, &ACatAirBalloonPawn::MoveUpInput);
	PlayerInputComponent->BindAction("GoDown", EInputEvent::IE_Pressed, this, &ACatAirBalloonPawn::MoveDownInput);

	PlayerInputComponent->BindAxis("MoveRight", this, &ACatAirBalloonPawn::MoveRightInput);
}

void ACatAirBalloonPawn::MoveUpInput()
{
	// Jettison baggage to be lighter and go up
	if (GoldCount > 0) {
		GoldCount -= 100;

		if (GoldCount < 0) {
			GoldCount = 0;
		}

		print(TEXT("Dropped Money"));

		// TODO: make this not hacky but whatever it's ludum and it's ok that we do this every time once we're out of bags
		if (MoneyGroupMeshComponent && MoneyBags.Num() == 0) {
			MoneyGroupMeshComponent->GetChildrenComponents(false, MoneyBags);
		}

		USceneComponent* Bag = MoneyBags.Pop();
		if (Bag) {
			UChildActorComponent* BagActor = Cast<UChildActorComponent>(Bag);
			if (BagActor) {
				BagActor->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

				AActor* ChildActor = BagActor->GetChildActor();

				if (ChildActor) {
					TArray<UStaticMeshComponent*> Components;
					ChildActor->GetComponents<UStaticMeshComponent>(Components);
					for (UStaticMeshComponent *C : Components) {
						C->SetSimulatePhysics(true);
					}
				}


			}
		}

		BP_DroppedMoney();
	}
	/*
	else if (CatCount > 0) {
		CatCount--;

		print(TEXT("Dropped Cat"));
		BP_DroppedCat();
	}
	else {
		print(TEXT("Captain Jumped"));
		GameOver();
		BP_CaptainJumpedOut();
	} */
}

void ACatAirBalloonPawn::MoveDownInput()
{
	// Jettison Hot air to be less buoyant and go down
	if (HotAir > 0) {
		HotAir--;

		print(TEXT("Vented Air"));
		BP_VentAir();
	}

	if (HotAir == 0) {
		// No Air left, accelerate to terminal velocity

		print(TEXT("No Air Left!!"));
		GameOver();
		BP_NoAirLeft();
	}

	/*
	// Target pitch speed is based in input
	float TargetPitchSpeed = (Val * TurnSpeed * -1.f);

	// When steering, we decrease pitch slightly
	TargetPitchSpeed += (FMath::Abs(CurrentYawSpeed) * -0.2f);

	// Smoothly interpolate to target pitch speed
	CurrentPitchSpeed = FMath::FInterpTo(CurrentPitchSpeed, TargetPitchSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
	*/
}

void ACatAirBalloonPawn::MoveRightInput(float Val)
{
	// Target yaw speed is based on input
	float TargetYawSpeed = (Val * TurnSpeed);

	// Smoothly interpolate to target yaw speed
	CurrentYawSpeed = FMath::FInterpTo(CurrentYawSpeed, TargetYawSpeed, GetWorld()->GetDeltaSeconds(), 2.f);

	// Is there any left/right input?
	const bool bIsTurning = FMath::Abs(Val) > 0.2f;

	// If turning, yaw value is used to influence roll
	// If not turning, roll to reverse current roll value.
	float TargetRollSpeed = bIsTurning ? (CurrentYawSpeed * 0.5f) : (GetActorRotation().Roll * -2.f);

	// Smoothly interpolate roll speed
	CurrentRollSpeed = FMath::FInterpTo(CurrentRollSpeed, TargetRollSpeed, GetWorld()->GetDeltaSeconds(), 2.f);
}

void ACatAirBalloonPawn::UpdateFinalScore()
{
	FinalScore = DistanceTravelled * DistanceMultiplier + AirTime * AirTimeMultiplier + CatCount * CatMultiplier + GoldCount * GoldMultiplier;
}