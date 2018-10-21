#pragma once

#include "RunTime/Engine/Classes/Animation/AnimInstance.h"
#include "PBAnimInstance.generated.h"


UCLASS(Blueprintable, BlueprintType)
class UPBAnimInstance : public UAnimInstance
{
	GENERATED_UCLASS_BODY()

	virtual void BeginDestroy() override;

public : // Anim instance attributes

	/** Is character currently in ragdoll */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim")
	bool bInRagdoll; 

	/** Root bone offset during IK */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim")
	float RootOffset; 

	/** Left foot vector offset to apply */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim")
	FVector LeftFootEffector; 

	/** Right foot vector offset to apply */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim")
	FVector RightFootEffector;


	/** Left foot rotation*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Anim")
	FRotator LeftFootRotation;

	/** Right foot rotation */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Anim")
	FRotator RightFootRotation;

public :  // Public Methods

	void UpdateFootEffector(const float Offset, const bool bRightFoot); 
	void UpdateRootOffset(const float Offset); 
	void UpdateFootRotation(const FVector& Impact, const bool bRightFoot); 


public : // Callbacks 

	void OnEnterRagdoll(const bool bInRagdoll); 

};

