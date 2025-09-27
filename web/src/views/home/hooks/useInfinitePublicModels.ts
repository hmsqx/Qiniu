import { useCallback, useEffect, useRef, useState } from "react";
import { queryPublicModels } from "@/api/public";
import type { QueryPublicModelsResult } from "@/api/public";
import type { Inspiration } from "../data/type";
import { buildAssetUrl } from "@/utils/asset";

export interface UseInfinitePublicModelsOptions {
  pageSize?: number;
  enabled?: boolean;
  /** Optional scroll container to observe; defaults to viewport */
  root?: Element | null;
  /** Fallback scroll distance (px) from bottom to trigger loadMore when using custom root */
  fallbackScrollDistancePx?: number;
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
  const imageRaw = item.imgUrl || item.previewImages || item.coverUrl || "";
  const image = buildAssetUrl(imageRaw) || imageRaw || "";
  const id = String(item.modelId || item.jobId || item.id || "");
  const title = item.prompt || item.modelId || item.jobId || "模型";
  const author =
    item.username ||
    (item.userId ? `User-${String(item.userId).slice(0, 6)}` : "Public");
  return {
    id,
    title,
    author,
    tags: [item.resultFormat || "Model"],
    image,
    views:
      typeof item.viewCount === "number"
        ? item.viewCount
        : Math.floor(2000 + Math.random() * 20000),
    status: item.status,
    downloadCount: item.downloadCount,
    like: item.like,
    resultFormat: item.resultFormat,
    createTime: item.create_time,
    userId: item.userId,
    version: item.version,
    isPrivate: item.Isprivate ?? item.isPrivate,
    isLiked: !!item.islike,
  };
}

export function useInfinitePublicModels(
  opts: UseInfinitePublicModelsOptions = {}
): UseInfinitePublicModelsResult {
  const {
    pageSize = 24,
    enabled = true,
    root = null,
    fallbackScrollDistancePx = 300,
  } = opts;
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
        const mapped = res.list.map((i: any) => mapToInspiration(i.raw ?? i));

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

  const retry = useCallback(() => {
    if (loading || loadingMore) return;
    setError(null);
    setHasMore(true);
    fetchPage(pageNum, items.length > 0);
  }, [fetchPage, loading, loadingMore, pageNum, items.length]);

  const observer = useRef<IntersectionObserver | null>(null);
  const observedEl = useRef<HTMLElement | null>(null);
  const observerRef = useCallback(
    (el: HTMLElement | null) => {
      observedEl.current = el;
      if (observer.current) observer.current.disconnect();
      if (!el) return;
      observer.current = new IntersectionObserver(
        (entries) => {
          if (entries[0].isIntersecting) {
            loadMore();
            if (!hasMore) observer.current?.disconnect();
          }
        },
        { root: root ?? null, rootMargin: "400px 0px 200px 0px", threshold: 0 }
      );
      observer.current.observe(el);
    },
    [loadMore, hasMore, root]
  );

  // Reconnect observer when root changes
  useEffect(() => {
    if (observedEl.current) {
      observerRef(observedEl.current);
    }
  }, [observerRef, root]);

  // Cleanup on unmount or re-init
  useEffect(() => {
    return () => observer.current?.disconnect();
  }, []);

  // Fallback: when a custom root is used, also monitor its scroll position to trigger loadMore near bottom
  useEffect(() => {
    const el = root instanceof Element ? (root as HTMLElement) : null;
    if (!el) return;

    let ticking = false;
    const onScroll = () => {
      if (ticking) return;
      ticking = true;
      requestAnimationFrame(() => {
        ticking = false;
        const { scrollTop, clientHeight, scrollHeight } = el;
        const nearBottom =
          scrollTop + clientHeight >= scrollHeight - fallbackScrollDistancePx;
        if (nearBottom && hasMore && !loading && !loadingMore) {
          loadMore();
        }
      });
    };

    el.addEventListener("scroll", onScroll, { passive: true });
    return () => el.removeEventListener("scroll", onScroll);
  }, [root, fallbackScrollDistancePx, hasMore, loading, loadingMore, loadMore]);

  return {
    items,
    loading,
    loadingMore,
    error,
    hasMore,
    loadMore,
    reset,
    retry,
    observerRef,
  };
}
