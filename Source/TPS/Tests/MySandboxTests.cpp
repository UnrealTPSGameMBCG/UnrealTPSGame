// My game copyright

// exclude code for testing from Shipping builds
#if (WITH_DEV_AUTOMATION_TESTS || WITH_PERF_AUTOMATION_TESTS)

#include "Tests/MySandboxTests.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMyMathSin, "TPSGame.MyMath.Sin",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

bool FMyMathSin::RunTest(const FString& Parameters)
{
    AddInfo("FMath::Sin testing");

    // just experimenting
    TestTrue("Sin(0) equals 0 [1]", FMath::Sin(0.0) == 0.0);
    TestTrueExpr(FMath::Sin(0.0) == 0.0);  // just to see difference between TestTrueExpr and TestTrue
    TestEqual("Sin(0) equals 0 [2]", FMath::Sin(0.0), 0.0);

    // output warning example
    if (FMath::Sin(0.0) != 0.1)
    {
        AddWarning("Just a warning example");
        return true;  // if return "false" then the test will fail, whereas true allows to use comment and warning.
    }

    return true;  // it should always return true, as test failures should be tracked in functions like TestTrue etc
}

/*
* // we don't need default-generated class
*
MySandboxTests::MySandboxTests()
{
}

MySandboxTests::~MySandboxTests()
{
}
*/

#endif  // (WITH_DEV_AUTOMATION_TESTS || WITH_PERF_AUTOMATION_TESTS)