import { Button } from "@/components/ui/button";
import { Checkbox } from "@/components/ui/checkbox";
import { RotateCw, Globe2, Lock, SquareCheckBig, X } from "lucide-react";

interface ToolbarProps {
  loading: boolean;
  onRefresh: () => void;
  selectionMode: boolean;
  onEnterSelectionMode: () => void;
  onExitSelectionMode: () => void;
  onToggleAll: () => void;
  allSelected: boolean;
  selectedCount: number;
  totalCount: number;
  onBulkPublic: () => void;
  onBulkPrivate: () => void;
  disableBulkAction?: boolean;
}

export function Toolbar({
  loading,
  onRefresh,
  selectionMode,
  onEnterSelectionMode,
  onExitSelectionMode,
  onToggleAll,
  allSelected,
  selectedCount,
  totalCount,
  onBulkPublic,
  onBulkPrivate,
  disableBulkAction,
}: ToolbarProps) {
  return (
    <div className="flex items-center justify-between pb-4 gap-4 flex-wrap">
      <div className="flex items-center gap-4 text-sm">
        {!selectionMode && (
          <button
            onClick={onEnterSelectionMode}
            className="inline-flex items-center gap-1 text-white/80 hover:text-white transition-colors"
          >
            <SquareCheckBig className="w-4 h-4" /> 批量操作
          </button>
        )}
        {selectionMode && (
          <div className="flex items-center gap-3 flex-wrap">
            <div className="flex items-center gap-2 text-white/70">
              <Checkbox
                checked={allSelected}
                onCheckedChange={onToggleAll}
                className="size-4 border-white/30 data-[state=checked]:bg-purple-600 data-[state=checked]:border-purple-600"
              />
              <span className="whitespace-nowrap">
                已选 {selectedCount}/{totalCount}
              </span>
            </div>
            <div className="flex items-center gap-2">
              <Button
                size="sm"
                variant="outline"
                disabled={selectedCount === 0 || disableBulkAction || loading}
                onClick={onBulkPublic}
                className="bg-transparent border-emerald-500/40 text-emerald-400 hover:bg-emerald-500/10 disabled:opacity-50"
              >
                <Globe2 className="w-4 h-4 mr-1" /> 设为公开
              </Button>
              <Button
                size="sm"
                variant="outline"
                disabled={selectedCount === 0 || disableBulkAction || loading}
                onClick={onBulkPrivate}
                className="bg-transparent border-white/30 text-white/80 hover:bg-white/10 disabled:opacity-50"
              >
                <Lock className="w-4 h-4 mr-1" /> 设为私有
              </Button>
            </div>
            <button
              onClick={onExitSelectionMode}
              className="inline-flex items-center gap-1 text-white/60 hover:text-white text-sm px-2 py-1 rounded border border-white/20 hover:bg-white/10 transition-colors"
            >
              <X className="w-3.5 h-3.5" /> 退出
            </button>
            <div className="flex items-center text-xs text-white/50 gap-1">
              <Lock className="w-3.5 h-3.5" /> 私有任务可切换为公开
            </div>
          </div>
        )}
      </div>
      <div className="flex items-center gap-3 ml-auto">
        <Button
          variant="outline"
          size="sm"
          onClick={onRefresh}
          disabled={loading}
          className="bg-transparent border-white/20 hover:bg-white/10 text-white"
        >
          <RotateCw
            className={`h-4 w-4 mr-2 ${loading ? "animate-spin" : ""}`}
          />
          刷新
        </Button>
      </div>
    </div>
  );
}
