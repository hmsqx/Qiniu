import { useEffect, useRef, useState } from "react";
import * as THREE from "three";
import { OrbitControls } from "three/examples/jsm/controls/OrbitControls.js";
import { GLTFLoader } from "three/examples/jsm/loaders/GLTFLoader.js";
import { DRACOLoader } from "three/examples/jsm/loaders/DRACOLoader.js";
import { KTX2Loader } from "three/examples/jsm/loaders/KTX2Loader.js";
import { MeshoptDecoder } from "three/examples/jsm/libs/meshopt_decoder.module.js";
import { OBJLoader } from "three/examples/jsm/loaders/OBJLoader.js";
import { FBXLoader } from "three/examples/jsm/loaders/FBXLoader.js";
import { STLLoader } from "three/examples/jsm/loaders/STLLoader.js";

export type ThreeViewerOptions = {
  url: string | null;
  format: string;
  isVideo: boolean;
  isUsd: boolean;
  autoRotate: boolean;
  stlColor: string;
};

export function useThreeViewer(
  containerRef: React.RefObject<HTMLDivElement | null>,
  opts: ThreeViewerOptions
) {
  const canvasRef = useRef<HTMLCanvasElement | null>(null);
  const sceneRef = useRef<THREE.Scene | null>(null);
  const cameraRef = useRef<THREE.PerspectiveCamera | null>(null);
  const rendererRef = useRef<THREE.WebGLRenderer | null>(null);
  const controlsRef = useRef<OrbitControls | null>(null);
  const stlMaterialRef = useRef<THREE.MeshStandardMaterial | null>(null);
  const initialCamRef = useRef<{
    pos: THREE.Vector3;
    target: THREE.Vector3;
  } | null>(null);
  const modelRootRef = useRef<THREE.Object3D | null>(null);
  const [loadingProgress, setLoadingProgress] = useState<number>(0);

  useEffect(() => {
    const { url, format, isVideo, isUsd, autoRotate, stlColor } = opts;
    if (!containerRef.current || !url || isVideo || isUsd) return;

    const safeUrl = url as string;
    const container = containerRef.current;

    const manager = new THREE.LoadingManager();
    const proxyify = (inputUrl: string) => {
      try {
        if (/^(data:|blob:)/i.test(inputUrl)) return inputUrl;
        if (/^(\.\.\/|\.\/|\/)/.test(inputUrl)) return inputUrl;
        const u = new URL(inputUrl, window.location.origin);
        if (u.hostname.endsWith("tencentcos.cn")) {
          // Ensure assets referenced inside GLTF/MTL also go through our proxy
          return "/model" + u.pathname + (u.search || "");
        }
        return inputUrl;
      } catch {
        return inputUrl;
      }
    };
    manager.setURLModifier(proxyify);
    manager.onProgress = (_url, itemsLoaded, itemsTotal) => {
      if (itemsTotal > 0) {
        setLoadingProgress(Math.round((itemsLoaded / itemsTotal) * 100));
      }
    };
    manager.onLoad = () => setLoadingProgress(100);

    const renderer = new THREE.WebGLRenderer({
      antialias: true,
      alpha: true,
      powerPreference: "high-performance",
    });
    // Cap DPR to reduce GPU load on high-DPI displays
    renderer.setPixelRatio(Math.min(window.devicePixelRatio || 1, 1.5));
    renderer.setSize(container.clientWidth, container.clientHeight);
    renderer.outputColorSpace = THREE.SRGBColorSpace;
    container.appendChild(renderer.domElement);
    canvasRef.current = renderer.domElement;
    rendererRef.current = renderer;

    const scene = new THREE.Scene();
    // No background color logic; keep transparent to show page background
    scene.background = null;
    sceneRef.current = scene;

    const camera = new THREE.PerspectiveCamera(
      60,
      Math.max(1, container.clientWidth) / Math.max(1, container.clientHeight),
      0.1,
      1000
    );
    camera.position.set(2.5, 2.0, 3.5);
    cameraRef.current = camera;
    initialCamRef.current = {
      pos: camera.position.clone(),
      target: new THREE.Vector3(0, 0.5, 0),
    };

    const hemi = new THREE.HemisphereLight(0xffffff, 0x444444, 1.0);
    hemi.position.set(0, 20, 0);
    scene.add(hemi);

    const dir = new THREE.DirectionalLight(0xffffff, 1.0);
    dir.position.set(5, 10, 7.5);
    dir.castShadow = true;
    scene.add(dir);

    // Removed grid helper to eliminate grid rendering

    const controls = new OrbitControls(camera, renderer.domElement);
    controls.enableDamping = true;
    controls.target.set(0, 0.5, 0);
    controlsRef.current = controls;
    controls.autoRotate = autoRotate;

    function centerAndScale(object: THREE.Object3D) {
      const box = new THREE.Box3().setFromObject(object);
      const size = new THREE.Vector3();
      const center = new THREE.Vector3();
      box.getSize(size);
      box.getCenter(center);
      const maxDim = Math.max(size.x, size.y, size.z) || 1;
      const scale = 2.0 / maxDim;
      object.position.sub(center);
      object.position.multiplyScalar(scale);
      object.scale.setScalar(scale);
    }

    async function load() {
      try {
        let root: THREE.Object3D | null = null;
        if (format === "glb" || format === "gltf") {
          const loader = new GLTFLoader(manager);
          // Prefer compressed assets when available
          try {
            const draco = new DRACOLoader(manager);
            draco.setDecoderPath("https://www.gstatic.com/draco/v1/decoders/");
            loader.setDRACOLoader(draco);
          } catch {}
          try {
            loader.setMeshoptDecoder(MeshoptDecoder as any);
          } catch {}
          loader.setCrossOrigin("anonymous");
          const basePath = new URL(
            safeUrl,
            window.location.origin
          ).href.replace(/[^/]*$/g, "");
          loader.setResourcePath(basePath);
          try {
            const ktx2 = new KTX2Loader(manager)
              .setTranscoderPath(
                // Note: you can host these under /basis/ for offline usage
                "https://unpkg.com/three@0.160.0/examples/jsm/libs/basis/"
              )
              .detectSupport(renderer);
            loader.setKTX2Loader(ktx2);
          } catch {}
          const gltf = await loader.loadAsync(safeUrl);
          root = gltf.scene;
        } else if (format === "obj") {
          const objLoader = new OBJLoader(manager);
          root = await objLoader.loadAsync(safeUrl);
        } else if (format === "fbx") {
          const loader = new FBXLoader(manager);
          const basePath = new URL(
            safeUrl,
            window.location.origin
          ).href.replace(/[^/]*$/g, "");
          loader.setResourcePath?.(basePath as any);
          root = await loader.loadAsync(safeUrl);
        } else if (format === "stl") {
          const loader = new STLLoader(manager);
          const geometry = await loader.loadAsync(safeUrl);
          const material = new THREE.MeshStandardMaterial({
            color: new THREE.Color(stlColor),
            metalness: 0.1,
            roughness: 0.8,
          });
          const mesh = new THREE.Mesh(geometry, material);
          mesh.castShadow = true;
          mesh.receiveShadow = true;
          root = mesh;
          stlMaterialRef.current = material;
        } else if (!format) {
          throw new Error("未知格式：无法从链接推断");
        } else {
          throw new Error(`暂不支持该格式: ${format}`);
        }

        if (root) {
          centerAndScale(root);
          scene.add(root);
          modelRootRef.current = root;
        }
      } catch (err) {
        console.error(err);
      }
    }
    load();

    let raf = 0;
    const onResize = () => {
      const w = Math.max(1, container.clientWidth);
      const h = Math.max(1, container.clientHeight);
      renderer.setSize(w, h);
      camera.aspect = w / h;
      camera.updateProjectionMatrix();
    };

    const animate = () => {
      controls.update();
      renderer.render(scene, camera);
      raf = requestAnimationFrame(animate);
    };
    animate();

    window.addEventListener("resize", onResize);

    return () => {
      window.removeEventListener("resize", onResize);
      cancelAnimationFrame(raf);
      controls.dispose();
      renderer.dispose();
      scene.traverse((obj) => {
        const mesh = obj as THREE.Mesh;
        if (mesh.isMesh) {
          mesh.geometry?.dispose?.();
          const mats = Array.isArray(mesh.material)
            ? mesh.material
            : [mesh.material];
          mats.forEach((m: any) => {
            if (m && typeof m.dispose === "function") m.dispose();
          });
        }
      });
      if (renderer.domElement && renderer.domElement.parentElement) {
        renderer.domElement.parentElement.removeChild(renderer.domElement);
      }
    };
  }, [containerRef, opts.url, opts.format, opts.isVideo, opts.isUsd]);

  useEffect(() => {
    if (controlsRef.current) controlsRef.current.autoRotate = opts.autoRotate;
  }, [opts.autoRotate]);

  useEffect(() => {
    if (stlMaterialRef.current) {
      stlMaterialRef.current.color = new THREE.Color(opts.stlColor);
      stlMaterialRef.current.needsUpdate = true;
    }
  }, [opts.stlColor]);

  const handleResetView = () => {
    const cam = cameraRef.current;
    const controls = controlsRef.current;
    if (!cam || !controls || !initialCamRef.current) return;
    cam.position.copy(initialCamRef.current.pos);
    controls.target.copy(initialCamRef.current.target);
    controls.update();
  };

  const handleScreenshot = () => {
    const renderer = rendererRef.current;
    if (!renderer) return;
    const url = renderer.domElement.toDataURL("image/png");
    const a = document.createElement("a");
    a.href = url;
    a.download = "screenshot.png";
    a.click();
  };

  return {
    canvasRef,
    sceneRef,
    cameraRef,
    rendererRef,
    controlsRef,
    stlMaterialRef,
    initialCamRef,
    modelRootRef,
    loadingProgress,
    handleResetView,
    handleScreenshot,
  } as const;
}
