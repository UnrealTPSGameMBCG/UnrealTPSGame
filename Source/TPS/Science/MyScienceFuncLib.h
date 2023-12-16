// My game copyright

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyScienceFuncLib.generated.h"

/**
 * My class for some science functions that I will test using unit testing
 */
UCLASS()
class TPS_API UMyScienceFuncLib : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = Science)
    static int32 Fibonacci(const int32 Value);
};
