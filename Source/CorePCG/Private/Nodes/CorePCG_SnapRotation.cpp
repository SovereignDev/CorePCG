// © 2021, Brock Marsh. All rights reserved.


#include "Nodes/CorePCG_SnapRotation.h"

#include "PCGContext.h"

float SnapFloat(float In, float SnapSize)
{
	float Out = In;
	Out /= SnapSize;
	Out = FMath::RoundToInt(Out);
	Out *= SnapSize;

	return Out;
}

FRotator SnapRotation(const FRotator& Rotator, const float SnapSize)
{
	float X = Rotator.Roll;
	float Y = Rotator.Pitch;
	float Z = Rotator.Yaw;

	X = SnapFloat(X, SnapSize);
	Y = SnapFloat(Y, SnapSize);
	Z = SnapFloat(Z, SnapSize);

	return FRotator(Y,Z,X);
}

bool FPCGSnapRotationElement::ExecuteInternal(FPCGContext* Context) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGSnapRotationElement::Execute);

	const UPCGSnapRotationSettings* Settings = Context->GetInputSettings<UPCGSnapRotationSettings>();
	check(Settings);
	
	TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;

	float SnapSize = Settings->SnapSize;
	
	ProcessPoints(Context, Inputs, Outputs,[&SnapSize](const FPCGPoint& InPoint, FPCGPoint& OutPoint)->bool
	{
		OutPoint = InPoint;

		OutPoint.Transform.SetRotation(SnapRotation(InPoint.Transform.Rotator(), SnapSize).Quaternion());

		return true;
	});

	return true;
}