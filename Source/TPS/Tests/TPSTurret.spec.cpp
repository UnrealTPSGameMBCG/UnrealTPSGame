// My game copyright

#if WITH_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "TPS/Tests/TestUtils.h"
#include "Weapon/TPSTurret.h"

BEGIN_DEFINE_SPEC(FTurret, "TPSGame.Turret",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority)
UWorld* World;
ATPSTurret* Turret;
const FTransform InitialTransform{FVector{0.0f, 286.0f, 110.0f}};
END_DEFINE_SPEC(FTurret)

using namespace TPS::Test;

namespace
{
constexpr char* MapName = "/Game/Tests/EmptyTestLevel";
constexpr char* TurretBPName = "Blueprint'/Game/Weapon/BP_TPSTurret.BP_TPSTurret'";
constexpr char* TurretBPTestName = "Blueprint'/Game/Tests/BP_Test_TPSTurret.BP_Test_TPSTurret'";

}  // namespace

void FTurret::Define()
{
    Describe("Creational",
        [this]()
        {
            BeforeEach(
                [this]()
                {
                    AutomationOpenMap(MapName);
                    World = GetTestGameWorld();
                    TestNotNull("World exists", World);
                });

            It("Cpp instance can't be created",
                [this]()
                {
                    const FString ExpectedWarnMsg =
                        FString::Printf(TEXT("SpawnActor failed because class %s is abstract"), *ATPSTurret::StaticClass()->GetName());
                    AddExpectedError(ExpectedWarnMsg, EAutomationExpectedErrorFlags::Exact);

                    Turret = World->SpawnActor<ATPSTurret>(ATPSTurret::StaticClass(), InitialTransform);
                    TestNull("Turret doesn't exist", Turret);
                });

            It("Blueprint instance can be created",
                [this]()
                {
                    Turret = CreateBlueprint<ATPSTurret>(World, TurretBPName, InitialTransform);
                    TestNotNull("Turret exists", Turret);
                });

            AfterEach([this]() { SpecCloseLevel(World); });
        });

    Describe("Defaults",
        [this]()
        {
            BeforeEach(
                [this]()
                {
                    AutomationOpenMap(MapName);
                    World = GetTestGameWorld();
                    TestNotNull("World exists", World);

                    Turret = CreateBlueprint<ATPSTurret>(World, TurretBPTestName, InitialTransform);
                    TestNotNull("Turret exists", Turret);
                });

            const TArray<TTuple<int32, float>> TestData{{45, 2.0f}, {15, 3.0f}, {5, 5.0f}};
            for (const auto& Data : TestData)
            {
                const auto TestName = FString::Printf(TEXT("Ammo: %i and freq: %.0f should be set up correctly"), Data.Key, Data.Value);
                It(TestName,
                    [this, Data]()
                    {
                        const auto [Ammo, Freq] = Data;
                        CallFuncByNameWithParams(Turret, "SetTurretData", {FString::FromInt(Ammo), FString::SanitizeFloat(Freq)});

                        const int32 AmmoCount = GetPropertyValueByName<ATPSTurret, int32>(Turret, "AmmoCount");
                        TestTrueExpr(AmmoCount == Ammo);

                        const float FireFrequency = GetPropertyValueByName<ATPSTurret, float>(Turret, "FireFrequency");
                        TestTrueExpr(FireFrequency == Freq);
                    });
            }

            AfterEach([this]() { SpecCloseLevel(World); });
        });

    Describe("Ammo (UE bug)",
        [this]()
        {
            const int32 InitialAmmoCount = 7;
            const float FireFreq = 0.5f;
            LatentBeforeEach(
                [this, InitialAmmoCount, FireFreq](const FDoneDelegate& TestDone)
                {
                    AutomationOpenMap(MapName);
                    World = GetTestGameWorld();
                    TestNotNull("World exists", World);

                    Turret = CreateBlueprint<ATPSTurret>(World, TurretBPTestName, InitialTransform);
                    TestNotNull("Turret exists", Turret);
                    CallFuncByNameWithParams(
                        Turret, "SetTurretData", {FString::FromInt(InitialAmmoCount), FString::SanitizeFloat(FireFreq)});
                    TestDone.Execute();
                });

            // "x" is for we don't need this test to be run in UE
            LatentIt(FString::Printf(TEXT("Should be empty after %i sec"), FMath::RoundToInt(InitialAmmoCount * FireFreq)),
                EAsyncExecution::ThreadPool,
                [this, InitialAmmoCount, FireFreq](const FDoneDelegate& TestDone)
                {
                    AsyncTask(ENamedThreads::GameThread,
                        [&]()
                        {
                            const int32 AmmoCount = GetPropertyValueByName<ATPSTurret, int32>(Turret, "AmmoCount");
                            TestTrueExpr(AmmoCount == InitialAmmoCount);

                            const FTimerHandle FireTimerHandle =
                                GetPropertyValueByName<ATPSTurret, FTimerHandle>(Turret, "FireTimerHandle");
                            TestTrueExpr(World->GetTimerManager().IsTimerActive(FireTimerHandle));
                        });

                    const float SynchDelta = 0.5f;
                    FPlatformProcess::Sleep(InitialAmmoCount * FireFreq + SynchDelta);

                    AsyncTask(ENamedThreads::GameThread,
                        [&]()
                        {
                            const int32 AmmoCount = GetPropertyValueByName<ATPSTurret, int32>(Turret, "AmmoCount");
                            TestTrueExpr(AmmoCount == 0);
                            const FTimerHandle FireTimerHandle =
                                GetPropertyValueByName<ATPSTurret, FTimerHandle>(Turret, "FireTimerHandle");
                            TestTrueExpr(!World->GetTimerManager().IsTimerActive(FireTimerHandle));
                        });

                    TestDone.Execute();
                });

            LatentAfterEach(
                [this](const FDoneDelegate& TestDone)
                {
                    SpecCloseLevel(World);
                    TestDone.Execute();
                });
        });

    // Bugfix: to use CreateBlueprintDeferred instead of CreateBlueprint so that SetTurretData takes effect on the spawned BP
    Describe("Ammo (bug fixed) - latent - OK",
        [this]()
        {
            const int32 InitialAmmoCount = 7;
            const float FireFreq = 0.5f;
            LatentBeforeEach(
                // Using EAsyncExecution to create FAsyncUntilDoneLatentCommand instead of FUntilDoneLatentCommand
                EAsyncExecution::ThreadPool,
                [this, InitialAmmoCount, FireFreq](const FDoneDelegate& TestDone)
                {
                    AsyncTask(ENamedThreads::GameThread,
                        [&]()
                        {
                            AutomationOpenMap(MapName);
                            World = GetTestGameWorld();
                            TestNotNull("World exists", World);

                            // let's try CreateBlueprintDeferred to see if this will make firing frequency correct in BP
                            // Turret = CreateBlueprint<ATPSTurret>(World, TurretBPTestName, InitialTransform);
                            Turret = CreateBlueprintDeferred<ATPSTurret>(World, TurretBPTestName, InitialTransform);
                            TestNotNull("Turret exists", Turret);
                            CallFuncByNameWithParams(
                                Turret, "SetTurretData", {FString::FromInt(InitialAmmoCount), FString::SanitizeFloat(FireFreq)});
                            // I don't understand why "Turret->FinishSpawning(InitialTransform);" is a valid because FinishSpawning requires
                            // more than one parameter. Turret->FinishSpawning(InitialTransform);
                            Turret->FinishSpawning(InitialTransform, false, nullptr, ESpawnActorScaleMethod::OverrideRootScale);
                            UE_LOG(LogTemp, Display, TEXT("Turret: LatentBeforeEach - AsyncTask 1"));
                        });

                    // SynchDelta is for overhead happening in GameThread
                    // const float SynchDelta = 0.5f;
                    // FPlatformProcess::Sleep(SynchDelta); // probably this cuases bug: while we wailt turrent already starts firing.
                    // No,it'not, We don't need to sleep
                    TestDone.Execute();
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentBeforeEach - finished"));
                });

            LatentIt(FString::Printf(TEXT("Should be empty after %i sec"), FMath::RoundToInt(InitialAmmoCount * FireFreq)),
                EAsyncExecution::ThreadPool,
                [this, InitialAmmoCount, FireFreq](const FDoneDelegate& TestDone)
                {
                    AsyncTask(ENamedThreads::GameThread,
                        [&]()
                        {
                            const int32 AmmoCount = GetPropertyValueByName<ATPSTurret, int32>(Turret, "AmmoCount");
                            TestTrueExpr(AmmoCount == InitialAmmoCount);

                            const FTimerHandle FireTimerHandle =
                                GetPropertyValueByName<ATPSTurret, FTimerHandle>(Turret, "FireTimerHandle");
                            TestTrueExpr(World->GetTimerManager().IsTimerActive(FireTimerHandle));

                            UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - AsyncTask 2"));
                        });

                    // SynchDelta is for possible overhead happening in GameThread
                    const float SynchDelta = 0.5f;
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - before sleep"));
                    // sleep runs not in the Game thread, that's why we need to run it here to sync with what runs in the Game thread
                    FPlatformProcess::Sleep(InitialAmmoCount * FireFreq + SynchDelta);
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - after sleep"));

                    AsyncTask(ENamedThreads::GameThread,
                        [&]()
                        {
                            const int32 AmmoCount = GetPropertyValueByName<ATPSTurret, int32>(Turret, "AmmoCount");
                            TestTrueExpr(AmmoCount == 0);
                            const FTimerHandle FireTimerHandle =
                                GetPropertyValueByName<ATPSTurret, FTimerHandle>(Turret, "FireTimerHandle");
                            TestTrueExpr(!World->GetTimerManager().IsTimerActive(FireTimerHandle));

                            UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - AsyncTask 3"));
                        });

                    TestDone.Execute();
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - finished"));
                });

            LatentAfterEach(
                // using EAsyncExecution to create FAsyncUntilDoneLatentCommand instead of FUntilDoneLatentCommand
                EAsyncExecution::ThreadPool,  //
                [this](const FDoneDelegate& TestDone)
                {
                    AsyncTask(ENamedThreads::GameThread,
                        [&]()
                        {
                            // SpecCloseLevel closes the map but the testing process continues, we need to use a latent command
                            // SpecCloseLevel(World);
                            ADD_LATENT_AUTOMATION_COMMAND(FExitGameCommand);

                            UE_LOG(LogTemp, Display, TEXT("Turret: LatentAfterEach - AsyncTask 4"));
                        });

                    // SynchDelta is for overhead happening in GameThread
                    const float SynchDelta = 0.5f;
                    // FPlatformProcess::Sleep(SynchDelta); // probably not needed

                    TestDone.Execute();
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentAfterEach - finished"));
                });
        });

    Describe("Ammo - no AsyncTask in LatentBeforeEach (throws exception)",
        [this]()
        {
            const int32 InitialAmmoCount = 7;
            const float FireFreq = 0.5f;
            LatentBeforeEach(
                // Using EAsyncExecution to create FAsyncUntilDoneLatentCommand instead of FUntilDoneLatentCommand
                EAsyncExecution::ThreadPool,
                [this, InitialAmmoCount, FireFreq](const FDoneDelegate& TestDone)
                {
                    AutomationOpenMap(MapName);
                    World = GetTestGameWorld();
                    TestNotNull("World exists", World);

                    // let's try CreateBlueprintDeferred to see if this will make firing frequency correct in BP
                    // Turret = CreateBlueprint<ATPSTurret>(World, TurretBPTestName, InitialTransform);
                    Turret = CreateBlueprintDeferred<ATPSTurret>(World, TurretBPTestName, InitialTransform);
                    TestNotNull("Turret exists", Turret);
                    CallFuncByNameWithParams(
                        Turret, "SetTurretData", {FString::FromInt(InitialAmmoCount), FString::SanitizeFloat(FireFreq)});
                    // I don't understand why "Turret->FinishSpawning(InitialTransform);" is a valid because FinishSpawning requires
                    // more than one parameter. Turret->FinishSpawning(InitialTransform);
                    Turret->FinishSpawning(InitialTransform, false, nullptr, ESpawnActorScaleMethod::OverrideRootScale);
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentBeforeEach - AsyncTask 1"));

                    // SynchDelta is for overhead happening in GameThread
                    // const float SynchDelta = 0.5f;
                    // FPlatformProcess::Sleep(SynchDelta); // probably this cuases bug: while we wailt turrent already starts firing.
                    // No,it'not, We don't need to sleep
                    TestDone.Execute();
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentBeforeEach - finished"));
                });

            LatentIt(FString::Printf(TEXT("Should be empty after %i sec"), FMath::RoundToInt(InitialAmmoCount * FireFreq)),
                EAsyncExecution::ThreadPool,
                [this, InitialAmmoCount, FireFreq](const FDoneDelegate& TestDone)
                {
                    AsyncTask(ENamedThreads::GameThread,
                        [&]()
                        {
                            const int32 AmmoCount = GetPropertyValueByName<ATPSTurret, int32>(Turret, "AmmoCount");
                            TestTrueExpr(AmmoCount == InitialAmmoCount);

                            const FTimerHandle FireTimerHandle =
                                GetPropertyValueByName<ATPSTurret, FTimerHandle>(Turret, "FireTimerHandle");
                            TestTrueExpr(World->GetTimerManager().IsTimerActive(FireTimerHandle));

                            UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - AsyncTask 2"));
                        });

                    // SynchDelta is for possible overhead happening in GameThread
                    const float SynchDelta = 0.5f;
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - before sleep"));
                    // sleep runs not in the Game thread, that's why we need to run it here to sync with what runs in the Game thread
                    FPlatformProcess::Sleep(InitialAmmoCount * FireFreq + SynchDelta);
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - after sleep"));

                    AsyncTask(ENamedThreads::GameThread,
                        [&]()
                        {
                            const int32 AmmoCount = GetPropertyValueByName<ATPSTurret, int32>(Turret, "AmmoCount");
                            TestTrueExpr(AmmoCount == 0);
                            const FTimerHandle FireTimerHandle =
                                GetPropertyValueByName<ATPSTurret, FTimerHandle>(Turret, "FireTimerHandle");
                            TestTrueExpr(!World->GetTimerManager().IsTimerActive(FireTimerHandle));

                            UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - AsyncTask 3"));
                        });

                    TestDone.Execute();
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - finished"));
                });

            LatentAfterEach(
                // using EAsyncExecution to create FAsyncUntilDoneLatentCommand instead of FUntilDoneLatentCommand
                EAsyncExecution::ThreadPool,  //
                [this](const FDoneDelegate& TestDone)
                {
                    AsyncTask(ENamedThreads::GameThread,
                        [&]()
                        {
                            // SpecCloseLevel closes the map but the testing process continues, we need to use a latent command
                            // SpecCloseLevel(World);
                            ADD_LATENT_AUTOMATION_COMMAND(FExitGameCommand);

                            UE_LOG(LogTemp, Display, TEXT("Turret: LatentAfterEach - AsyncTask 4"));
                        });

                    // SynchDelta is for overhead happening in GameThread
                    const float SynchDelta = 0.5f;
                    // FPlatformProcess::Sleep(SynchDelta); // probably not needed

                    TestDone.Execute();
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentAfterEach - finished"));
                });
        });

    Describe("Ammo (not latent close of map)",
        [this]()
        {
            const int32 InitialAmmoCount = 7;
            const float FireFreq = 0.5f;
            LatentBeforeEach(
                // Using EAsyncExecution to create FAsyncUntilDoneLatentCommand instead of FUntilDoneLatentCommand
                EAsyncExecution::ThreadPool,
                [this, InitialAmmoCount, FireFreq](const FDoneDelegate& TestDone)
                {
                    AsyncTask(ENamedThreads::GameThread,
                        [&]()
                        {
                            AutomationOpenMap(MapName);
                            World = GetTestGameWorld();
                            TestNotNull("World exists", World);

                            // let's try CreateBlueprintDeferred to see if this will make firing frequency correct in BP
                            // Turret = CreateBlueprint<ATPSTurret>(World, TurretBPTestName, InitialTransform);
                            Turret = CreateBlueprintDeferred<ATPSTurret>(World, TurretBPTestName, InitialTransform);
                            TestNotNull("Turret exists", Turret);
                            CallFuncByNameWithParams(
                                Turret, "SetTurretData", {FString::FromInt(InitialAmmoCount), FString::SanitizeFloat(FireFreq)});
                            // I don't understand why "Turret->FinishSpawning(InitialTransform);" is a valid because FinishSpawning requires
                            // more than one parameter. Turret->FinishSpawning(InitialTransform);
                            Turret->FinishSpawning(InitialTransform, false, nullptr, ESpawnActorScaleMethod::OverrideRootScale);
                            UE_LOG(LogTemp, Display, TEXT("Turret: LatentBeforeEach - AsyncTask 1"));
                        });

                    // SynchDelta is for overhead happening in GameThread
                    const float SynchDelta = 0.5f;
                    // FPlatformProcess::Sleep(SynchDelta); // probably this cuases bug: while we wailt turrent already starts firing
                    TestDone.Execute();
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentBeforeEach - finished"));
                });

            LatentIt(FString::Printf(TEXT("Should be empty after %i sec"), FMath::RoundToInt(InitialAmmoCount * FireFreq)),
                EAsyncExecution::ThreadPool,
                [this, InitialAmmoCount, FireFreq](const FDoneDelegate& TestDone)
                {
                    AsyncTask(ENamedThreads::GameThread,
                        [&]()
                        {
                            const int32 AmmoCount = GetPropertyValueByName<ATPSTurret, int32>(Turret, "AmmoCount");
                            TestTrueExpr(AmmoCount == InitialAmmoCount);

                            const FTimerHandle FireTimerHandle =
                                GetPropertyValueByName<ATPSTurret, FTimerHandle>(Turret, "FireTimerHandle");
                            TestTrueExpr(World->GetTimerManager().IsTimerActive(FireTimerHandle));

                            UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - AsyncTask 2"));
                        });

                    // SynchDelta is for possible overhead happening in GameThread
                    const float SynchDelta = 0.5f;
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - before sleep"));
                    // sleep runs not in the Game thread, that's why we need to run it here to sync with what runs in the Game thread
                    FPlatformProcess::Sleep(InitialAmmoCount * FireFreq + SynchDelta);
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - after sleep"));

                    AsyncTask(ENamedThreads::GameThread,
                        [&]()
                        {
                            const int32 AmmoCount = GetPropertyValueByName<ATPSTurret, int32>(Turret, "AmmoCount");
                            TestTrueExpr(AmmoCount == 0);
                            const FTimerHandle FireTimerHandle =
                                GetPropertyValueByName<ATPSTurret, FTimerHandle>(Turret, "FireTimerHandle");
                            TestTrueExpr(!World->GetTimerManager().IsTimerActive(FireTimerHandle));

                            UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - AsyncTask 3"));
                        });

                    TestDone.Execute();
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - finished"));
                });

            LatentAfterEach(
                // using EAsyncExecution to create FAsyncUntilDoneLatentCommand instead of FUntilDoneLatentCommand
                EAsyncExecution::ThreadPool,  //
                [this](const FDoneDelegate& TestDone)
                {
                    AsyncTask(ENamedThreads::GameThread,
                        [&]()
                        {
                            // SpecCloseLevel closes the map but the testing process continues, we need to use a latent command
                            SpecCloseLevel(World);  // not OK
                            // ADD_LATENT_AUTOMATION_COMMAND(FExitGameCommand); // OK

                            UE_LOG(LogTemp, Display, TEXT("Turret: LatentAfterEach - AsyncTask 4"));
                        });

                    // SynchDelta is for overhead happening in GameThread
                    const float SynchDelta = 0.5f;
                    // FPlatformProcess::Sleep(SynchDelta); // probably not needed

                    TestDone.Execute();
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentAfterEach - finished"));
                });
        });

    Describe("Ammo (bug fixing) - 2: not latent BeforeEach",
        [this]()
        {
            const int32 InitialAmmoCount = 10;
            const float FireFreq = 0.5f;

            BeforeEach(
                [this, InitialAmmoCount, FireFreq]()
                {
                    AutomationOpenMap(MapName);
                    World = GetTestGameWorld();
                    TestNotNull("World exists", World);

                    Turret = CreateBlueprint<ATPSTurret>(World, TurretBPTestName, InitialTransform);
                    TestNotNull("Turret exists", Turret);
                    CallFuncByNameWithParams(
                        Turret, "SetTurretData", {FString::FromInt(InitialAmmoCount), FString::SanitizeFloat(FireFreq)});

                    UE_LOG(LogTemp, Display, TEXT("Turret: BeforeEach"));
                });

            LatentIt(FString::Printf(TEXT("Should be empty after %i sec"), FMath::RoundToInt(InitialAmmoCount * FireFreq)),
                EAsyncExecution::ThreadPool,
                [this, InitialAmmoCount, FireFreq](const FDoneDelegate& TestDone)
                {
                    AsyncTask(ENamedThreads::GameThread,
                        [&]()
                        {
                            const int32 AmmoCount = GetPropertyValueByName<ATPSTurret, int32>(Turret, "AmmoCount");
                            TestTrueExpr(AmmoCount == InitialAmmoCount);

                            const FTimerHandle FireTimerHandle =
                                GetPropertyValueByName<ATPSTurret, FTimerHandle>(Turret, "FireTimerHandle");
                            TestTrueExpr(World->GetTimerManager().IsTimerActive(FireTimerHandle));

                            UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - AsyncTask 2"));
                        });

                    // SynchDelta is for overhead happening in GameThread
                    const float SynchDelta = 0.5f;
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - before sleep"));
                    // sleep runs not in the Game thread, that's why we need to run it to sync with what runs in the Game thread
                    FPlatformProcess::Sleep(InitialAmmoCount * FireFreq + SynchDelta);
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - after sleep"));

                    AsyncTask(ENamedThreads::GameThread,
                        [&]()
                        {
                            const int32 AmmoCount = GetPropertyValueByName<ATPSTurret, int32>(Turret, "AmmoCount");
                            TestTrueExpr(AmmoCount == 0);
                            const FTimerHandle FireTimerHandle =
                                GetPropertyValueByName<ATPSTurret, FTimerHandle>(Turret, "FireTimerHandle");
                            TestTrueExpr(!World->GetTimerManager().IsTimerActive(FireTimerHandle));

                            UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - AsyncTask 3"));
                        });

                    TestDone.Execute();
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - finished"));
                });

            LatentAfterEach(
                // using EAsyncExecution to create FAsyncUntilDoneLatentCommand instead of FUntilDoneLatentCommand
                EAsyncExecution::ThreadPool,  //
                [this](const FDoneDelegate& TestDone)
                {
                    AsyncTask(ENamedThreads::GameThread,
                        [&]()
                        {
                            // trying to figure out what causes bug
                            // SpecCloseLevel(World);
                            ADD_LATENT_AUTOMATION_COMMAND(FExitGameCommand);

                            UE_LOG(LogTemp, Display, TEXT("Turret: LatentAfterEach - AsyncTask 4"));
                        });

                    // SynchDelta is for overhead happening in GameThread
                    const float SynchDelta = 0.5f;
                    // FPlatformProcess::Sleep(SynchDelta); // probably not needed

                    TestDone.Execute();
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentAfterEach - finished"));
                });
        });

    // mike: trying to debug why LatentIt does not work in UE5.3.
    // .. see bug in FUntilDoneLatentCommand https://forums.unrealengine.com/t/automated-tests-are-broken-ue-5-3/1371033
    Describe("Debug - open close map - OK",
        [this]()
        {
            const int32 InitialAmmoCount = 1;
            const float FireFreq = 1.5f;
            LatentBeforeEach(
                // using EAsyncExecution to create FAsyncUntilDoneLatentCommand instead of broken FUntilDoneLatentCommand
                EAsyncExecution::ThreadPool,
                [this, InitialAmmoCount, FireFreq](const FDoneDelegate& TestDone)
                {
                    AsyncTask(ENamedThreads::GameThread,
                        [&]()
                        {
                            AutomationOpenMap(MapName);
                            UE_LOG(LogTemp, Display, TEXT("Turret: LatentBeforeEach - finished"));
                        });
                    TestDone.Execute();
                });

            // testing if "It" works better than "LatentIt"
            LatentIt(FString::Printf(TEXT("Should simply run")),  //
                EAsyncExecution::ThreadPool,                      //
                FTimespan::FromSeconds(5.0),                      // shorter timeout for my test
                [this, InitialAmmoCount, FireFreq](const FDoneDelegate& TestDone)
                {
                    AsyncTask(ENamedThreads::GameThread, [&]() { UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - AsyncTask 1")); });

                    AsyncTask(ENamedThreads::GameThread, [&]() { UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - AsyncTask 2")); });

                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - Sleep started"));
                    const float SynchDelta = 0.5f;
                    FPlatformProcess::Sleep(InitialAmmoCount * FireFreq + SynchDelta);
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - Sleep finished"));

                    TestDone.Execute();
                });

            LatentAfterEach(
                // using EAsyncExecution to create FAsyncUntilDoneLatentCommand instead of broken FUntilDoneLatentCommand
                EAsyncExecution::ThreadPool,
                [this](const FDoneDelegate& TestDone)
                {
                    AsyncTask(ENamedThreads::GameThread,
                        [&]()
                        {
                            // SpecCloseLevel(World); // throws exception
                            ADD_LATENT_AUTOMATION_COMMAND(FExitGameCommand);  // OK, does not throw exception
                            UE_LOG(LogTemp, Display, TEXT("Turret: LatentAfterEach - AsyncTask 3"));
                        });

                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentAfterEach"));
                    TestDone.Execute();
                });
        });

    // mike: trying to debug why LatentIt does not work in UE5.3.
    // .. see bug in FUntilDoneLatentCommand https://forums.unrealengine.com/t/automated-tests-are-broken-ue-5-3/1371033
    Describe("Debug simplest latent - OK",
        [this]()
        {
            const int32 InitialAmmoCount = 1;
            const float FireFreq = 1.5f;
            LatentBeforeEach(
                // using EAsyncExecution to create FAsyncUntilDoneLatentCommand instead of broken FUntilDoneLatentCommand
                EAsyncExecution::ThreadPool,
                [this, InitialAmmoCount, FireFreq](const FDoneDelegate& TestDone)
                {
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentBeforeEach - before sleep"));
                    FPlatformProcess::Sleep(1.0f);  // sleep in sec
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentBeforeEach - after sleep"));
                    TestDone.Execute();
                });

            // testing if "It" works better than "LatentIt"
            LatentIt(FString::Printf(TEXT("Should simply run")),  //
                EAsyncExecution::ThreadPool,                      //
                FTimespan::FromSeconds(5.0),                      // shorter timeout for my test
                [this, InitialAmmoCount, FireFreq](const FDoneDelegate& TestDone)
                {
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - Should simply run 1"));

                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - Sleep started"));
                    FPlatformProcess::Sleep(1.0f);  // sleep in sec
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentIt - Sleep finished"));

                    AsyncTask(ENamedThreads::GameThread, [&]() { UE_LOG(LogTemp, Display, TEXT("Turret: Should simply run 2")); });

                    TestDone.Execute();
                });

            LatentAfterEach(
                // using EAsyncExecution to create FAsyncUntilDoneLatentCommand instead of broken FUntilDoneLatentCommand
                EAsyncExecution::ThreadPool,
                [this](const FDoneDelegate& TestDone)
                {
                    UE_LOG(LogTemp, Display, TEXT("Turret: LatentAfterEach"));
                    TestDone.Execute();
                });
        });

    Describe("Debug simplest NoLatent - OK",
        [this]()
        {
            const int32 InitialAmmoCount = 1;
            const float FireFreq = 1.5f;
            BeforeEach([this, InitialAmmoCount, FireFreq]() { UE_LOG(LogTemp, Display, TEXT("Turret: BeforeEach - OK")); });

            It(FString::Printf(TEXT("Should simply run")),  //
                EAsyncExecution::ThreadPool,                // to use a specific It constructor
                FTimespan::FromSeconds(5.0),                // shorter timeout for my test
                [this, InitialAmmoCount, FireFreq]()
                {
                    // UE BUG: it does not go in here
                    UE_LOG(LogTemp, Display, TEXT("Turret: Should simply run 1 - OK"));

                    UE_LOG(LogTemp, Display, TEXT("Turret: Sleep started"));
                    FPlatformProcess::Sleep(1.5f);  // sleep in sec
                    UE_LOG(LogTemp, Display, TEXT("Turret: Sleep finished"));

                    // AsyncTask(ENamedThreads::GameThread, [&]() { UE_LOG(LogTemp, Display, TEXT("Turret: Should simply run 2 - OK")); });
                });

            AfterEach([this]() { UE_LOG(LogTemp, Display, TEXT("Turret: AfterEach - OK")); });
        });

    // END: mike: trying to debug why LatentIt does not work in UE5.3
}

#endif
