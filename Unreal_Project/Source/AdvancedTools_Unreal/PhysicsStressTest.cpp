#include "PhysicsStressTest.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Components/PrimitiveComponent.h"
#include "HAL/PlatformFilemanager.h"

APhysicsStressTest::APhysicsStressTest()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void APhysicsStressTest::BeginPlay()
{
    // Force VSync OFF
    GEngine->Exec(GetWorld(), TEXT("r.VSync 0"));
    Super::BeginPlay();

    FString FilePath = FPaths::ProjectSavedDir() + "Logs/" + FileName;

    // Only write the Header if the file DOES NOT exist yet
    if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
    {
        FString Header = "Time,ObjectCount,FPS,FrameTime(ms),TotalMemory(MB),ActiveObjects\n";
        FFileHelper::SaveStringToFile(Header, *FilePath);
    }
}

void APhysicsStressTest::StartTest(int32 NewCount)
{
    TargetObjectCount = NewCount;

    for (AActor* Actor : SpawnedActors)
    {
        if (Actor) Actor->Destroy();
    }
    SpawnedActors.Empty();

    CSVContent.Empty();

    CurrentSpawnCount = 0;
    isSpawning = true;
    isRecording = false;
    Timer = 0.0f;

    UE_LOG(LogTemp, Warning, TEXT("Starting Test with %d Objects..."), TargetObjectCount);
}

void APhysicsStressTest::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // SPAWNING
    if (isSpawning)
    {
        int32 RowSize = 10;
        for (int32 i = 0; i < SpawnsPerFrame; i++)
        {
            if (CurrentSpawnCount >= TargetObjectCount)
            {
                isSpawning = false;
                isRecording = true;
                Timer = 0.0f;
                return;
            }

            float X = (CurrentSpawnCount % RowSize) * Spacing;
            float Y = 0.0f;
            float Z = 500.0f + ((CurrentSpawnCount / RowSize) * Spacing);
            X += FMath::RandRange(-10.0f, 10.0f);

            FVector SpawnLocation(X, Y, Z);
            FRotator SpawnRotation = FRotator::ZeroRotator;

            AActor* NewActor = GetWorld()->SpawnActor<AActor>(ObjectToSpawn, SpawnLocation, SpawnRotation);
            if (NewActor)
            {
                SpawnedActors.Add(NewActor);
            }
            CurrentSpawnCount++;
        }
        return;
    }

    // RECORDING DATA 
    if (!isRecording) return;

    Timer += DeltaTime;

    if (Timer >= MaxRecordingTime)
    {
        isRecording = false;
        SaveData();
        return;
    }

    float FPS = 1.0f / DeltaTime;
    float FrameTimeMS = DeltaTime * 1000.0f;

    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    int32 TotalMemoryMB = MemStats.UsedPhysical / (1024 * 1024);

    // Search for the Mesh Component 
    int32 ActiveCount = 0;
    for (AActor* Actor : SpawnedActors)
    {
        if (Actor)
        {
            UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
            if (PrimComp && PrimComp->IsAnyRigidBodyAwake())
            {
                ActiveCount++;
            }
        }
    }

    FString Line = FString::Printf(TEXT("%.2f,%d,%.1f,%.4f,%d,%d\n"),
        Timer, TargetObjectCount, FPS, FrameTimeMS, TotalMemoryMB, ActiveCount);

    CSVContent += Line;
}

void APhysicsStressTest::SaveData()
{
    FString FilePath = FPaths::ProjectSavedDir() + "Logs/" + FileName;
    FFileHelper::SaveStringToFile(CSVContent, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);

    UE_LOG(LogTemp, Warning, TEXT("Test Complete. Data APPENDED to: %s"), *FilePath);
}