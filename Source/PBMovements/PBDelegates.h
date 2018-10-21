#pragma once
#include "CoreMinimal.h"


/** Delegate system */
class FPBDelegates
{
public: 
	static FPBDelegates& Get()
	{
		static FPBDelegates Instance;
		return Instance; 
	}

public : // Anim delegates


	DECLARE_MULTICAST_DELEGATE_OneParam(FOnEnterRagdoll, const bool);
	FOnEnterRagdoll OnEnterRagdoll; 

private: // Private constructor to ensure singleton
	FPBDelegates() {}
	~FPBDelegates() {}
};

