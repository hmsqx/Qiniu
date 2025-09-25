import React from "react";
import { Card, CardHeader, CardTitle, CardContent } from "@/components/ui/card";
import { ArrowUpRight, ArrowDownRight } from "lucide-react";
import { useDashboardMetrics } from "../hooks/useDashboardData";

interface StatCardProps {
  title: string;
  value: string;
  trend?: string;
  up?: boolean;
  desc?: string;
}

const StatCard: React.FC<StatCardProps> = ({
  title,
  value,
  trend,
  up,
  desc,
}) => {
  return (
    <Card className="panel">
      <CardHeader className="pb-2">
        <CardTitle className="text-sm font-medium flex items-center gap-2">
          {title}
          {trend && (
            <span
              className={`inline-flex items-center rounded-full border px-1.5 py-0.5 text-[10px] font-medium ${
                up
                  ? "text-green-600 border-green-600/40"
                  : "text-red-600 border-red-600/40"
              }`}
            >
              {up ? (
                <ArrowUpRight className="h-3 w-3" />
              ) : (
                <ArrowDownRight className="h-3 w-3" />
              )}{" "}
              {trend}
            </span>
          )}
        </CardTitle>
      </CardHeader>
      <CardContent className="space-y-1">
        <div className="text-2xl font-semibold tracking-tight">{value}</div>
        {desc && (
          <p className="text-xs text-muted-foreground leading-snug line-clamp-2">
            {desc}
          </p>
        )}
      </CardContent>
    </Card>
  );
};

const StatsCards: React.FC = () => {
  const { metrics } = useDashboardMetrics();
  return (
    <div className="grid gap-4 md:grid-cols-2 xl:grid-cols-4">
      {metrics.map((m) => (
        <StatCard
          key={m.key}
          title={m.title}
          value={m.value}
          trend={m.trend}
          up={m.up}
          desc={m.desc}
        />
      ))}
    </div>
  );
};

export default StatsCards;
