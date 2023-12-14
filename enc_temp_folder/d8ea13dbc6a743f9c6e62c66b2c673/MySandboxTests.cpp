// My game copyright

#include "Tests/MySandboxTests.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMyMathSin, "TPSGame.Math.Sin",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

bool FMyMathSin::RunTest(const FString& Parameters)
{
    TestTrue("Sin(0) equals 0", FMath::Sin(0.0) == 0.0);
    return true;
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