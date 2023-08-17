using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.UI;

public class Test : MonoBehaviour
{

    public ComputeShader cs;
    RenderTexture rt;
    CommandBuffer cb1;
    CommandBuffer cb2;
    GraphicsFence gf1;
    public RawImage img;
    // Start is called before the first frame update
    void Start()
    {

        cb1 = new CommandBuffer();
        cb2 = new CommandBuffer();

        cb1.name = "FenceSignal";
        cb2.name = "WaitOnFence";




        rt = new RenderTexture(16, 16, 1, RenderTextureFormat.ARGB32, RenderTextureReadWrite.Linear)
        {
            enableRandomWrite = true,
            name = "ComputeShaderTexture"
            
        };
        rt.Create();

        img.texture = rt;

        cs.SetTexture(cs.FindKernel("CSMain"), Shader.PropertyToID("Result"), rt);

        cb1.SetExecutionFlags(CommandBufferExecutionFlags.AsyncCompute);
        cb1.DispatchCompute(cs, cs.FindKernel("CSMain"),2,2,1);
        gf1 = cb1.CreateAsyncGraphicsFence();

        cb2.WaitOnAsyncGraphicsFence(gf1);
        Camera.main.AddCommandBuffer(CameraEvent.BeforeForwardOpaque, cb2);
    }

    private void OnPreRender()
    {
        Graphics.ExecuteCommandBufferAsync(cb1, ComputeQueueType.Default);
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
