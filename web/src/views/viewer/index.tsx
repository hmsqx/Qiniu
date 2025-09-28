import React, { useMemo, useRef, useState } from "react";
import { useSearchParams } from "react-router-dom";
import { downloadModelFile } from "@/utils/download";
import Toolbar from "./_modules/Toolbar";
import useContainerSize from "./hooks/useContainerSize";
import { useThreeViewer } from "./hooks/useThreeViewer";
import { getFormatFromUrl } from "./utils/format";

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
  const { width, height } = useContainerSize(containerRef);

  const isVideo = format === "mp4";
  const isUsd = format === "usdz" || format === "usd";

  const [autoRotate, setAutoRotate] = useState(false);
  const defaultStlColor = "#aaaaaa";
  const [stlColor, setStlColor] = useState<string>(defaultStlColor);
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
  const { handleResetView, handleScreenshot, loadingProgress } = useThreeViewer(
    containerRef,
    {
      url: urlParam,
      format,
      isVideo,
      isUsd,
      autoRotate,
      stlColor,
    }
  );

  const handleResetAll = () => {
    handleResetView();
    setStlColor(defaultStlColor);
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
          下载到本地查看
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
    <div ref={containerRef} className="w-full h-[calc(100%-64px)] relative">
      {loadingProgress > 0 && loadingProgress < 100 && (
        <div className="absolute inset-0 flex items-center justify-center bg-black/40 z-10">
          <div className="text-slate-200 text-sm bg-black/60 px-3 py-2 rounded">
            正在加载模型… {loadingProgress}%
          </div>
        </div>
      )}
      <div className="absolute left-4 top-2 text-slate-300 text-sm bg-black/30 px-2 py-1 rounded">
        旋转: 拖拽 缩放: 滚轮 平移: 右键
      </div>
      <Toolbar
        format={format}
        width={width}
        height={height}
        downloading={downloading}
        onDownload={handleDownload}
        onScreenshot={handleScreenshot}
        onToggleRotate={() => setAutoRotate((v) => !v)}
        onReset={handleResetAll}
        stlColor={stlColor}
        onStlColor={setStlColor}
      />
    </div>
  );
};

export default Viewer;
