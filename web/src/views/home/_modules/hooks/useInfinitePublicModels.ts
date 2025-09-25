import { useCallback, useEffect, useRef, useState } from "react";
import { queryPublicModels } from "@/api/public";
import type { QueryPublicModelsResult } from "@/api/public";
import type { Inspiration } from "../type";

export interface UseInfinitePublicModelsOptions {
  pageSize?: number;
  enabled?: boolean;
}

export interface UseInfinitePublicModelsResult {
  items: Inspiration[];
  loading: boolean;
  loadingMore: boolean;
  error: string | null;
  hasMore: boolean;
  loadMore: () => void;
  reset: () => void;
  retry: () => void;
  observerRef: (el: HTMLElement | null) => void;
}

function mapToInspiration(item: any): Inspiration {
  const image = item.imgUrl || item.previewImages || item.coverUrl || "";
  return {
    id: String(item.modelId || item.jobId || item.id || ""),
    title: item.modelId || item.jobId || "模型",
    author: item.userId ? `User-${String(item.userId).slice(0, 6)}` : "Public",
    tags: [item.resultFormat || "Model"],
    image,
    views: Math.floor(2000 + Math.random() * 20000),
    status: item.status,
    downloadCount: item.downloadCount,
    like: item.like,
    resultFormat: item.resultFormat,
    createTime: item.create_time,
    userId: item.userId,
    version: item.version,
    isPrivate: item.Isprivate,
  };
}

export function useInfinitePublicModels(
  opts: UseInfinitePublicModelsOptions = {}
): UseInfinitePublicModelsResult {
  const { pageSize = 24, enabled = true } = opts;
  const [pageNum, setPageNum] = useState(1);
  const [items, setItems] = useState<Inspiration[]>([]);
  const [loading, setLoading] = useState(false);
  const [loadingMore, setLoadingMore] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [hasMore, setHasMore] = useState(true);
  const totalRef = useRef<number>(0);

  const itemsCountRef = useRef(0);

  const fetchPage = useCallback(
    async (page: number, append: boolean) => {
      if (!enabled) return;
      try {
        if (page === 1 && !append) setLoading(true);
        else setLoadingMore(true);
        setError(null);
        const res: QueryPublicModelsResult = await queryPublicModels({
          pageNum: page,
          pageSize,
        });
        totalRef.current = res.total;
        const mapped = res.list.map(mapToInspiration);

        setItems((prev) => {
          const next = append ? [...prev, ...mapped] : mapped;
          itemsCountRef.current = next.length;
          return next;
        });

        const returned = mapped.length;
        const totalLoadedAfter = append ? itemsCountRef.current : mapped.length;
        const more = returned === pageSize && totalLoadedAfter < res.total;
        setHasMore(more);
      } catch (e: any) {
        setError(e?.message || "加载失败");
        // [修改] 关键修复：失败时停止后续的自动加载，防止无限循环
        setHasMore(false);
      } finally {
        setLoading(false);
        setLoadingMore(false);
      }
    },
    [pageSize, enabled]
  );

  useEffect(() => {
    if (enabled) fetchPage(1, false);
  }, [enabled, fetchPage]);

  const loadMore = useCallback(() => {
    if (loading || loadingMore || !hasMore) return;
    const next = pageNum + 1;
    setPageNum(next);
    fetchPage(next, true);
  }, [fetchPage, hasMore, loading, loadingMore, pageNum]);

  const reset = useCallback(() => {
    setPageNum(1);
    setItems([]);
    setHasMore(true);
    setError(null);
    fetchPage(1, false);
  }, [fetchPage]);

  // [新增] 创建一个专门的 retry 函数
  const retry = useCallback(() => {
    if (loading || loadingMore) return;
    setError(null);
    setHasMore(true); // 重新允许加载
    // 重新请求当前失败的页面
    // `items.length > 0` 判断是重试第一页还是后续页
    fetchPage(pageNum, items.length > 0);
  }, [fetchPage, loading, loadingMore, pageNum, items.length]);

  // IntersectionObserver sentinel
  const observer = useRef<IntersectionObserver | null>(null);
  const observerRef = useCallback(
    (el: HTMLElement | null) => {
      if (observer.current) observer.current.disconnect();
      if (!el) return;
      observer.current = new IntersectionObserver(
        (entries) => {
          if (entries[0].isIntersecting) {
            loadMore();
            if (!hasMore) observer.current?.disconnect();
          }
        },
        { rootMargin: "200px 0px 0px 0px" }
      );
      observer.current.observe(el);
    },
    [loadMore, hasMore]
  );

  return {
    items,
    loading,
    loadingMore,
    error,
    hasMore,
    loadMore,
    reset,
    retry, // [新增] 导出 retry 函数
    observerRef,
  };
}
