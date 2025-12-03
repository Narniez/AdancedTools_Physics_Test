using System.Collections;
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

    [Header("Settings")]
    public string fileName = "PhysicsData_Combined.csv";
    public int rowSize = 10;
    public float spacing = 1.5f;
    [Tooltip("Stop recording after this many seconds to prevent infinite data")]
    public float maxRecordingTime = 20.0f;

    // Profilers
    private ProfilerRecorder physicsTimeRecorder;
    private ProfilerRecorder totalMemoryRecorder;

    private List<GameObject> activeObjects = new List<GameObject>();
    private bool isRecording = false;
    private float timer = 0f;
    private int currentTargetCount = 0;

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
            File.WriteAllText(filePath, "Time,ObjectCount,FPS,PhysicsTime(ms),TotalMemory(MB),ActiveObjects\n");
        }
    }

    public void OnStartTestClicked()
    {
        foreach (var obj in activeObjects) Destroy(obj);
        activeObjects.Clear();

        if (int.TryParse(countInputField.text, out int count))
        {
            currentTargetCount = count;
            StartCoroutine(SpawnRoutine(count));
        }
    }

    IEnumerator SpawnRoutine(int count)
    {
        isRecording = false; 
        yield return new WaitForSeconds(0.5f);

        int spawned = 0;

        while (spawned < count)
        {
            // Spawn 20 per frame to avoid freezing
            for (int i = 0; i < 20; i++)
            {
                if (spawned >= count) break;

                Vector3 pos = new Vector3(
                    (spawned % rowSize) * spacing,
                    (spawned / rowSize) * spacing + 5f,
                    0
                );

                // Add noise to prevent stacking sleep
                pos.x += Random.Range(-0.1f, 0.1f);

                GameObject newObj = Instantiate(objectToSpawn, pos, Quaternion.identity);
                activeObjects.Add(newObj);
                spawned++;
            }
            yield return null;
        }

        Debug.Log($"Spawned {count} objects. Starting recording...");
        timer = 0f;
        isRecording = true;
    }

    void Update()
    {
        if (!isRecording) return;

        timer += Time.deltaTime;

        // Auto-stop feature: Stop after 20 seconds
        if (timer >= maxRecordingTime)
        {
            isRecording = false;
            Debug.Log($"Test Complete. Stopped after {maxRecordingTime} seconds.");
            return;
        }

        // Record data every frame
        double physicsTimeMS = physicsTimeRecorder.LastValue * (1e-6);
        long memoryMB = totalMemoryRecorder.LastValue / (1024 * 1024);
        float fps = 1.0f / Time.deltaTime;

        int activeCount = 0;
        foreach (var obj in activeObjects)
        {
            if (obj != null)
            {
                Rigidbody rb = obj.GetComponent<Rigidbody>();
                // If RigidBody is NOT sleeping, it is calculating physics
                if (rb != null && !rb.IsSleeping())
                {
                    activeCount++;
                }
            }
        }

        // Added activeCount to the CSV line
        string line = $"{timer:F2},{currentTargetCount},{fps:F1},{physicsTimeMS:F4},{memoryMB},{activeCount}\n";

        // Append to file immediately (safer for crashes)
        string filePath = Path.Combine(Application.dataPath, "..", fileName);
        File.AppendAllText(filePath, line);
    }
}