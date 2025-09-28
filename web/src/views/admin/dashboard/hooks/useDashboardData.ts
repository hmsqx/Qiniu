import { useEffect, useMemo, useState } from "react";
import { metrics } from "../data/metrics";
import type { MetricItem } from "../data/metrics";
import { generateVisitors } from "../data/visitors";
import type { VisitorPoint } from "../data/visitors";
import { getAdminOverview, type AdminOverview } from "@/api/admin";
import type { ApiEnvelope } from "@/api/statistics";

export function useDashboardMetrics(): { metrics: MetricItem[] } {
  return { metrics };
}

export function useVisitorsSeries(range: string): VisitorPoint[] {
  const [series, setSeries] = useState<VisitorPoint[]>([]);

  // 统一向后端请求 14 天数据；7 天仅在前端取后 7 天展示
  const requestDays = 14;

  // 生成默认访客数据（14 天），后续根据 range 进行截取
  const fallback14 = useMemo(() => generateVisitors(requestDays), []);

  useEffect(() => {
    let cancelled = false;

    // 异步获取数据
    async function fetchData() {
      try {
        const resp = await getAdminOverview({ days: requestDays });
        const data: AdminOverview = ((): AdminOverview => {
          const r = resp as any;
          if (r && typeof r === "object" && "data" in r && r.data) {
            return (r as ApiEnvelope<AdminOverview>).data as AdminOverview;
          }
          return r as AdminOverview;
        })();
        // 格式化数据点
        const points: VisitorPoint[] | undefined = data?.dailyViews?.map(
          (d) => ({
            label: formatDateLabel(d.date),
            value: typeof d.views === "number" ? d.views : Number(d.views ?? 0),
          })
        );
        if (!cancelled) {
          if (points && points.length) {
            setSeries(points);
          } else {
            setSeries(fallback14);
          }
        }
      } catch (e) {
        if (!cancelled) setSeries(fallback14);
      }
    }

    fetchData();
    // 清理函数，防止内存泄漏
    return () => {
      cancelled = true;
    };
  }, []);

  // 根据选择范围返回：7 天返回后 7 天；14 天返回全部
  const full14 = series.length ? series : fallback14;
  const result = useMemo(() => {
    if (range === "7d") {
      return full14.slice(-7);
    }
    // 默认 14 天
    return full14;
  }, [full14, range]);

  return result;
}

// 格式化日期标签
function formatDateLabel(dateStr?: string): string {
  if (!dateStr) return "";
  // 期望格式为 YYYY-MM-DD；否则使用本地解析
  const d = new Date(dateStr);
  if (!isNaN(d.getTime())) {
    return `${d.getMonth() + 1}/${d.getDate()}`;
  }
  try {
    const [y, m, d2] = dateStr.split(/[-/]/).map((s) => Number(s));
    if (y && m && d2) return `${m}/${d2}`;
  } catch {}
  return dateStr;
}
