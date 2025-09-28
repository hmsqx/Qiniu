import React from "react";
import { cn } from "@/lib/utils";
import { useVisitorsSeries } from "../hooks/useDashboardData";
import {
  AreaChart,
  Area,
  CartesianGrid,
  XAxis,
  YAxis,
  Tooltip,
} from "recharts";
import { ChartContainer, ChartTooltipContent } from "@/components/ui/chart";

interface VisitorsChartProps {
  range: string;
  className?: string;
}

const chartConfig = {
  visitors: {
    label: "每日使用次数",
    color: "var(--color-chart-1)",
  },
} as const;

const VisitorsChart: React.FC<VisitorsChartProps> = ({ range, className }) => {
  const data = useVisitorsSeries(range).map(
    (d: { label: string; value: number }) => ({
      label: d.label,
      visitors: d.value,
    })
  );

  return (
    <ChartContainer
      config={chartConfig}
      className={cn("h-60 w-full", className)}
      aspect={null}
    >
      <AreaChart
        data={data}
        margin={{ left: 24, right: 12, top: 8, bottom: 20 }}
      >
        <defs>
          <linearGradient id="visitorsGradient" x1="0" x2="0" y1="0" y2="1">
            <stop
              offset="5%"
              stopColor="var(--color-visitors)"
              stopOpacity={0.35}
            />
            <stop
              offset="95%"
              stopColor="var(--color-visitors)"
              stopOpacity={0.02}
            />
          </linearGradient>
        </defs>
        <CartesianGrid
          strokeDasharray="3 3"
          vertical={false}
          strokeOpacity={0.25}
        />
        <XAxis
          dataKey="label"
          tickLine={false}
          axisLine={false}
          interval="preserveStartEnd"
          tick={{ fill: "hsl(var(--muted-foreground))", fontSize: 11 }}
        />
        <YAxis
          width={40}
          tickLine={false}
          axisLine={false}
          tick={{ fill: "hsl(var(--muted-foreground))", fontSize: 11 }}
          allowDecimals={false}
        />
        <Tooltip
          cursor={{ stroke: "var(--border)", strokeWidth: 1 }}
          content={<ChartTooltipContent labelKey="label" />}
        />
        <Area
          dataKey="visitors"
          type="monotone"
          stroke="var(--color-visitors)"
          fill="url(#visitorsGradient)"
          strokeWidth={2}
          isAnimationActive={false}
          activeDot={{ r: 4, strokeWidth: 0 }}
        />
      </AreaChart>
    </ChartContainer>
  );
};

export default VisitorsChart;
