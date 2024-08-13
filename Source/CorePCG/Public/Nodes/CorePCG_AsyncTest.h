// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "PCGContext.h"
#include "Helpers/CorePCGAsyncHelpers.h"
#include "Misc/CorePCGMacros.h"
#include "UObject/Object.h"
#include "CorePCG_AsyncTest.generated.h"

struct FPCGAsyncTestContext : public FPCGContext
{
	FTimerHandle TimerHandle;
	bool bIsFinished = false;
};

class FPCGAsyncTestElement : public FPCGPointProcessingElementBase
{
protected:
	virtual FPCGContext* Initialize(const FPCGDataCollection& InputData, TWeakObjectPtr<UPCGComponent> SourceComponent, const UPCGNode* Node) override;
	virtual bool ExecuteInternal(FPCGContext* InContext) const override;
};

class FPCGAsyncTestElement2 : public FCorePCGAsyncIterationElementBase
{
protected:
	// Wrapper for Execute Internal so that we can do async stuff. NOTE: returning false will cancel the execution
	virtual bool AsyncExecuteInternal(FCorePCGAsyncContext* Context) const override;
};

UCLASS(BlueprintType, ClassGroup = (Procedural))
class UPCGAsyncTestSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("Async Test")); }

	virtual FText GetDefaultNodeTitle() const override
	{
		return NSLOCTEXT("PCGAsyncTestSettings", "NodeTitle", "Async Test");
	}

	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Spatial; }
#endif

protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override
	{
		return Super::DefaultPointInputPinProperties();
	}

	virtual TArray<FPCGPinProperties> OutputPinProperties() const override
	{
		return Super::DefaultPointOutputPinProperties();
	}

	virtual FPCGElementPtr CreateElement() const override { return MakeShared<FPCGAsyncTestElement2>(); }
};
