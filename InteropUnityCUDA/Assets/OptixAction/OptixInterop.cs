using ActionUnity;
using System.Runtime.InteropServices;
using UnityEngine;
using UnityEngine.Experimental.Rendering;
using UnityEngine.UI;
using UnityEngine.XR;
using static UnityEngine.Camera;

public class OptixInterop : InteropHandler
{
    [Range(-2.0f, 2.0f)]
    public float hSliderValue = 0f;

    [Range(-2.0f, 2.0f)]
    public float vSliderValue = 0f;

    struct EyeCamera
    {
        public EyeCamera(int eyeIdx, MonoOrStereoscopicEye camEye, XRNode n, InputFeatureUsage<Vector3> feature)
        {
            eyeIndex = eyeIdx;
            cameraEye = camEye;
            xrNode = n;
            nodeUsage = feature;
        }
        public readonly int eyeIndex;
        public readonly Camera.MonoOrStereoscopicEye cameraEye;
        public readonly XRNode xrNode;
        public readonly InputFeatureUsage<Vector3> nodeUsage;
    }

    readonly EyeCamera[] eyeCameraMapping = new EyeCamera[]
    {
        new( 0, Camera.MonoOrStereoscopicEye.Left, XRNode.LeftEye, CommonUsages.leftEyePosition),
        new( 1, Camera.MonoOrStereoscopicEye.Right, XRNode.RightHand, CommonUsages.rightEyePosition)
    };

    private readonly string _ActionTestOptixName = "optixTextureArray";
    
    Texture _externalTargetTexture = null;
    Texture _oldExternalTexture = null;
    RenderTexture tmpTex = null;
    
    RenderTextureDescriptor _rendgineTextureDescriptor;
    ActionUnityTestOptixOwl optixAction = null;
    public RawImage img;
    public Renderer cubeRenderer;
    public Transform boxTransform;


