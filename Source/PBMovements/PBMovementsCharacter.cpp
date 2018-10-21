// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "PBMovementsCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "RunTime/Engine/Classes/Animation/AnimInstance.h"
#include "DrawDebugHelpers.h"
#include "PBDelegates.h"


static TAutoConsoleVariable<int32> CVarDebugFootPlacement(
	TEXT("r.DebugFootPlacement"),
	0,
	TEXT("Displays vectors used during foot placement computation.\n")
	TEXT("<=0: off \n")
	TEXT("  1: On\n"),
	ECVF_Scalability | ECVF_RenderThreadSafe);


APBMovementsCharacter::APBMovementsCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Attributes
	bInRagdoll = false; 
	AnimInstance = nullptr; 
	LeftFootOffsetLimit = 50.f;
	RightFootOffsetLimit = 50.f; 
}

void APBMovementsCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("TriggerRagdoll", IE_Pressed, this, &APBMovementsCharacter::TriggerRagdoll);


	PlayerInputComponent->BindAxis("MoveForward", this, &APBMovementsCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APBMovementsCharacter::MoveRight);


	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &APBMovementsCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &APBMovementsCharacter::LookUpAtRate);
}

void APBMovementsCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void APBMovementsCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void APBMovementsCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void APBMovementsCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

float APBMovementsCharacter::FootTrace(const FName& InSocketName, const float TraceDistance, FVector& OutImpactNormal)
{
	const USkeletalMeshComponent* const Skm = GetMesh();
	if (!ensure(Skm))
		return 0.f;

	const float IKTraceDistance = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector SocketLocation = Skm->GetSocketLocation(InSocketName);
	const FVector ActorLocation = GetActorLocation();

	FVector StartLocation = SocketLocation;
	FVector EndLocation = SocketLocation;

	StartLocation.Z = ActorLocation.Z;
	EndLocation.Z = ActorLocation.Z - IKTraceDistance - TraceDistance;

	if (CVarDebugFootPlacement.GetValueOnAnyThread())
		DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red);

	FHitResult HitResult;
	if (!GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECollisionChannel::ECC_Visibility))
		return 0.f;

	if (CVarDebugFootPlacement.GetValueOnAnyThread())
	{
		DrawDebugPoint(GetWorld(), HitResult.Location, 20.f, FColor::Green);
	}

	OutImpactNormal = HitResult.Normal;
	return (HitResult.Location - HitResult.TraceEnd).Size() - TraceDistance;
}

void APBMovementsCharacter::TriggerRagdoll()
{
	bInRagdoll = !bInRagdoll;

	USkeletalMeshComponent* const Mesh = GetMesh();
	if (!ensureMsgf(Mesh, TEXT("Could not retreive mesh component while triggering ragdoll.")))
		return;

	UCapsuleComponent* const Capsule = GetCapsuleComponent();
	if (!ensureMsgf(Capsule, TEXT("Could not retreive capsule component while triggering ragdoll.")))
		return;

	const FName& RootBone = TEXT("pelvis"); 
	if (!ensureMsgf(!RootBone.IsNone(), TEXT("Could not retreive root bone name.")))
		return;

	Mesh->SetAllBodiesBelowSimulatePhysics(RootBone, bInRagdoll, true);
	Mesh->SetAllBodiesBelowPhysicsBlendWeight(RootBone, 1.f, false, true);
	
	if (bInRagdoll)
	{
		check(GetCharacterMovement()); 
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None); 
	}
	else
	{
		check(Mesh->GetAnimInstance()); 
		UAnimInstance* const AnimInstance = Mesh->GetAnimInstance(); 
		check(AnimInstance); 
		AnimInstance->SavePoseSnapshot(TEXT("ragdoll")); 
	}

	FPBDelegates& Delegates = FPBDelegates::Get();
	Delegates.OnEnterRagdoll.Broadcast(bInRagdoll); 

	OnTriggerRagdoll_BP(); 
}

void APBMovementsCharacter::BeginPlay() 
{
	Super::BeginPlay(); 

	const USkeletalMeshComponent* const Mesh = GetMesh();
	check(Mesh); 

	// Get our custom anim instance reference
	AnimInstance = Cast<UPBAnimInstance>(Mesh->GetAnimInstance()); 
}

void APBMovementsCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// IK foot placement
	check(GetMesh());
	const float LocalLeftFootOffset = FootTrace(LeftFootIkSocket, 60.f, LeftFootImpact); //FMath::Clamp(FootTrace(LeftFootIkSocket, 60.f, LeftFootImpact), -LeftFootOffsetLimit, 0.f);
	const float LocalRightFootOffset = FootTrace(RightFootIkSocket, 60.f, RightFootImpact); //FMath::Clamp(FootTrace(RightFootIkSocket, 60.f, RightFootImpact),-RightFootOffsetLimit, 0.f);

	float OffsetMin = FMath::Min(LocalRightFootOffset, LocalLeftFootOffset);
	RootOffset = FMath::Lerp(RootOffset, OffsetMin > 0.f ? 0.f : OffsetMin, 10.f*DeltaTime);

	// Correct Foot offsets
	LeftFootOffset = FMath::Lerp(LeftFootOffset, LocalLeftFootOffset - RootOffset, 10.f*DeltaTime);
	RightFootOffset = FMath::Lerp(RightFootOffset, LocalRightFootOffset - RootOffset, 10.f*DeltaTime);

	// Update anim instance
	check(AnimInstance); 
	AnimInstance->UpdateFootEffector(LeftFootOffset, false);
	AnimInstance->UpdateFootEffector(RightFootOffset, true);

	AnimInstance->UpdateRootOffset(RootOffset); 

	AnimInstance->UpdateFootRotation(LeftFootImpact, false); 
	AnimInstance->UpdateFootRotation(RightFootImpact, true);


}