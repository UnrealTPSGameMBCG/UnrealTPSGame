// My game copyright

#include "Tests/MyScienceFuncLibTests.h"
#include "Misc/AutomationTest.h"
#include "Science/MyScienceFuncLib.h"
#include "Tests/MyTestUtils.h"

// exclude code for testing from Shipping builds
#if (WITH_AUTOMATION_TESTS)  // == (WITH_DEV_AUTOMATION_TESTS || WITH_PERF_AUTOMATION_TESTS)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMyFibonacciSimple, "TPSGame.MyScience.Fibonacci.Simple",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::LowPriority);

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMyFibonacciLogHasErrors, "TPSGame.MyScience.Fibonacci.LogHasErrors",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMyMathSqrt, "TPSGame.MyMath.Sqrt",
    // COP: to test pull request verification
    // EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::LowPriority); 
    
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter |
        EAutomationTestFlags::CriticalPriority);  ///< I set this CriticalPriority is to ensure that there is at least one Critical to let
                                                  ///< Jenkins verifier job work properly

bool FMyFibonacciSimple::RunTest(const FString& Parameters)
{
    AddInfo("UMyScienceFuncLib::Fibonacci simple testing");

    using FibonacciArray = TArray<TPSGame::TestPayload<int32, int32>>;

    // clang-format off
    // fibonacci sequence: 0 1 1 2 3 5 8 ... 
    const FibonacciArray FibonacciTestPayloadData 
    {  
        { 0, 0 },
        { 1, 1 },
        { 2, 1 },
        { 3, 2 },
        { 4, 3 },
        { 5, 5 },
        { 6, 8 }
    };
    // clang-format on

    for (const auto FibonaciTestPair : FibonacciTestPayloadData)
    {
        const FString InfoString =
            FString::Printf(TEXT("Fibonacci input: %i, Expected result: %i"), FibonaciTestPair.TestValue, FibonaciTestPair.ExpectedValue);
        TestEqual(InfoString, UMyScienceFuncLib::Fibonacci(FibonaciTestPair.TestValue), FibonaciTestPair.ExpectedValue);
    }

    /*
    * // we use TArray above instead
    TestTrueExpr(UMyScienceFuncLib::Fibonacci(0) == 0);
    TestTrueExpr(UMyScienceFuncLib::Fibonacci(1) == 1);
    TestTrueExpr(UMyScienceFuncLib::Fibonacci(2) == 1);
    TestTrueExpr(UMyScienceFuncLib::Fibonacci(3) == 2);
    TestTrueExpr(UMyScienceFuncLib::Fibonacci(4) == 3);
    TestTrueExpr(UMyScienceFuncLib::Fibonacci(5) == 5);
    */

    return true;
}

bool FMyFibonacciLogHasErrors::RunTest(const FString& Parameters)
{
    AddInfo("UMyScienceFuncLib::Fibonacci with negative input should output error in logs");
    AddExpectedError("Invalid input for Fibonacci", EAutomationExpectedMessageFlags::Contains, 2);

    UMyScienceFuncLib::Fibonacci(-10);
    UMyScienceFuncLib::Fibonacci(1);
    UMyScienceFuncLib::Fibonacci(-13);

    return true;
}

bool FMyMathSqrt::RunTest(const FString& Parameters)
{
    AddInfo("MathSqrt simple testing");

    using SqrtPayloadData = TArray<TPSGame::TestPayload<float, float>>;

    // clang-format off
    const SqrtPayloadData SqrtTestPayloadData 
    {  
        //{ -1.0f, 0.0f },
        { 4.0f, 123.0f }, // to cause error
        { 0.0f, 0.0f },
        { 1.0f, 1.0f },
        { 4.0f, 2.0f },
        { 3.0f, 1.7f, 0.1f },
        { 3.0f, 1.73f, 0.01f },
        { 3.0f, 1.73205f, 1e-5f }
    };
    // clang-format on

    for (const auto SqrtTestSample : SqrtTestPayloadData)
    {
        const bool isNearlyEqual =
            FMath::IsNearlyEqual(FMath::Sqrt(SqrtTestSample.TestValue), SqrtTestSample.ExpectedValue, SqrtTestSample.Tolerance);
        TestTrueExpr(isNearlyEqual);
    }

    return true;
}

#endif  // exclude code for testing from Shipping builds