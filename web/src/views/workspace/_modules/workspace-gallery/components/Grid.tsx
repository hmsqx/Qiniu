import type { PropsWithChildren } from "react";
import { ImageOff, AlertTriangle } from "lucide-react";

export function Grid({ children }: PropsWithChildren) {
  return (
    <div className="grid gap-6 grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4 2xl:grid-cols-5">
      {children}
    </div>
  );
}

type StatePanelProps = {
  icon: React.ReactNode;
  title: string;
  description?: React.ReactNode;
  tone?: "neutral" | "error";
  size?: "normal" | "tall";
};

function StatePanel({
  icon,
  title,
  description,
  tone = "neutral",
  size = "tall",
}: StatePanelProps) {
  const base =
    "col-span-full relative overflow-hidden rounded-lg md:rounded-xl text-center select-none flex flex-col items-center justify-center";
  const spacing = size === "tall" ? "min-h-[700px] px-6 py-24" : "px-6 py-16";
  const palette =
    tone === "error"
      ? [
          "border border-red-400/5 dark:border-red-500/15",
          "bg-[linear-gradient(to_bottom,rgba(244,63,94,0.025),rgba(244,63,94,0.008))] dark:bg-[linear-gradient(to_bottom,rgba(190,18,60,0.14),rgba(190,18,60,0.04))]",
          "text-red-500 dark:text-red-300",
        ].join(" ")
      : [
          "border border-slate-400/5 dark:border-slate-600/20",
          "bg-[linear-gradient(to_bottom,rgba(100,116,139,0.045),rgba(100,116,139,0.012))] dark:bg-[linear-gradient(to_bottom,rgba(71,85,105,0.32),rgba(71,85,105,0.08))]",
          "text-slate-600 dark:text-slate-300",
        ].join(" ");
  return (
    <div className={`${base} ${spacing} ${palette}`}>
      <div className="flex h-14 w-14 items-center justify-center rounded-md bg-white/45 dark:bg-slate-800/55 ring-1 ring-slate-200/40 dark:ring-slate-700/70 shadow-sm">
        {icon}
      </div>
      <h3 className="mt-5 text-base md:text-lg font-semibold tracking-wide text-current">
        {title}
      </h3>
      {description && (
        <div className="mt-2 text-xs md:text-sm leading-relaxed max-w-md mx-auto text-current/70">
          {description}
        </div>
      )}
    </div>
  );
}

export function EmptyState() {
  return (
    <StatePanel
      size="tall"
      icon={<ImageOff className="h-7 w-7 text-slate-400 dark:text-slate-500" />}
      title="暂无数据"
      description={<p>暂无可展示内容，调整筛选或稍后点击刷新。</p>}
    />
  );
}

export function ErrorState({ message }: { message: string }) {
  return (
    <StatePanel
      size="tall"
      tone="error"
      icon={<AlertTriangle className="h-7 w-7" />}
      title="加载出错"
      description={
        <p className="break-words">{message || "请求失败，请稍后再试。"}</p>
      }
    />
  );
}
