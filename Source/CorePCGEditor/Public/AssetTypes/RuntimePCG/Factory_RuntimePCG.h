// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "Factory_RuntimePCG.generated.h"

/**
 * 
 */
UCLASS()
class COREPCGEDITOR_API UFactory_RuntimePCG : public UFactory
{
	GENERATED_BODY()

	UFactory_RuntimePCG();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};