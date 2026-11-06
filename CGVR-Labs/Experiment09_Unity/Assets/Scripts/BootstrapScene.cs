using UnityEngine;

// Starts the complete mini-game without requiring a hand-authored scene file.
public static class BootstrapScene
{
    [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.AfterSceneLoad)]
    private static void CreateLabScene()
    {
        if (Object.FindFirstObjectByType<NeonRelicRun>() != null) return;
        new GameObject("Neon Relic Run — Runtime Scene").AddComponent<NeonRelicRun>();
    }
}
