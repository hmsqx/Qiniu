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
  const { handleResetView, handleScreenshot } = useThreeViewer(containerRef, {
    url: urlParam,
    format,
    isVideo,
    isUsd,
    showGrid,
    autoRotate,
    stlColor,
  });

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
        onToggleGrid={() => setShowGrid((v) => !v)}
        onToggleRotate={() => setAutoRotate((v) => !v)}
        onReset={handleResetView}
        stlColor={stlColor}
        onStlColor={setStlColor}
      />
    </div>
  );
};

export default Viewer;
