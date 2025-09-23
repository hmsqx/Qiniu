import { Button } from "@/components/ui/button";

interface PaginationProps {
  total: number;
  pageNum: number;
  totalPages: number;
  loading: boolean;
  onPrev: () => void;
  onNext: () => void;
}

export function Pagination({
  total,
  pageNum,
  totalPages,
  loading,
  onPrev,
  onNext,
}: PaginationProps) {
  if (total <= 0) return null;
  return (
    <div className="mt-8 flex flex-col sm:flex-row items-center justify-between text-sm gap-4">
      <div className="text-slate-400">
        共 {total} 条 · 第 {pageNum} / {totalPages} 页
      </div>
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
    </div>
  );
}
