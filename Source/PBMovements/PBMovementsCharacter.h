// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PBAnimInstance.h"
#include "PBMovementsCharacter.generated.h"

UCLASS(config=Game)
class APBMovementsCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	APBMovementsCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

public: // Foot IK Attributes

	/** Socket used for IK on left foot */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = IK)
	FName LeftFootIkSocket;

	/** Socket used for IK on right foot */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = IK)
	FName RightFootIkSocket;

	/** Foot offset limit for IK */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	float FootOffsetLimit;

	/** Left foot target location computed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	float LeftFootOffset;

	/** Right foot target location computed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	float RightFootOffset;

	/** Translation to apply on root bone during IK  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	float RootOffset;

	/** Left hit trace normal */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	FVector RightFootImpact;

	/** Right hit trace normal */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	FVector LeftFootImpact;

	/**  
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	FRotator RightFootRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	FRotator LeftFootRotation; */

	/** Is character in ragdoll mode */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay)
	bool bInRagdoll; 

	/** Lerping speed for IK attributes */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	float IKLerpSpeed;

	/** Line trace distance used to track collisions during IK */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	float IKTraceDistance;

private: // Private attributes

	UPROPERTY()
	UPBAnimInstance* AnimInstance; 

protected:

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** */
	float FootTrace(const FName& InSocketName, const float TraceDistance, FVector& OutImpactNormal); 

	/** */
	void TriggerRagdoll(); 

public: // Blueprint Events

	/** */
	UFUNCTION(BlueprintImplementableEvent, Category = "Gameplay")
	void OnTriggerRagdoll_BP(); 

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override; 
	virtual void Tick(float DeltaTime) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

