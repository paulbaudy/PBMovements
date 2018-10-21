#include "PBAnimInstance.h"
#include "PBDelegates.h"

UPBAnimInstance::UPBAnimInstance(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
, bInRagdoll(false)
, RootOffset(0.f)
, LeftFootEffector(FVector::ZeroVector)
, RightFootEffector(FVector::ZeroVector)
, LeftFootRotation(FRotator::ZeroRotator)
, RightFootRotation(FRotator::ZeroRotator)
{
	FPBDelegates& Delegates = FPBDelegates::Get();
	Delegates.OnEnterRagdoll.AddUObject(this, &UPBAnimInstance::OnEnterRagdoll); 
}

void UPBAnimInstance::BeginDestroy()
{
	Super::BeginDestroy(); 

	FPBDelegates& Delegates = FPBDelegates::Get();
	Delegates.OnEnterRagdoll.RemoveAll(this); 
}

void UPBAnimInstance::OnEnterRagdoll(const bool bInRagdoll)
{
	this->bInRagdoll = bInRagdoll; 
}

void UPBAnimInstance::UpdateFootEffector(const float Offset, const bool bRightFoot)
{
	FVector& FootVector = bRightFoot ? RightFootEffector : LeftFootEffector; 
	FootVector.X = bRightFoot ? - Offset : Offset;
}

void UPBAnimInstance::UpdateRootOffset(const float Offset)
{
	RootOffset = Offset; 
}

void UPBAnimInstance::UpdateFootRotation(const FVector& Impact, const bool bRightFoot)
{
	FRotator& FootRotator = bRightFoot ? RightFootRotation : LeftFootRotation; 
    FootRotator.Roll = FMath::RadiansToDegrees(FMath::Atan2(Impact.Y, Impact.Z)); 
	FootRotator.Pitch = -FMath::RadiansToDegrees(FMath::Atan2(Impact.X, Impact.Z));
}