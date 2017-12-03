// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/BoxComponent.h"

#include "CatAirBalloonPawn.generated.h"

UCLASS(Config=Game)
class ACatAirBalloonPawn : public APawn
{
	GENERATED_BODY()

	/** StaticMesh component that will be the visuals for our flying pawn */
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* PlaneMesh;

	/** Spring arm that will offset the camera */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArm;

	/** Camera component that will be our viewpoint */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;
public:
	ACatAirBalloonPawn();

	// Begin AActor overrides
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;

	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	// End AActor overrides

protected:

	// Begin APawn overrides
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override; // Allows binding actions/axes to functions
	// End APawn overrides
	
	void MoveUpInput();

	void MoveDownInput();

	void MoveRightInput(float Val);

private:

	/** How quickly forward speed changes */
	UPROPERTY(Category=Plane, EditAnywhere)
	float Acceleration;

	/** How quickly pawn can steer */
	UPROPERTY(Category=Plane, EditAnywhere)
	float TurnSpeed;

	/** Max forward speed */
	UPROPERTY(Category = Pitch, EditAnywhere)
	float MaxSpeed;

	/** Min forward speed */
	UPROPERTY(Category=Yaw, EditAnywhere)
	float MinSpeed;

	/** Current forward speed */
	float CurrentForwardSpeed;

	/** Current yaw speed */
	float CurrentYawSpeed;

	/** Current pitch speed */
	float CurrentPitchSpeed;

	/** Current roll speed */
	float CurrentRollSpeed;

public:

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* MyRootComponent;

	float HP;
	float MaxHP;

	// Heavy things
	UPROPERTY(BlueprintReadOnly, Category = "cats")
	int CatCount;

	UPROPERTY(BlueprintReadOnly, Category = "cats")
	int CatMultiplier;

	UPROPERTY(BlueprintReadOnly, Category = "cats")
	int GoldCount;

	UPROPERTY(BlueprintReadOnly, Category = "cats")
	float AirTime;

	UPROPERTY(BlueprintReadOnly, Category = "cats")
	int GoldMultiplier;

	UPROPERTY(BlueprintReadOnly, Category = "cats")
	int AirTimeMultiplier;

	UPROPERTY(BlueprintReadOnly, Category = "cats")
	int DistanceMultiplier;

	UPROPERTY(BlueprintReadOnly, Category = "cats")
	float FinalScore;

	// Light things
	float MaxHotAir;
	float HotAir;

	UPROPERTY(BlueprintReadOnly, Category = "cats")
	float DistanceTravelled;

	UPROPERTY(BlueprintReadOnly, Category = "cats")
	float CurrentHeight;

	UPROPERTY(BlueprintReadOnly, Category = "cats")
	float HPPercentage;


	UFUNCTION(BlueprintImplementableEvent)
	void BP_CollidingWithWall();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_WarnTerminalVelocity();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_StopWarningTerminalVelocity();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_DroppedMoney();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_DroppedCat();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_PickedUpMoney();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_PickedUpCat();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_VentAir();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_DidLandOnGround();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_CaptainJumpedOut();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_NoAirLeft();

	UFUNCTION(BlueprintCallable)
	void UpdateFinalScore();


	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	UStaticMeshComponent* MoneyGroupMeshComponent;

	TArray<USceneComponent*> MoneyBags;


	// Lower corner items
	FVector StartLocation;

	FTimerHandle TimerHandle;
	void TimerExpired();

	void GameOver();




	/** Returns SpringArm subobject **/
	FORCEINLINE class USpringArmComponent* GetSpringArm() const { return SpringArm; }
	/** Returns Camera subobject **/
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }


private:


};
