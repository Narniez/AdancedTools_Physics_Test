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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress Test")
    TSubclassOf<AActor> objectToSpawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress Test")
    int columnNumber = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress Test")
    int targetObjectCount = 1000;

    UPROPERTY(EditAnywhere, Category = "Stress Test")
    float spacing = 150.0f;

    UPROPERTY(EditAnywhere, Category = "Stress Test")
    int spawnsPerFrame = 20;

    UPROPERTY(EditAnywhere, Category = "Stress Test")
    FString fileName = "UnrealPhysicsData.csv";

    UPROPERTY(EditAnywhere, Category = "Stress Test")
    float maxRecordingTime = 30.0f;

    UFUNCTION(BlueprintCallable, Category = "Stress Test")
    void StartTest(int NewCount);

private:
    void SaveData();

    bool isSpawning = false;
    bool isRecording = false;
    float Timer = 0.0f;
    int currentSpawnCount = 0;
    FString CSVContent;
    TArray<AActor*> spawnedActors;
};