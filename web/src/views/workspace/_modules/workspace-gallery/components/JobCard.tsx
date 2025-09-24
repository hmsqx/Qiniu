import { Download, ImageOff, Loader2 } from "lucide-react";
import type { JobItem } from "@/api/mode3D";
import { StatusBadge } from "./StatusBadge";
import { Link } from "react-router-dom";

function getExt(url?: string | null): string | null {
  if (!url) return null;
  const m = url
    .split("?")[0]
    .split("#")[0]
    .match(/\.([a-z0-9]+)$/i);
  return m ? m[1].toLowerCase() : null;
}

function toProxiedUrl(url?: string | null): string {
  if (!url) return "";
  try {
    const u = new URL(url);
    // 将 COS 域名的绝对地址改写为 /cos 前缀，交给 Vite 代理
    if (u.hostname.endsWith("tencentcos.cn")) {
      return "/cos" + u.pathname + (u.search || "");
    }
    // 其它域名：保持原样（若目标已允许 CORS）
    return url;
  } catch {
    return url || "";
  }
}

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
        <div className="flex items-center gap-3">
          {isDone && item.modelUrl ? (
            <>
              <Link
                to={`/viewer?url=${encodeURIComponent(
                  toProxiedUrl(item.modelUrl)
                )}&format=${getExt(item.modelUrl) || ""}`}
                className="inline-flex items-center gap-1 text-emerald-400 hover:underline hover:text-emerald-300"
              >
                {/* Using Download icon for consistency if View not available */}
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
              <a
                href={item.modelUrl}
                target="_blank"
                rel="noreferrer"
                className="inline-flex items-center gap-1 text-purple-400 hover:underline hover:text-purple-300"
              >
                <Download className="w-3.5 h-3.5" /> 下载
              </a>
            </>
          ) : (
            <div className="h-5" />
          )}
        </div>
      </div>
    </div>
  );
}
