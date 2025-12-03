#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhysicsStressTest.generated.h"

UCLASS()
class ADVANCEDTOOLS_UNREAL_API APhysicsStressTest : public AActor
{
    GENERATED_BODY()

public:
    APhysicsStressTest();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // --- Settings ---
    UPROPERTY(EditAnywhere, Category = "Stress Test")
    TSubclassOf<AActor> ObjectToSpawn;

    UPROPERTY(EditAnywhere, Category = "Stress Test")
    int32 TargetObjectCount = 1000;

    UPROPERTY(EditAnywhere, Category = "Stress Test")
    float Spacing = 150.0f;

    UPROPERTY(EditAnywhere, Category = "Stress Test")
    int32 SpawnsPerFrame = 20; 

    UPROPERTY(EditAnywhere, Category = "Stress Test")
    FString FileName = "UnrealPhysicsData.csv";

    UPROPERTY(EditAnywhere, Category = "Stress Test")
    float MaxRecordingTime = 20.0f;

    UFUNCTION(CallInEditor, Category = "Stress Test")
    void RunTest();

private:
    void SaveData();

    // Internal State
    bool bIsSpawning = false; 
    bool bIsRecording = false;
    float Timer = 0.0f;
    int32 CurrentSpawnCount = 0; 
    FString CSVContent;
    TArray<AActor*> SpawnedActors;
};