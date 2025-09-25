import React, { useEffect, useMemo, useRef, useState } from "react";
import { useSearchParams } from "react-router-dom";
import { downloadModelFile } from "@/utils/download";
import { Download, Loader2 } from "lucide-react";
import * as THREE from "three";
import { OrbitControls } from "three/examples/jsm/controls/OrbitControls.js";
import { GLTFLoader } from "three/examples/jsm/loaders/GLTFLoader.js";
import { OBJLoader } from "three/examples/jsm/loaders/OBJLoader.js";
import { FBXLoader } from "three/examples/jsm/loaders/FBXLoader.js";
import { STLLoader } from "three/examples/jsm/loaders/STLLoader.js";
import { MTLLoader } from "three/examples/jsm/loaders/MTLLoader.js";

function useContainerSize(
  containerRef: React.RefObject<HTMLDivElement | null>
) {
  const [size, setSize] = useState({ width: 0, height: 0 });
  useEffect(() => {
    if (!containerRef.current) return;
    const el = containerRef.current;
    const ro = new ResizeObserver(() => {
      setSize({ width: el.clientWidth, height: el.clientHeight });
    });
    ro.observe(el);
    setSize({ width: el.clientWidth, height: el.clientHeight });
    return () => ro.disconnect();
  }, [containerRef]);
  return size;
}

function getFormatFromUrl(url: string | null): string | null {
  if (!url) return null;
  const m = url
    .split("?")[0]
    .split("#")[0]
    .match(/\.([a-z0-9]+)$/i);
  return m ? m[1].toLowerCase() : null;
}

