// My game copyright

#include "Science/MyScienceFuncLib.h"

DEFINE_LOG_CATEGORY_STATIC(MyScienceFuncLib, All, All);

int32 UMyScienceFuncLib::Fibonacci(const int32 Value)
{
    // checkf(Value > 0, TEXT("Invalid input for Fibonacci. Input value must be integer value > 0, whereas the input value is %i"), Value);
    // // ok check(Value > 0); // ok

    if (Value < 0)
    {
        UE_LOG(MyScienceFuncLib, Error,
            TEXT("Invalid input for Fibonacci. Input value must be integer value > 0, whereas the input value is %i"), Value);
    }

    return Value <= 1 ? Value : Fibonacci(Value - 1) + Fibonacci(Value - 2);
}