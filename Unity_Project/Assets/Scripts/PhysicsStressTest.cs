using System.Collections.Generic;
using System.IO;
using TMPro;
using Unity.Profiling;
using UnityEngine;
using UnityEngine.UI;

public class PhysicsStressTest : MonoBehaviour
{
    [Header("References")]
    public GameObject objectToSpawn;
    public TMP_InputField countInputField;
    public Button spawnButton;
    public string fileName = "PhysicsData_Combined.csv";

    [Header("Spawn Settings")]
    public int columnNumber = 10;
    public float spacing = 1.5f;
    public float maxRecordingTime = 30.0f;
    public int spawnsPerFrame = 20;

    // Profilers
    private ProfilerRecorder physicsTimeRecorder;
    private ProfilerRecorder totalMemoryRecorder;

    private List<GameObject> activeObjects = new List<GameObject>();
    private bool isRecording = false;
    private bool isSpawning = false;
    private float timer = 0f;

    private int currentTargetCount = 0;
    private int currentSpawnCount = 0;

    void OnEnable()
    {
        physicsTimeRecorder = ProfilerRecorder.StartNew(ProfilerCategory.Physics, "Physics.Simulate");
        totalMemoryRecorder = ProfilerRecorder.StartNew(ProfilerCategory.Memory, "Total Used Memory");
    }

    void OnDisable()
    {
        physicsTimeRecorder.Dispose();
        totalMemoryRecorder.Dispose();
    }

    void Start()
    {
        spawnButton.onClick.AddListener(OnStartTestClicked);

        // Write Header only if file doesn't exist
        string filePath = Path.Combine(Application.dataPath, "..", fileName);
        if (!File.Exists(filePath))
        {
            File.WriteAllText(filePath, "Time,ObjectCount,FPS,FrameTime(ms),PhysicsTime(ms),TotalMemory(MB),ActiveObjects\n");
        }
    }

    public void OnStartTestClicked()
    {
        // Cleanup previous run
        foreach (var obj in activeObjects) if (obj != null) Destroy(obj);
        activeObjects.Clear();

        // Parse target count
        if (int.TryParse(countInputField.text, out int count))
        {
            currentTargetCount = Mathf.Max(0, count);
        }
        else
        {
            currentTargetCount = 0;
        }

        // Reset for new run
        currentSpawnCount = 0;
        timer = 0f;
        isSpawning = currentTargetCount > 0;
        isRecording = true;

        Debug.Log($"Starting Test with {currentTargetCount} objects...");
    }

    void Update()
    {
        //Spawning logic
        if (isSpawning)
        {
            int toSpawnThisFrame = Mathf.Min(spawnsPerFrame, currentTargetCount - currentSpawnCount);
            for (int i = 0; i < toSpawnThisFrame; i++)
            {
                int spawnedIndex = currentSpawnCount + i;

                Vector3 pos = new Vector3(
                    (spawnedIndex % columnNumber) * spacing,
                    (spawnedIndex / columnNumber) * spacing + 5f,
                    0f
                );

                GameObject newObj = Instantiate(objectToSpawn, pos, Quaternion.identity);
                activeObjects.Add(newObj);
            }

            currentSpawnCount += toSpawnThisFrame;

            if (currentSpawnCount >= currentTargetCount)
            {
                isSpawning = false;
                Debug.Log($"Spawned {currentTargetCount} objects. Starting recording...");
            }
        }

        if (!isRecording) return;

        timer += Time.deltaTime;

        if (timer >= maxRecordingTime)
        {
            isRecording = false;
            Debug.Log($"Test Complete. Stopped after {maxRecordingTime} seconds.");
            return;
        }

        // Record data every frame
        double physicsTimeMS = physicsTimeRecorder.LastValue * (1e-6);
        float frameTimeMS = Time.deltaTime * 1000.0f;
        long memoryMB = totalMemoryRecorder.LastValue / (1024 * 1024);

        float fps = 1.0f / Mathf.Max(Time.deltaTime, 1e-6f);

        int activeCount = 0;
        for (int i = 0; i < activeObjects.Count; i++)
        {
            var obj = activeObjects[i];
            if (obj == null) continue;

            Rigidbody rb = obj.GetComponent<Rigidbody>();
            if (rb != null && !rb.IsSleeping())
            {
                activeCount++;
            }
        }

        string line = $"{timer:F2},{currentTargetCount},{fps:F1},{frameTimeMS:F4},{physicsTimeMS:F4},{memoryMB},{activeCount}\n";
        string filePath = Path.Combine(Application.dataPath, "..", fileName);
        File.AppendAllText(filePath, line);
    }
}