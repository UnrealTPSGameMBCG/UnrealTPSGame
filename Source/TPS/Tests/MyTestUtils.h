#pragma once

#if WITH_AUTOMATION_TESTS

namespace TPSGame
{

template <typename Type1, typename Type2>
struct TestPayload
{
    Type1 TestValue;
    Type2 ExpectedValue;
    float Tolerance = KINDA_SMALL_NUMBER;  // for working with float data (e.g. with FMath::NearlyEqual() )
};

}  // namespace TPSGame

#endif  // WITH_AUTOMATION_TESTS