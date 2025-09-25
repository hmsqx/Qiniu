import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { useEffect, useState } from "react";
import { Loader2 } from "lucide-react";

interface PaginationProps {
  total: number;
  pageNum: number;
  totalPages: number;
  loading: boolean;
  onPrev: () => void;
  onNext: () => void;
  onJump?: (page: number) => void; // 新增: 直接跳转
}

export function Pagination({
  total,
  pageNum,
  totalPages,
  loading,
  onPrev,
  onNext,
  onJump,
}: PaginationProps) {
  const [jumpVal, setJumpVal] = useState("");
  const [jumpError, setJumpError] = useState("");
  const [jumping, setJumping] = useState(false);
  if (total <= 0) return null;

  const buildPages = () => {
    const span = 2;
    const pages: (number | string)[] = [];
    const start = Math.max(1, pageNum - span);
    const end = Math.min(totalPages, pageNum + span);
    // 首页
    if (start > 1) {
      pages.push(1);
      if (start > 2) pages.push("ellipsis-start");
    }
    for (let p = start; p <= end; p++) pages.push(p);
    if (end < totalPages) {
      if (end < totalPages - 1) pages.push("ellipsis-end");
      pages.push(totalPages);
    }
    return pages;
  };
  const pages = buildPages();

  const validate = (raw: string) => {
    if (!raw) {
      setJumpError("");
      return;
    }
    const n = Number(raw);
    if (!Number.isInteger(n) || n < 1 || n > totalPages) {
      setJumpError(`范围 1-${totalPages}`);
    } else {
      setJumpError("");
    }
  };

  const triggerJump = () => {
    if (!onJump) return;
    if (jumpError || !jumpVal) return;
    const n = parseInt(jumpVal, 10);
    if (!isNaN(n)) {
      const target = Math.min(Math.max(1, n), totalPages);
      if (target !== pageNum) {
        setJumping(true);
        onJump(target);
      }
    }
  };

  useEffect(() => {
    // 数据加载完后结束 jumping 状态
    if (!loading && jumping) setJumping(false);
  }, [loading, jumping]);

  return (
    <div className="mt-8 flex flex-col md:flex-row md:items-center md:justify-between text-sm gap-4">
      <div className="text-slate-400 flex flex-wrap items-center gap-3">
        <span>
          共 {total} 条 · 第 {pageNum} / {totalPages} 页
        </span>
        <div className="flex items-center gap-1 min-h-8">
          {loading && (
            <div className="flex gap-1">
              {Array.from({ length: 5 }).map((_, i) => (
                <div
                  key={i}
                  className="w-8 h-8 rounded-md bg-white/10 animate-pulse"
                />
              ))}
            </div>
          )}
          {!loading &&
            pages.map((p) => {
              if (typeof p === "string" && p.startsWith("ellipsis")) {
                return (
                  <span key={p} className="px-1 text-slate-500 select-none">
                    …
                  </span>
                );
              }
              const num = p as number;
              return (
                <Button
                  key={num}
                  size="sm"
                  variant={num === pageNum ? "default" : "outline"}
                  disabled={loading}
                  onClick={() => onJump && onJump(num)}
                  className={
                    num === pageNum
                      ? "bg-white/20 text-white border-white/30"
                      : "bg-transparent border-white/20 hover:bg-white/10 text-white"
                  }
                >
                  {num}
                </Button>
              );
            })}
        </div>
      </div>
      <div className="flex flex-col sm:flex-row items-center gap-3">
        <div className="flex items-center gap-2">
          <Button
            variant="outline"
            size="sm"
            onClick={onPrev}
            disabled={pageNum <= 1 || loading}
            className="bg-transparent border-white/20 hover:bg-white/10 text-white"
          >
            上一页
          </Button>
          <Button
            variant="outline"
            size="sm"
            onClick={onNext}
            disabled={pageNum >= totalPages || loading}
            className="bg-transparent border-white/20 hover:bg-white/10 text-white"
          >
            下一页
          </Button>
        </div>
        {onJump && (
          <div className="flex items-center gap-2">
            <div className="flex items-center gap-2 bg-white/5 rounded px-2 py-1">
              <span className="text-xs text-slate-400">跳转</span>
              <Input
                value={jumpVal}
                onChange={(e) => {
                  const v = e.target.value.replace(/[^0-9]/g, "");
                  setJumpVal(v);
                  validate(v);
                }}
                onKeyDown={(e) => e.key === "Enter" && triggerJump()}
                className="w-16 h-8 text-center bg-transparent border-white/20 focus-visible:ring-0 focus-visible:border-white/40 text-white placeholder:text-slate-500"
                placeholder="页码"
              />
              <Button
                size="sm"
                variant="outline"
                disabled={loading || !jumpVal || !!jumpError}
                onClick={triggerJump}
                className="bg-transparent border-white/20 hover:bg-white/10 text-white"
              >
                {jumping ? <Loader2 className="w-4 h-4 animate-spin" /> : "GO"}
              </Button>
              {jumpError && (
                <span className="text-[10px] text-red-400 px-1">
                  {jumpError}
                </span>
              )}
            </div>
          </div>
        )}
      </div>
    </div>
  );
}
