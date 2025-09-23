import type { PropsWithChildren } from "react";

export function Grid({ children }: PropsWithChildren) {
  return (
    <div className="grid gap-6 grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4 2xl:grid-cols-5">
      {children}
    </div>
  );
}

export function EmptyState() {
  return (
    <div className="col-span-full text-center text-slate-500 py-16">
      <p>暂无数据</p>
      <p className="text-sm mt-2">点击“刷新”按钮或稍后再试。</p>
    </div>
  );
}

export function ErrorState({ message }: { message: string }) {
  return (
    <div className="col-span-full text-center text-red-400 bg-red-500/10 rounded-lg p-6">
      <p className="font-semibold">加载出错</p>
      <p className="text-sm mt-1">{message}</p>
    </div>
  );
}
