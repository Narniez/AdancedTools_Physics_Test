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

    FString FilePath = FPaths::ProjectSavedDir() + "Logs/" + fileName;

    // Only write the Header if the file doesnt exist yet
    if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
    {
        FString Header = "Time,ObjectCount,FPS,FrameTime(ms),TotalMemory(MB),ActiveObjects\n";
        FFileHelper::SaveStringToFile(Header, *FilePath);
    }
}

void APhysicsStressTest::StartTest(int NewCount)
{
    targetObjectCount = NewCount;

    for (AActor* Actor : spawnedActors)
    {
        if (Actor) Actor->Destroy();
    }
    spawnedActors.Empty();

    CSVContent.Empty();

    currentSpawnCount = 0;
    isSpawning = true;
    isRecording = true;
    Timer = 0.0f;

    UE_LOG(LogTemp, Warning, TEXT("Starting Test with %d Objects..."), targetObjectCount);
}

void APhysicsStressTest::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    //Spawning logic
    if (isSpawning)
    {
		//Spawn a set number of objects per frame to avoid engine freeze
        for (int i = 0; i < spawnsPerFrame && currentSpawnCount < targetObjectCount; i++)
        {
            float X = (currentSpawnCount % columnNumber) * spacing;
            float Y = 0.0f;
            float Z = 500.0f + ((currentSpawnCount / columnNumber) * spacing);

            FVector SpawnLocation(X, Y, Z);
            FRotator SpawnRotation = FRotator::ZeroRotator;

            AActor* NewActor = GetWorld()->SpawnActor<AActor>(objectToSpawn, SpawnLocation, SpawnRotation);
            if (NewActor)
            {
                spawnedActors.Add(NewActor);
            }
            currentSpawnCount++;
        }

        if (currentSpawnCount >= targetObjectCount)
        {
            isSpawning = false;
        }
    }

    if (!isRecording) return;

    // RECORDING DATA 
    Timer += DeltaTime;

    if (Timer >= maxRecordingTime)
    {
        isRecording = false;
        SaveData();
        return;
    }

    float FPS = 1.0f / DeltaTime;
    float FrameTimeMS = DeltaTime * 1000.0f;

    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();

    //convert to MB
    int TotalMemoryMB = MemStats.UsedPhysical / (1024 * 1024);

    // Search for the Cube Mesh Component 
    int ActiveCount = 0;
    for (AActor* Actor : spawnedActors)
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
        Timer, targetObjectCount, FPS, FrameTimeMS, TotalMemoryMB, ActiveCount);

    CSVContent += Line;
}

void APhysicsStressTest::SaveData()
{
    FString FilePath = FPaths::ProjectSavedDir() + "Logs/" + fileName;
    FFileHelper::SaveStringToFile(CSVContent, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);

    UE_LOG(LogTemp, Warning, TEXT("Test Complete. Data APPENDED to: %s"), *FilePath);
}