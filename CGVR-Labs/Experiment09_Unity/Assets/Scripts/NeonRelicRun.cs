using System.Collections.Generic;
using UnityEngine;

// Experiment 9: a complete, asset-free interactive 3D scene.
// Collect the floating relics, then enter the portal.
public class NeonRelicRun : MonoBehaviour
{
    public static NeonRelicRun Instance { get; private set; }
    public int Collected { get; private set; }
    public int Total { get; private set; }
    public bool Complete { get; private set; }

    private float startTime;
    private Transform player;
    private Material floorMaterial, cyan, magenta, gold, violet;

    void Awake() => Instance = this;

    void Start()
    {
        Application.targetFrameRate = 120;
        RenderSettings.ambientMode = UnityEngine.Rendering.AmbientMode.Flat;
        RenderSettings.ambientLight = new Color(.025f, .035f, .10f);
        RenderSettings.fog = true; RenderSettings.fogColor = new Color(.018f, .012f, .06f); RenderSettings.fogDensity = .018f;
        floorMaterial = MakeMaterial(new Color(.035f, .055f, .12f), new Color(.01f, .02f, .08f));
        cyan = MakeMaterial(new Color(.02f, .45f, .7f), new Color(0f, .9f, 1f));
        magenta = MakeMaterial(new Color(.7f, .03f, .48f), new Color(1f, 0f, .55f));
        gold = MakeMaterial(new Color(.8f, .35f, .02f), new Color(1f, .35f, .01f));
        violet = MakeMaterial(new Color(.27f, .06f, .7f), new Color(.5f, .05f, 1f));
        CreateWorld(); CreatePlayer(); CreateRelics(); CreatePortal(); CreateSky();
        startTime = Time.time;
    }

    Material MakeMaterial(Color baseColor, Color emission)
    {
        Shader shader = Shader.Find("Universal Render Pipeline/Lit") ?? Shader.Find("Standard") ?? Shader.Find("Sprites/Default");
        var material = new Material(shader) { color = baseColor };
        if (material.HasProperty("_BaseColor")) material.SetColor("_BaseColor", baseColor);
        if (material.HasProperty("_EmissionColor")) { material.EnableKeyword("_EMISSION"); material.SetColor("_EmissionColor", emission); }
        return material;
    }

    GameObject Primitive(PrimitiveType type, string label, Vector3 position, Vector3 scale, Material material)
    {
        var go = GameObject.CreatePrimitive(type); go.name = label; go.transform.SetPositionAndRotation(position, Quaternion.identity); go.transform.localScale = scale;
        go.GetComponent<Renderer>().sharedMaterial = material; return go;
    }

    void CreateWorld()
    {
        Primitive(PrimitiveType.Plane, "Midnight Arena", Vector3.zero, new Vector3(4, 1, 4), floorMaterial);
        for (int i = -18; i <= 18; i += 2)
        {
            var a = Primitive(PrimitiveType.Cube, "Grid line", new Vector3(i, .012f, 0), new Vector3(.025f, .02f, 36), i % 4 == 0 ? cyan : violet);
            var b = Primitive(PrimitiveType.Cube, "Grid line", new Vector3(0, .012f, i), new Vector3(36, .02f, .025f), i % 4 == 0 ? cyan : violet);
            a.GetComponent<Collider>().enabled = b.GetComponent<Collider>().enabled = false;
        }
        Vector3[] pillars = { new(-14,0,-14), new(14,0,-14), new(-14,0,14), new(14,0,14), new(-14,0,0), new(14,0,0), new(0,0,-14), new(0,0,14) };
        for (int i=0;i<pillars.Length;i++)
        {
            var p = Primitive(PrimitiveType.Cylinder, "Neon Obelisk", pillars[i] + Vector3.up * 2.2f, new Vector3(.65f, 2.2f, .65f), i % 2 == 0 ? magenta : cyan);
            p.AddComponent<NeonPulse>().speed = .8f + i * .08f;
            var light = p.AddComponent<Light>(); light.type = LightType.Point; light.range = 7; light.intensity = 2.2f; light.color = i % 2 == 0 ? new Color(1,.05f,.5f) : new Color(0,.85f,1);
        }
        var sun = new GameObject("Soft violet moonlight").AddComponent<Light>(); sun.type = LightType.Directional; sun.color = new Color(.35f,.35f,1); sun.intensity = .55f; sun.transform.rotation = Quaternion.Euler(48,-28,0);
    }

