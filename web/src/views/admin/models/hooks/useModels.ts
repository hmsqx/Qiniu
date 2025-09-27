import { useCallback, useEffect, useState } from "react";
import { listAdminModels } from "@/api/models";
import type { ListAdminModelsResponse } from "@/api/models";

export interface UseModelsFilters {
  minLike?: number;
  maxLike?: number;
  minDownload?: number;
  maxDownload?: number;
}

export function useModels() {
  const [data, setData] = useState<ListAdminModelsResponse | null>(null);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<any>(null);

  const [page, setPage] = useState(1);
  const [pageSize] = useState(5);

  const [minLike, setMinLike] = useState<number | undefined>(undefined);
  const [maxLike, setMaxLike] = useState<number | undefined>(undefined);
  const [minDownload, setMinDownload] = useState<number | undefined>(undefined);
  const [maxDownload, setMaxDownload] = useState<number | undefined>(undefined);

  const fetchData = useCallback(async () => {
    setLoading(true);
    setError(null);
    try {
      const resp = await listAdminModels({
        page,
        pageSize,
        minLike,
        maxLike,
        minDownload,
        maxDownload,
      });
      setData(resp);
    } catch (e) {
      setError(e);
    } finally {
      setLoading(false);
    }
  }, [page, pageSize, minLike, maxLike, minDownload, maxDownload]);

  useEffect(() => {
    fetchData();
  }, [fetchData]);

  return {
    data,
    list: data?.list || [],
    total: data?.total || 0,
    page,
    pageSize,
    minLike,
    setMinLike,
    maxLike,
    setMaxLike,
    minDownload,
    setMinDownload,
    maxDownload,
    setMaxDownload,
    loading,
    error,
    refresh: fetchData,
    nextPage: () => setPage((p) => p + 1),
    prevPage: () => setPage((p) => Math.max(1, p - 1)),
    setPage,
  };
}
