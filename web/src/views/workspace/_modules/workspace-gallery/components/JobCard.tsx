import { Download, ImageOff, Loader2 } from "lucide-react";
import type { JobItem } from "@/api/mode3D";
import { StatusBadge } from "./StatusBadge";

export function JobCard({ item }: { item: JobItem }) {
  const statusText = item.status || "";
  const isDone = statusText.includes("完成");
  const isProcessing =
    statusText.includes("处理中") || statusText.includes("进行");
  const hasPreview = !!item.imgUrl;

  return (
    <div className="bg-slate-800/50 rounded-lg p-3 flex flex-col hover:shadow-xl hover:shadow-purple-900/10 transition-all duration-300 border border-slate-700/30 hover:border-purple-500/40 hover:-translate-y-0.5">
      <div className="w-full h-60 bg-black/20 rounded-md flex items-center justify-center overflow-hidden relative group">
        {hasPreview ? (
          <img
            src={item.imgUrl ?? undefined}
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

        {isProcessing && (
          <div className="absolute inset-0 flex items-center justify-center bg-black/20">
            <Loader2 className="w-6 h-6 text-purple-300 animate-spin" />
          </div>
        )}
      </div>

      <div className="mt-3 flex items-center justify-between text-xs text-slate-400">
        <div className="truncate font-mono">ID: {item.jobId}</div>
        {isDone && item.modelUrl ? (
          <a
            href={item.modelUrl}
            target="_blank"
            rel="noreferrer"
            className="inline-flex items-center gap-1 text-purple-400 hover:underline hover:text-purple-300"
          >
            <Download className="w-3.5 h-3.5" /> 下载
          </a>
        ) : (
          <div className="h-5" />
        )}
      </div>
    </div>
  );
}