    void CreatePlayer()
    {
        var go = Primitive(PrimitiveType.Capsule, "Relic Runner", new Vector3(0, 1.1f, -11), new Vector3(.8f, 1.1f, .8f), cyan);
        Destroy(go.GetComponent<Collider>()); var controller = go.AddComponent<CharacterController>(); controller.height = 2; controller.radius = .42f; controller.center = new Vector3(0, .0f, 0);
        go.AddComponent<PlayerPilot>(); player = go.transform;
        var glow = new GameObject("Runner glow").AddComponent<Light>(); glow.transform.SetParent(player); glow.transform.localPosition = new Vector3(0,.4f,0); glow.color = Color.cyan; glow.range = 6; glow.intensity = 3;
        var camera = new GameObject("Runner Camera").AddComponent<Camera>(); camera.tag = "MainCamera"; camera.fieldOfView = 66; camera.backgroundColor = new Color(.012f,.008f,.045f); camera.gameObject.AddComponent<AudioListener>();
        camera.gameObject.AddComponent<NeonCamera>().target = player;
    }

    void CreateRelics()
    {
        Vector3[] sites = { new(-10,1,-9), new(-4,1,-5), new(6,1,-9), new(11,1,-3), new(-11,1,2), new(-5,1,6), new(4,1,4), new(10,1,9), new(-10,1,10), new(0,1,11), new(0,1,1), new(8,1,0) };
        foreach (var site in sites)
        {
            var relic = Primitive(PrimitiveType.Cube, "Floating Relic", site, Vector3.one * .78f, gold);
            relic.transform.rotation = Quaternion.Euler(45, 45, 0);
            relic.GetComponent<Collider>().isTrigger = true; relic.AddComponent<Relic>(); relic.AddComponent<NeonPulse>().speed = 1.6f;
            var light = relic.AddComponent<Light>(); light.type = LightType.Point; light.color = new Color(1,.35f,.03f); light.range = 4; light.intensity = 2;
            Total++;
        }
    }

    void CreatePortal()
    {
        Vector3 p = new(0, 1.5f, 14);
        var gate = new GameObject("Victory Portal"); gate.transform.position = p;
        var trigger = gate.AddComponent<BoxCollider>(); trigger.size = new Vector3(3.4f, 3.5f, 1.3f); trigger.isTrigger = true; gate.AddComponent<PortalGate>();
        for (int i=0;i<22;i++)
        {
            float a = i * Mathf.PI * 2 / 22f;
            var shard = Primitive(PrimitiveType.Sphere, "Portal shard", p + new Vector3(Mathf.Cos(a)*1.5f, Mathf.Sin(a)*1.5f, 0), Vector3.one*.22f, magenta);
            shard.transform.SetParent(gate.transform); shard.AddComponent<NeonPulse>().speed = 1.8f;
        }
        var core = Primitive(PrimitiveType.Cylinder, "Portal core", p, new Vector3(1.38f,.03f,1.38f), violet); core.transform.rotation = Quaternion.Euler(90,0,0); core.transform.SetParent(gate.transform);
        var light = gate.AddComponent<Light>(); light.type=LightType.Point; light.color = new Color(1,0,.6f); light.range=9; light.intensity=3;
    }

    void CreateSky()
    {
        Random.InitState(12);
        for(int i=0;i<110;i++)
        {
            Vector3 p = new(Random.Range(-34f,34f), Random.Range(5f,21f), Random.Range(-34f,34f));
            var star = Primitive(PrimitiveType.Sphere, "Star", p, Vector3.one*Random.Range(.025f,.075f), i%5==0?magenta:cyan);
            star.GetComponent<Collider>().enabled=false;
        }
    }

