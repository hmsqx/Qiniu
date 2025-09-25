export function SkeletonCard() {
  return (
    <div className="bg-slate-800/40 rounded-lg p-3 flex flex-col animate-pulse border border-slate-700/40">
      <div className="w-full h-60 bg-slate-700/40 rounded-md" />
      <div className="mt-3 flex items-center justify-between">
        <div className="h-4 w-2/3 bg-slate-700/40 rounded" />
        <div className="h-4 w-1/4 bg-slate-700/40 rounded" />
      </div>
    </div>
  );
}