const Viewer: React.FC = () => {
  const [search] = useSearchParams();
  const rawUrl = search.get("url");

  const urlParam = useMemo(() => {
    if (!rawUrl) return rawUrl;
    try {
      const u = new URL(rawUrl, window.location.origin);
      if (u.hostname.endsWith("tencentcos.cn")) {
        console.log("Proxying model URL through /model");
        console.log("/model" + u.pathname + (u.search || ""));
        return "/model" + u.pathname + (u.search || "");
      }
      return rawUrl;
    } catch {
      return rawUrl;
    }
  }, [rawUrl]);

  const formatParam = (search.get("format") || "").toLowerCase();
  const jobIdParam = search.get("jobId") || "";
  const format = useMemo(
    () => formatParam || getFormatFromUrl(urlParam) || "",
    [formatParam, urlParam]
  );

  const containerRef = useRef<HTMLDivElement>(null);
  const canvasRef = useRef<HTMLCanvasElement | null>(null);
  const { width, height } = useContainerSize(containerRef);

  const isVideo = format === "mp4";
  const isUsd = format === "usdz" || format === "usd";

  const [showGrid, setShowGrid] = useState(true);
  const [autoRotate, setAutoRotate] = useState(false);
  const [stlColor, setStlColor] = useState<string>("#aaaaaa");
  const [downloading, setDownloading] = useState(false);

  function handleDownload(e: React.MouseEvent) {
    e.preventDefault();
    if (!urlParam || downloading) return;
    setDownloading(true);
    downloadModelFile(urlParam, {
      jobId: jobIdParam || undefined,
      fileName: jobIdParam || "model",
      extHint: format || undefined,
      onSuccess: () => setDownloading(false),
      onError: () => setDownloading(false),
    });
  }

  // Refs for three objects
  const sceneRef = useRef<THREE.Scene | null>(null);
  const cameraRef = useRef<THREE.PerspectiveCamera | null>(null);
  const rendererRef = useRef<THREE.WebGLRenderer | null>(null);
  const controlsRef = useRef<OrbitControls | null>(null);
  const gridRef = useRef<THREE.GridHelper | null>(null);
  const stlMaterialRef = useRef<THREE.MeshStandardMaterial | null>(null);
  const initialCamRef = useRef<{
    pos: THREE.Vector3;
    target: THREE.Vector3;
  } | null>(null);
  const modelRootRef = useRef<THREE.Object3D | null>(null);

  useEffect(() => {
    if (!containerRef.current || !urlParam || isVideo || isUsd) return;
    const safeUrl = urlParam as string; // narrowed by guard above

    const container = containerRef.current;

    // Loading manager: rewrite any sub-asset URL (textures/MTL) to go through /model proxy when needed
    const manager = new THREE.LoadingManager();
    const proxyify = (inputUrl: string) => {
      try {
        if (/^(data:|blob:)/i.test(inputUrl)) return inputUrl;
        // keep relative paths as-is (they will resolve against resourcePath we set)
        if (/^(\.\.\/|\.\/|\/)/.test(inputUrl)) return inputUrl;
        const u = new URL(inputUrl, window.location.origin);
        if (u.hostname.endsWith("tencentcos.cn")) {
          return "/model" + u.pathname + (u.search || "");
        }
        return inputUrl;
      } catch {
        return inputUrl;
      }
    };
    manager.setURLModifier(proxyify);

    const renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });
    renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
    renderer.setSize(container.clientWidth, container.clientHeight);
    renderer.outputColorSpace = THREE.SRGBColorSpace;
    container.appendChild(renderer.domElement);
    canvasRef.current = renderer.domElement;
    rendererRef.current = renderer;

    const scene = new THREE.Scene();
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

    // lights
    const hemi = new THREE.HemisphereLight(0xffffff, 0x444444, 1.0);
    hemi.position.set(0, 20, 0);
    scene.add(hemi);

    const dir = new THREE.DirectionalLight(0xffffff, 1.0);
    dir.position.set(5, 10, 7.5);
    dir.castShadow = true;
    scene.add(dir);

    const grid = new THREE.GridHelper(10, 20, 0x444444, 0x222222);
    grid.position.y = -0.001;
    scene.add(grid);
    gridRef.current = grid;
    grid.visible = showGrid;

    const controls = new OrbitControls(camera, renderer.domElement);
    controls.enableDamping = true;
    controls.target.set(0, 0.5, 0);
    controlsRef.current = controls;
    controls.autoRotate = autoRotate;

    // load model
    function centerAndScale(object: THREE.Object3D) {
      const box = new THREE.Box3().setFromObject(object);
      const size = new THREE.Vector3();
      const center = new THREE.Vector3();
      box.getSize(size);
      box.getCenter(center);
      const maxDim = Math.max(size.x, size.y, size.z) || 1;
      const scale = 2.0 / maxDim; // scale model to fit nicely
      object.position.sub(center);
      object.position.multiplyScalar(scale);
      object.scale.setScalar(scale);
    }

    async function load() {
      try {
        let root: THREE.Object3D | null = null;
        if (format === "glb" || format === "gltf") {
          const loader = new GLTFLoader(manager);
          loader.setCrossOrigin("anonymous");
          const basePath = new URL(
            safeUrl,
            window.location.origin
          ).href.replace(/[^/]*$/g, "");
          loader.setResourcePath(basePath);
          const gltf = await loader.loadAsync(safeUrl);
          root = gltf.scene;
        } else if (format === "obj") {
          // Try to load MTL alongside OBJ for colors/textures
          try {
            const urlObj = new URL(safeUrl, window.location.origin);
            const basePath = urlObj.href.replace(/[^/]*$/g, "");
            const mtlUrl = urlObj.href.replace(/\.obj(\?.*)?$/i, ".mtl$1");
            const mtlLoader = new MTLLoader(manager);
            mtlLoader.setCrossOrigin("anonymous");
            mtlLoader.setResourcePath(basePath);
            const materials = await mtlLoader.loadAsync(mtlUrl);
            materials.preload();
            const objLoader = new OBJLoader(manager);
            objLoader.setMaterials(materials);
            root = await objLoader.loadAsync(safeUrl);
          } catch (e) {
            const objLoader = new OBJLoader(manager);
            root = await objLoader.loadAsync(safeUrl);
          }
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
      if (!container) return;
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
  }, [urlParam, format, isVideo, isUsd]);

  // react to UI changes (background removed by requirement)

  useEffect(() => {
    if (gridRef.current) gridRef.current.visible = showGrid;
  }, [showGrid]);

  useEffect(() => {
    if (controlsRef.current) controlsRef.current.autoRotate = autoRotate;
  }, [autoRotate]);

  useEffect(() => {
    if (stlMaterialRef.current) {
      stlMaterialRef.current.color = new THREE.Color(stlColor);
      stlMaterialRef.current.needsUpdate = true;
    }
  }, [stlColor]);

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

  if (!urlParam) {
    return (
      <div className="flex items-center justify-center h-full text-slate-300">
        缺少 url 参数
      </div>
    );
  }

  if (isUsd) {
    return (
      <div className="p-4 text-slate-200 space-y-2">
        <div>USDZ 文件暂不在网页中直接渲染。</div>
        <a
          className="text-purple-400 underline"
          href={urlParam}
          target="_blank"
          rel="noreferrer"
        >
          在系统中打开或下载
        </a>
      </div>
    );
  }

  if (isVideo) {
    return (
      <div className="w-full h-full flex items-center justify-center bg-black">
        <video src={urlParam} controls className="max-w-full max-h-full" />
      </div>
    );
  }

  return (
    <div ref={containerRef} className="w-full h-[calc(100vh-64px)] relative">
      {/* canvas injected by three */}
      <div className="absolute left-4 top-2 text-slate-300 text-sm bg-black/30 px-2 py-1 rounded">
        旋转: 拖拽 缩放: 滚轮 平移: 右键
      </div>
      <div className="absolute right-4 top-2 text-slate-400 text-xs font-mono flex items-center gap-3">
        <span>
          {format.toUpperCase()} | {Math.round(width)}x{Math.round(height)}
        </span>
        <div className="flex items-center gap-2 bg-black/30 px-2 py-1 rounded">
          <button
            onClick={handleDownload}
            disabled={downloading}
            className="flex items-center gap-1 text-emerald-300 hover:text-emerald-200 disabled:opacity-60"
            title="下载模型"
          >
            {downloading ? (
              <Loader2 className="w-3.5 h-3.5 animate-spin" />
            ) : (
              <>
                <Download className="w-3.5 h-3.5" />
                <span>下载</span>
              </>
            )}
          </button>
          <button
            onClick={handleScreenshot}
            className="text-sky-300 hover:text-sky-200"
            title="截图"
          >
            截图
          </button>
          <button
            onClick={() => setShowGrid((v) => !v)}
            className="text-purple-300 hover:text-purple-200"
            title="网格"
          >
            网格
          </button>
          <button
            onClick={() => setAutoRotate((v) => !v)}
            className="text-orange-300 hover:text-orange-200"
            title="旋转"
          >
            旋转
          </button>
          <button
            onClick={handleResetView}
            className="text-pink-300 hover:text-pink-200"
            title="重置"
          >
            重置
          </button>
          {format === "stl" && (
            <label
              className="flex items-center gap-1 text-slate-300"
              title="模型颜色"
            >
              颜色
              <input
                type="color"
                value={stlColor}
                onChange={(e) => setStlColor(e.target.value)}
                className="w-5 h-5 p-0 bg-transparent border-0"
              />
            </label>
          )}
        </div>
      </div>
    </div>
  );
};

export default Viewer;