    public void Collect(GameObject relic)
    {
        if (Complete) return; Collected++; Destroy(relic);
    }

    public void TryFinish()
    {
        if (Collected >= Total) Complete = true;
    }

    void Update()
    {
        if (Complete && Input.GetKeyDown(KeyCode.R)) UnityEngine.SceneManagement.SceneManager.LoadScene(UnityEngine.SceneManagement.SceneManager.GetActiveScene().buildIndex);
    }

    void OnGUI()
    {
        var title = new GUIStyle(GUI.skin.label) { fontSize=26, fontStyle=FontStyle.Bold, alignment=TextAnchor.UpperLeft, normal={textColor=Color.cyan} };
        var body = new GUIStyle(GUI.skin.label) { fontSize=16, normal={textColor=Color.white} };
        GUI.Box(new Rect(18,18,330,102), ""); GUI.Label(new Rect(34,28,300,35), "NEON RELIC RUN", title);
        GUI.Label(new Rect(36,67,290,24), $"RELICS  {Collected:D2} / {Total:D2}", body);
        GUI.Label(new Rect(36,91,300,24), Collected==Total ? "PORTAL UNLOCKED — reach the gate!" : "WASD / ARROWS: MOVE   SHIFT: BOOST", body);
        if (Complete)
        {
            var win = new GUIStyle(GUI.skin.label) { fontSize=38, fontStyle=FontStyle.Bold, alignment=TextAnchor.MiddleCenter, normal={textColor=new Color(1,.35f,.75f)} };
            GUI.Label(new Rect(0,Screen.height*.36f,Screen.width,60), "RELIC RUN COMPLETE", win);
            GUI.Label(new Rect(0,Screen.height*.36f+65,Screen.width,30), $"Arena cleared in {Time.time-startTime:0.0} seconds  •  Press R to replay", new GUIStyle(body){alignment=TextAnchor.MiddleCenter});
        }
    }
}

public class PlayerPilot : MonoBehaviour
{
    CharacterController controller; float vertical;
    void Start() => controller = GetComponent<CharacterController>();
    void Update()
    {
        float x=Input.GetAxisRaw("Horizontal"), z=Input.GetAxisRaw("Vertical"); Vector3 move=new Vector3(x,0,z).normalized;
        float speed=Input.GetKey(KeyCode.LeftShift)?10f:5.6f;
        if(controller.isGrounded){vertical=-1.2f;if(Input.GetKeyDown(KeyCode.Space))vertical=7.5f;} else vertical+=Physics.gravity.y*2.2f*Time.deltaTime;
        controller.Move((move*speed+Vector3.up*vertical)*Time.deltaTime);
        if(move.sqrMagnitude>.01f) transform.rotation=Quaternion.Slerp(transform.rotation,Quaternion.LookRotation(move),12*Time.deltaTime);
    }
}

public class NeonCamera : MonoBehaviour
{
    public Transform target;
    void LateUpdate(){ if(!target)return; Vector3 desired=target.position+new Vector3(0,5.2f,-7.5f); transform.position=Vector3.Lerp(transform.position,desired,7*Time.deltaTime); transform.LookAt(target.position+Vector3.up*.75f); }
}

public class Relic : MonoBehaviour
{
    void OnTriggerEnter(Collider other){if(other.GetComponent<PlayerPilot>()){NeonRelicRun.Instance.Collect(gameObject);}}
}

public class PortalGate : MonoBehaviour
{
    void OnTriggerEnter(Collider other){if(other.GetComponent<PlayerPilot>())NeonRelicRun.Instance.TryFinish();}
}

public class NeonPulse : MonoBehaviour
{
    public float speed=1; Vector3 basePosition;
    void Start()=>basePosition=transform.localPosition;
    void Update(){transform.Rotate(0,60*speed*Time.deltaTime,0,Space.Self);transform.localPosition=basePosition+Vector3.up*Mathf.Sin(Time.time*speed)*.12f;}
}
