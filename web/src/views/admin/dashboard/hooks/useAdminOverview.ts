import { useCallback, useEffect, useMemo, useState } from "react";
import { getAdminOverview } from "@/api/admin";
import type { AdminOverview } from "@/api/admin";
import type { ApiEnvelope } from "@/api/statistics";

type Overview = AdminOverview;

function unwrapEnvelope(resp: Overview | ApiEnvelope<Overview>): Overview {
  if (
    resp &&
    typeof resp === "object" &&
    "data" in resp &&
    (resp as any).data
  ) {
    return (resp as ApiEnvelope<Overview>).data as Overview;
  }
  return resp as Overview;
}

// 统一读取常见别名，尽量兼容后端字段差异
function pickNumber(
  obj: Record<string, any>,
  keys: string[],
  defaultValue: number | undefined = undefined
): number | undefined {
  for (const k of keys) {
    const v = obj?.[k];
    if (typeof v === "number") return v;
    // 字符串数字也尽量转换
    if (typeof v === "string" && v.trim() !== "" && !isNaN(Number(v))) {
      return Number(v);
    }
  }
  return defaultValue;
}

export interface UseAdminOverviewResult {
  overview: Overview | null;
  // 原始数值
  totalUsers?: number;
  totalModels?: number;
  downloadedModels?: number;
  likedModels?: number;
  // 百分比（0..1 转化为 0..100% 的字符串）
  downloadRateText?: string;
  likeRateText?: string;
  userGrowthRateText?: string;
  // 其他
  yesterdayNewUsers?: number;
  dayBeforeNewUsers?: number;
  loading: boolean;
  error: any;
  refresh: () => Promise<void>;
}

export function useAdminOverview(): UseAdminOverviewResult {
  const [overview, setOverview] = useState<Overview | null>(null);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<any>(null);

  const fetchData = useCallback(async () => {
    setLoading(true);
    setError(null);
    try {
      const resp = await getAdminOverview();
      const data = unwrapEnvelope(resp);
      setOverview(data || {});
    } catch (e) {
      setError(e);
    } finally {
      setLoading(false);
    }
  }, []);

  useEffect(() => {
    fetchData();
  }, [fetchData]);

  const mapped = useMemo(() => {
    const src = overview || ({} as Overview);
    const totalUsers = pickNumber(src as any, ["totalUsers"]);
    const totalModels = pickNumber(src as any, ["totalModels"]);
    const downloadedModels = pickNumber(src as any, ["downloadedModels"]);
    const likedModels = pickNumber(src as any, ["likedModels"]);
    const downloadRate = pickNumber(src as any, ["downloadRate"]);
    const likeRate = pickNumber(src as any, ["likeRate"]);
    const userGrowthRate = pickNumber(src as any, ["userGrowthRate"]);
    const yesterdayNewUsers = pickNumber(src as any, ["yesterdayNewUsers"]);
    const dayBeforeNewUsers = pickNumber(src as any, ["dayBeforeNewUsers"]);

    const pct = (v?: number) =>
      typeof v === "number" ? `${(v * 100).toFixed(2)}%` : undefined;

    return {
      totalUsers,
      totalModels,
      downloadedModels,
      likedModels,
      downloadRateText: pct(downloadRate),
      likeRateText: pct(likeRate),
      userGrowthRateText: pct(userGrowthRate),
      yesterdayNewUsers,
      dayBeforeNewUsers,
    };
  }, [overview]);

  return { overview, ...mapped, loading, error, refresh: fetchData };
}
