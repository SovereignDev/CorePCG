// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "PCGPin.h"
#include "PCGSettings.h"
#include "Elements/PCGPointProcessingElementBase.h"
#include "PCGPoint.h"

/*
*	Extract Macro to create a new C++ PCG Node
*/
#define PCG_POINT_NODE_BODY(ClassName)	\
	class FPCG##ClassName##Element : public FPCGPointProcessingElementBase	\
	{	\
		protected:	\
		virtual bool ExecuteInternal(FPCGContext* Context) const override;	\
	};	\
	UCLASS(BlueprintType, ClassGroup = (Procedural))	\
	class UPCG##ClassName##Settings : public UPCGSettings	\
	{	\
		GENERATED_BODY()	\
	public:	\
	if WITH_EDITOR	\
		virtual FName GetDefaultNodeName() const override { return FName(TEXT(#ClassName)); }	\
		virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT(PCG##ClassName##Settings, "NodeTitle", #ClassName); }	\
		virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Spatial; }	\
	endif	\
		\
	protected:	\
		virtual TArray<FPCGPinProperties> InputPinProperties() const override { return Super::DefaultPointInputPinProperties(); }	\
		virtual TArray<FPCGPinProperties> OutputPinProperties() const override { return Super::DefaultPointOutputPinProperties(); }	\
		virtual FPCGElementPtr CreateElement() const override { return MakeShared<FPCG##ClassName##Element>(); }	\
	};


/*
*	Extract this Macro inside the ExecuteInternal function of a PCG Node
*/
#define PCG_POINT_NODE_BODY_CPP(ClassName)	\
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCG##ClassName##Element::Execute);	\
	\
	const UPCG##ClassName##Settings* Settings = Context->GetInputSettings<UPCG##ClassName##Settings>();	\
	check(Settings);	\
	\
	TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);	\
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;


/*
*	Wrapper for ProcessPoints function to make it easier to use
*/
#define POINT_LOOP(...) ProcessPoints(Context, Inputs, Outputs, [##__VA_ARGS__](const FPCGPoint& InPoint, FPCGPoint& OutPoint)->bool