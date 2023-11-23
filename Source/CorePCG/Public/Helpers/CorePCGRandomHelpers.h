// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

/**
 * 
 */

/*	Sets a Variable based on an Array Rarity Based Structs */
#define GET_FROM_WEIGHTED_STRUCT(Type, Array) \
[](TArray<Type> PassedInArray) \
{ \
TArray<float> Weights; \
Weights.Reserve(PassedInArray.Num()); \
for(const Type& Element : PassedInArray) \
{ \
Weights.Add(Element.Weight); \
} \
const int32 Index = FCorePCGWeightedRandomHelpers::Get(Weights); \
return PassedInArray[Index]; \
}(Array);

/*	Sets a Variable based on an Array of Weighted Structs using a Stream */
#define GET_FROM_WEIGHT_STRUCT_WITH_STREAM(Type, Array, Stream) \
[](TArray<Type> PassedInArray, const FRandomStream& InputStream) \
{ \
TArray<float> Weights; \
for(const Type& Element : PassedInArray) \
{ \
Weights.Add(Element.Weight); \
} \
const int32 Index = FCorePCGWeightedRandomHelpers::Get(Weights, InputStream); \
return PassedInArray[Index]; \
}(Array, Stream)

/*	Sets a Variable based on an Array Rarity Based Structs with a Stream */
#define GET_FROM_WEIGHTED_STRUCT_WITH_SEED(Type, Array, Seed) \
[](TArray<Type> PassedInArray, const int32 InSeed) \
{ \
TArray<float> Weights; \
Weights.Reserve(PassedInArray.Num()); \
for(const Type& Element : PassedInArray) \
{ \
Weights.Add(Element.Weight); \
} \
const int32 Index = FCorePCGWeightedRandomHelpers::Get(Weights, InSeed); \
return PassedInArray[Index]; \
}(Array, Seed);

struct FCorePCGWeightedRandomHelpers
{
	/* Returns the Index of an array of Weights */
	static int32 Get(const TArray<float>& In)
	{
		float Sum = 0.f;

		for(const float Item : In)
		{
			Sum = Sum + Item;
		}
		
		float RandomValue = FMath::FRandRange(0.f, Sum);

		for(int32 Index = 0; Index < In.Num(); Index++)
		{
			if(RandomValue < In[Index])
			{
				return Index;
			}
			else
			{
				RandomValue = RandomValue - In[Index];
			}
		}

		/* If we get to this point then something was wrong */
		for (float Rar : In)
		{
			/* First check to see if all values are 0 */
			ensureMsgf(Rar > 0.f, TEXT("Weights Array cannot have values <= to 0"));
		}

		/* Then check if the array is empty */
		ensureMsgf(!In.IsEmpty(), TEXT("Cannot Generate a Weight on an Empty Array"));
		
		return -1;
	}
	
	/* Returns the Index of an array of Rarities */
	static int32 Get(const TArray<float>& In, const FRandomStream& Stream)
	{
		// Get Sum of Array
		float Sum = 0.f;
		for(const float Item : In) Sum = Sum + Item;
		
		// Generate Random Number
		float RandomValue = Stream.FRandRange(0.f, Sum);
		
		for(int32 Index = 0; Index < In.Num(); Index++)
		{
			if(RandomValue < In[Index])
			{
				return Index;
			}
			else
			{
				RandomValue = RandomValue - In[Index];
			}
		}
		
		checkf(false, TEXT("Cannot Generate a Weight on an Empty Array"));
		return 0;
	}

	/* Returns the Index of an array of Weights using the specified Seed */
	static int32 Get(const TArray<float>& In, const int32 InSeed)
	{
		return Get(In, FRandomStream(InSeed));
	}
	
	static FRandomStream NewStream()
	{
		FRandomStream Stream;
		Stream.GenerateNewSeed();
		return Stream;
	}
	static FRandomStream NewStream(const int32 InSeed)
	{
		return FRandomStream(InSeed);
	}
};