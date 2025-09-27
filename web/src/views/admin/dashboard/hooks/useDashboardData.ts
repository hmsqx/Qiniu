import { useMemo } from "react";
import { metrics } from "../data/metrics";
import type { MetricItem } from "../data/metrics";
import { generateVisitors } from "../data/visitors";
import type { VisitorPoint } from "../data/visitors";

export function useDashboardMetrics(): { metrics: MetricItem[] } {
  // 未来可接入真实 API
  return { metrics };
}

export function useVisitorsSeries(range: string): VisitorPoint[] {
  return useMemo(() => {
    switch (range) {
      case "7d":
        return generateVisitors(7);
      case "14d":
      default:
        return generateVisitors(14);
    }
  }, [range]);
}