    [StructLayout(LayoutKind.Sequential)]
    struct NativeCube
    {
        public Vector3 position;
        public Quaternion rotation;
        public Vector3 scale;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct NativeCameraData
    {
        public Vector3 cameraPositionWorld;
        public Vector3 cameraToUpperLeftScreen;
        public Vector3 pxStepU;
        public Vector3 pxStepV;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct NativeRenderingData
    {
        public NativeCube nativeCube;
        public int eyeCount;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public NativeCameraData[] nativeCameradata;
    }

    NativeRenderingData nativeRenderingData = new NativeRenderingData();
    System.IntPtr nativeRenderingDataPtr;

    // Start is called before the first frame update
    void Start()
    {
        nativeRenderingData = new NativeRenderingData();

        nativeRenderingData.nativeCube = new NativeCube();
        
        nativeRenderingData.nativeCameradata = new NativeCameraData[2];
        nativeRenderingDataPtr = Marshal.AllocHGlobal(Marshal.SizeOf<NativeRenderingData>());

        createExternalTargetTexture();
        InitializeInteropHandler();
    }

    void destroyAction()
    {
        if (optixAction != null)
        {
            CallFunctionOnDestroyInAction(_ActionTestOptixName);
            
            UnregisterActionUnity(_ActionTestOptixName);
            optixAction = null;
        }
    }

    void recreateAction()
    {
        destroyAction();
        optixAction = new ActionUnityTestOptixOwl(_externalTargetTexture, _rendgineTextureDescriptor.volumeDepth);
        RegisterActionUnity(optixAction, _ActionTestOptixName);
        CallFunctionStartInAction(_ActionTestOptixName);
    }

    protected override void InitializeActions()
    {
        base.InitializeActions();
        recreateAction();
    }

    private void OnPreRender()
    {
        if (_oldExternalTexture)
        {
            // _oldExternalTexture.
            _oldExternalTexture = null;
        }
        if (renderTextureDescriptorChanged())
        {
            Debug.Log("RenderTexture changed");
            createExternalTargetTexture();

            optixAction.setTextureForObject(_externalTargetTexture, _rendgineTextureDescriptor.width, _rendgineTextureDescriptor.height, _rendgineTextureDescriptor.volumeDepth);
            // recreateAction();
        }
        UpdateInteropHandler();
    }

    void setnativeRenderingCameraData(Vector3 eyePos, Camera.MonoOrStereoscopicEye stereoEye, int eyeIndex)
    {
        /*
            Oxxxx
            xxxxx
            xxxxx
            xxxxx
        */
        Vector3 pxUpperLeft = Camera.main.ScreenToWorldPoint(new Vector3(hSliderValue, _rendgineTextureDescriptor.height - 1 - vSliderValue, 1), stereoEye);

        /*
            xxxxO
            xxxxx
            xxxxx
            xxxxx
        */
        Vector3 pxUpperRight = Camera.main.ScreenToWorldPoint(new Vector3(_rendgineTextureDescriptor.width - 1 + hSliderValue, _rendgineTextureDescriptor.height - 1 - vSliderValue, 1), stereoEye);

        /*
            xxxxx
            xxxxx
            xxxxx
            Oxxxx
        */
        Vector3 pxLowerLeft = Camera.main.ScreenToWorldPoint(new Vector3(hSliderValue, -vSliderValue, 1), stereoEye);

        // Right
        var onePxDirectionU = (pxUpperRight - pxUpperLeft) / _rendgineTextureDescriptor.width;

        // Down
        var onePxDirectionV = (pxLowerLeft - pxUpperLeft) / _rendgineTextureDescriptor.height;

        var camToUpperLeft = (pxUpperLeft - eyePos);
        nativeRenderingData.nativeCameradata[eyeIndex].cameraPositionWorld = eyePos;
        nativeRenderingData.nativeCameradata[eyeIndex].cameraToUpperLeftScreen = camToUpperLeft;
        nativeRenderingData.nativeCameradata[eyeIndex].pxStepU = onePxDirectionU;
        nativeRenderingData.nativeCameradata[eyeIndex].pxStepV = onePxDirectionV;
    }

    void fillNativeRenderingData()
    {
        var camPosition = Camera.main.transform.position;
        if (XRSettings.isDeviceActive)
        {
            nativeRenderingData.eyeCount = 2;
            // UnityEngine.XR.OpenXR.Input.OpenXRDevice
            var centerEyeDevice = InputDevices.GetDeviceAtXRNode(XRNode.CenterEye);

            Vector3 cameraLocalPos = Camera.main.transform.localPosition;
            centerEyeDevice.TryGetFeatureValue(CommonUsages.centerEyePosition, out cameraLocalPos);

            foreach (var nodeUsage in eyeCameraMapping)
            {
                Camera.MonoOrStereoscopicEye stereoEye = nodeUsage.cameraEye;
                Vector3 eyePos = Vector3.zero;
                if (!centerEyeDevice.TryGetFeatureValue(nodeUsage.nodeUsage, out eyePos))
                {
                    stereoEye = MonoOrStereoscopicEye.Left;
                    Debug.Log("Could not get position for Node " + nodeUsage.nodeUsage.name);
                }
                
                Vector3 eyeWorldPos = Camera.main.transform.TransformPoint(eyePos - cameraLocalPos);

                setnativeRenderingCameraData(eyeWorldPos, stereoEye, nodeUsage.eyeIndex);
            }
        } 
        else
        {
            nativeRenderingData.eyeCount = 1;
            setnativeRenderingCameraData(Camera.main.transform.position, Camera.MonoOrStereoscopicEye.Mono, 0);
        }
        
        var mat = boxTransform.localToWorldMatrix;
        nativeRenderingData.nativeCube.position = boxTransform.position;
        nativeRenderingData.nativeCube.rotation = boxTransform.localRotation;
        nativeRenderingData.nativeCube.scale = boxTransform.localScale;
    }

    protected override void UpdateActions()
    {
        base.UpdateActions();
        fillNativeRenderingData();
        
        Marshal.StructureToPtr(nativeRenderingData, nativeRenderingDataPtr, true);

        (GetActionUnity(_ActionTestOptixName) as ActionUnityTestOptixOwl).setRenderingDataForObject(nativeRenderingDataPtr);

        CallFunctionUpdateInAction(_ActionTestOptixName);
        if(tmpTex)
        {
            Graphics.Blit(_externalTargetTexture, tmpTex, 0, 0);
        }
    }

    public void OnDestroy()
    {
        OnDestroyInteropHandler();
        Marshal.FreeHGlobal(nativeRenderingDataPtr);
        nativeRenderingDataPtr = System.IntPtr.Zero;
    }

    /// <summary>
    /// call onDestroy function of the two registered actions
    /// </summary>
    protected override void OnDestroyActions()
    {
        base.OnDestroyActions();
        CallFunctionOnDestroyInAction(_ActionTestOptixName);
    }

    bool renderTextureDescriptorChanged()
    {
        if (XRSettings.isDeviceActive)
        {
            return !_rendgineTextureDescriptor.Equals(XRSettings.eyeTextureDesc);
        }
        return !_rendgineTextureDescriptor.Equals(new RenderTextureDescriptor(Screen.width, Screen.height, RenderTextureFormat.ARGB32)
        {
            volumeDepth = 2
        });
    }

    // Update is called once per frame
    void createExternalTargetTexture()
    {
        if (XRSettings.isDeviceActive)
        {
            _rendgineTextureDescriptor = XRSettings.eyeTextureDesc;
        }
        else
        {
            _rendgineTextureDescriptor = new RenderTextureDescriptor(Screen.width, Screen.height, RenderTextureFormat.ARGB32) { volumeDepth = 2 };
        }
        // Debug.Log(_rendgineTextureDescriptor.width + " " + _rendgineTextureDescriptor.height + " " + _rendgineTextureDescriptor.volumeDepth);
        if(tmpTex)
        {
            tmpTex.Release();
        }
        _oldExternalTexture = _externalTargetTexture;
        if (_rendgineTextureDescriptor.volumeDepth > 1)
        {
            Texture2DArray t2dArr = new Texture2DArray(
                _rendgineTextureDescriptor.width,
                _rendgineTextureDescriptor.height,
                _rendgineTextureDescriptor.volumeDepth,
                _rendgineTextureDescriptor.graphicsFormat,
                TextureCreationFlags.None
                );
            nativeRenderingData.eyeCount = 2;
            _externalTargetTexture = t2dArr;
            tmpTex = new RenderTexture(
                _rendgineTextureDescriptor.width,
                _rendgineTextureDescriptor.height,
                _rendgineTextureDescriptor.graphicsFormat,
                0
                );
            tmpTex.Create();
            //img.texture = tmpTex;

        }
        else
        {
            tmpTex = null;
            _externalTargetTexture = new Texture2DArray(
                _rendgineTextureDescriptor.width,
                _rendgineTextureDescriptor.height,
                _rendgineTextureDescriptor.volumeDepth,
                _rendgineTextureDescriptor.graphicsFormat,
                TextureCreationFlags.None
                );
            nativeRenderingData.eyeCount = 1;
            tmpTex = new RenderTexture(
                _rendgineTextureDescriptor.width,
                _rendgineTextureDescriptor.height,
                _rendgineTextureDescriptor.graphicsFormat,
                0
                );
            tmpTex.Create();
            //img.texture = tmpTex;
        }
        cubeRenderer.material.SetTexture("_MyArr", _externalTargetTexture);
    }

}
