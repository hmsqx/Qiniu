import React from "react";
import { buildAssetUrl } from "@/utils/asset";
import { Download, ImageOff, Loader2, Globe2, Lock } from "lucide-react";
import type { JobItem } from "@/api/mode3D";
import { Checkbox } from "@/components/ui/checkbox";
import { StatusBadge } from "./StatusBadge";
import { Link } from "react-router-dom";
import { downloadModelFile } from "@/utils/download";
import { getExt, toProxiedUrl } from "@/utils/url";

interface JobCardProps {
  item: JobItem;
  selectable?: boolean; // 是否显示复选框
  selected?: boolean; // 当前是否选中
  onToggleSelect?: (jobId: string) => void;
  onToggleVisibility?: (job: JobItem) => void; // 单个切换
  toggling?: boolean; // 当前是否在切换此任务
}

export function JobCard({
  item,
  selectable,
  selected,
  onToggleSelect,
  onToggleVisibility,
  toggling,
}: JobCardProps) {
  const rawStatus = (item.status || "").toUpperCase();
  const isDone = rawStatus === "DONE" || rawStatus === "SUCCEED";
  const isProcessing =
    rawStatus === "RUN" || rawStatus === "WAITING" || rawStatus === "QUEUE";
  const hasPreview = !!item.imgUrl;
  const [downloading, setDownloading] = React.useState(false);

  function handleDownload(e: React.MouseEvent) {
    e.preventDefault();
    if (!item.modelUrl || downloading) return;
    setDownloading(true);
    downloadModelFile(item.modelUrl, {
      jobId: item.jobId,
      fileName: item.jobId,
      extHint: getExt(item.modelUrl) || undefined,
      onSuccess: () => setDownloading(false),
      onError: () => setDownloading(false),
    });
  }

  return (
    <div
      className={`relative bg-slate-800/50 rounded-lg p-3 flex flex-col hover:shadow-xl hover:shadow-purple-900/10 transition-all duration-300 border border-slate-700/30 hover:border-purple-500/40 hover:-translate-y-0.5 ${
        selected ? "ring-2 ring-purple-500 border-purple-500" : ""
      }`}
      onClick={() => {
        if (selectable && onToggleSelect) onToggleSelect(item.jobId);
      }}
    >
      <div className="w-full h-60 bg-black/20 rounded-md flex items-center justify-center overflow-hidden relative group">
        {hasPreview ? (
          <img
            src={buildAssetUrl(item.imgUrl) || undefined}
            alt={item.jobId}
            className="w-full h-full object-cover group-hover:scale-105 transition-transform duration-300"
          />
        ) : (
          <div className="flex flex-col items-center justify-center text-slate-400">
            <ImageOff className="w-10 h-10 mb-2" />
            <div className="text-sm">任务处理中</div>
          </div>
        )}

        <div className="absolute left-2 top-2">
          <StatusBadge status={item.status} />
        </div>

        {typeof item.isPrivate === "boolean" && !selectable && (
          <button
            type="button"
            title={item.isPrivate ? "设为公开" : "设为私有"}
            onClick={(e) => {
              e.stopPropagation();
              if (toggling) return;
              onToggleVisibility?.(item);
            }}
            disabled={toggling}
            className="absolute right-2 top-2 text-[10px] px-1.5 py-0.5 rounded bg-black/60 backdrop-blur border border-white/10 font-medium tracking-wide flex items-center gap-1 hover:bg-black/70 disabled:opacity-50"
          >
            {toggling ? (
              <Loader2 className="w-3 h-3 animate-spin" />
            ) : item.isPrivate ? (
              <Lock className="w-3 h-3" />
            ) : (
              <Globe2 className="w-3 h-3" />
            )}
            {item.isPrivate ? "私有" : "公开"}
          </button>
        )}

        {selectable && (
          <div className="absolute left-2 bottom-2">
            <Checkbox
              checked={!!selected}
              onCheckedChange={() => onToggleSelect?.(item.jobId)}
              onClick={(e) => e.stopPropagation()}
              className="size-5 border-white/40 data-[state=checked]:bg-purple-600 data-[state=checked]:border-purple-600"
            />
          </div>
        )}

        {isProcessing && (
          <div className="absolute inset-0 flex items-center justify-center bg-black/20">
            <Loader2 className="w-6 h-6 text-purple-300 animate-spin" />
          </div>
        )}
      </div>

      <div className="mt-3 flex items-center justify-between text-xs text-slate-400">
        <div className="truncate font-mono">ID: {item.jobId}</div>
        <div className="flex items-center gap-3">
          {isDone && item.modelUrl ? (
            <>
              <Link
                to={`/viewer?url=${encodeURIComponent(
                  toProxiedUrl(item.modelUrl)
                )}&format=${
                  getExt(item.modelUrl) || ""
                }&jobId=${encodeURIComponent(item.jobId)}`}
                className="inline-flex items-center gap-1 text-emerald-400 hover:underline hover:text-emerald-300"
              >
                <svg
                  viewBox="0 0 24 24"
                  className="w-3.5 h-3.5"
                  fill="none"
                  stroke="currentColor"
                  strokeWidth="2"
                  strokeLinecap="round"
                  strokeLinejoin="round"
                >
                  <path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z" />
                  <circle cx="12" cy="12" r="3" />
                </svg>
                预览
              </Link>
              <button
                onClick={handleDownload}
                disabled={downloading}
                className="inline-flex items-center text-purple-400 hover:text-purple-300 disabled:opacity-60"
                aria-label="下载模型"
              >
                {downloading ? (
                  <Loader2 className="w-4 h-4 animate-spin" />
                ) : (
                  <div className="flex items-center gap-1">
                    <Download className="w-4 h-4" />
                    <span>下载</span>
                  </div>
                )}
              </button>
            </>
          ) : (
            <div className="h-5" />
          )}
        </div>
      </div>
    </div>
  );
}
