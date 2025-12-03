#include "PhysicsStressTest.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Components/PrimitiveComponent.h"

APhysicsStressTest::APhysicsStressTest()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void APhysicsStressTest::BeginPlay()
{
    Super::BeginPlay();
    RunTest();
}

void APhysicsStressTest::RunTest()
{
    for (AActor* Actor : SpawnedActors)
    {
        if (Actor) Actor->Destroy();
    }
    SpawnedActors.Empty();

    CSVContent = "Time,ObjectCount,FPS,FrameTime(ms),TotalMemory(MB),ActiveObjects\n";

    if (!ObjectToSpawn)
    {
        UE_LOG(LogTemp, Error, TEXT("STOP! ObjectToSpawn is empty."));
        return;
    }

    CurrentSpawnCount = 0;
    bIsSpawning = true;  
    bIsRecording = false;  
    Timer = 0.0f;

    UE_LOG(LogTemp, Warning, TEXT("Starting Stream Spawn..."));
}

void APhysicsStressTest::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsSpawning)
    {
        int32 RowSize = 10;

        for (int32 i = 0; i < SpawnsPerFrame; i++)
        {
            if (CurrentSpawnCount >= TargetObjectCount)
            {
                bIsSpawning = false;
                bIsRecording = true;
                Timer = 0.0f;
                UE_LOG(LogTemp, Warning, TEXT("Spawning Finished. Starting Recording."));
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

    if (!bIsRecording) return;

    Timer += DeltaTime;

    if (Timer >= MaxRecordingTime)
    {
        bIsRecording = false;
        SaveData();
        return;
    }

    float FPS = 1.0f / DeltaTime;
    float FrameTimeMS = DeltaTime * 1000.0f;

    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    int32 TotalMemoryMB = MemStats.UsedPhysical / (1024 * 1024);

    int32 ActiveCount = 0;
    for (AActor* Actor : SpawnedActors)
    {
        if (Actor)
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
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
    FFileHelper::SaveStringToFile(CSVContent, *FilePath);
    UE_LOG(LogTemp, Warning, TEXT("Test Complete. Data saved to: %s"), *FilePath);
}