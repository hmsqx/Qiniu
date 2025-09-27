import { useCallback, useEffect, useMemo, useState } from "react";
import {
  queryJobsByPage,
  type JobItem,
  type QueryJobsResult,
} from "@/api/mode3D";

export interface UseWorkspaceJobsOptions {
  initialPage?: number;
  pageSize?: number;
}

export function useWorkspaceJobs(
  userId: string,
  options: UseWorkspaceJobsOptions = {}
) {
  const { initialPage = 1, pageSize = 10 } = options;

  const [pageNum, setPageNum] = useState<number>(initialPage);
  const [total, setTotal] = useState<number>(0);
  const [list, setList] = useState<JobItem[]>([]);
  const [loading, setLoading] = useState<boolean>(false);
  const [error, setError] = useState<string | null>(null);

  const totalPages = useMemo(
    () => Math.max(1, Math.ceil(total / Math.max(1, pageSize))),
    [total, pageSize]
  );

  const fetchPage = useCallback(
    async (page: number) => {
      if (!userId) return;
      setLoading(true);
      setError(null);
      try {
        const res = await queryJobsByPage({ pageNum: page, pageSize, userId });
        const { list: newList, total: newTotal } = res as QueryJobsResult;
        setList(newList || []);
        setTotal(newTotal || 0);
      } catch (e: any) {
        setError(e?.message || "加载失败");
        setList([]);
      } finally {
        setLoading(false);
      }
    },
    [userId, pageSize]
  );

  const refresh = useCallback(() => {
    setPageNum(1);
    fetchPage(1);
  }, [fetchPage]);

  const nextPage = useCallback(() => {
    setPageNum((prev) => {
      const next = Math.min(totalPages, prev + 1);
      if (next !== prev) fetchPage(next);
      return next;
    });
  }, [fetchPage, totalPages]);

  const prevPage = useCallback(() => {
    setPageNum((prev) => {
      const next = Math.max(1, prev - 1);
      if (next !== prev) fetchPage(next);
      return next;
    });
  }, [fetchPage]);

  const goToPage = useCallback(
    (page: number) => {
      const target = Math.min(totalPages, Math.max(1, page));
      setPageNum(target);
      fetchPage(target);
    },
    [fetchPage, totalPages]
  );

  useEffect(() => {
    if (!userId) return;
    setPageNum(1);
    fetchPage(1);
  }, [userId, pageSize, fetchPage]);

  const isEmpty = !loading && !error && list.length === 0;

  return {
    list,
    total,
    pageNum,
    pageSize,
    totalPages,
    loading,
    error,
    isEmpty,
    refresh,
    nextPage,
    prevPage,
    goToPage,
    fetchPage,
    setPageNum,
  } as const;
}
