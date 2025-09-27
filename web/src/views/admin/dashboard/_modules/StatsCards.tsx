import React from "react";
import { Card, CardHeader, CardTitle, CardContent } from "@/components/ui/card";
import { ArrowUpRight, ArrowDownRight } from "lucide-react";
import { useAdminOverview } from "../hooks/useAdminOverview";
import LoadingSkeleton from "@/components/LoadingSkeleton";

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
  const {
    loading,
    totalUsers,
    totalModels,
    downloadedModels,
    likedModels,
    downloadRateText,
    likeRateText,
    userGrowthRateText,
    yesterdayNewUsers,
  } = useAdminOverview();

  if (loading) {
    return (
      <div className="grid gap-4 md:grid-cols-2 xl:grid-cols-4">
        <Card className="panel">
          <CardContent>
            <LoadingSkeleton variant="panel" />
          </CardContent>
        </Card>
        <Card className="panel">
          <CardContent>
            <LoadingSkeleton variant="panel" />
          </CardContent>
        </Card>
        <Card className="panel">
          <CardContent>
            <LoadingSkeleton variant="panel" />
          </CardContent>
        </Card>
        <Card className="panel">
          <CardContent>
            <LoadingSkeleton variant="panel" />
          </CardContent>
        </Card>
      </div>
    );
  }

  return (
    <div className="grid gap-4 md:grid-cols-2 xl:grid-cols-4">
      {/* Overview totals */}
      <StatCard
        title="用户总数"
        value={totalUsers?.toLocaleString?.() ?? "-"}
        trend={userGrowthRateText}
        up={
          (userGrowthRateText && !userGrowthRateText.startsWith("-")) ||
          undefined
        }
        desc={
          typeof yesterdayNewUsers === "number"
            ? `昨日新增 ${yesterdayNewUsers.toLocaleString()}`
            : undefined
        }
      />
      <StatCard
        title="模型总数"
        value={totalModels?.toLocaleString?.() ?? "-"}
        desc={
          typeof downloadedModels === "number"
            ? `累计被下载 ${downloadedModels.toLocaleString()} 次`
            : undefined
        }
      />
      <StatCard
        title="下载率"
        value={downloadRateText ?? "-"}
        desc={
          typeof downloadedModels === "number"
            ? `被下载模型 ${downloadedModels.toLocaleString()} 个`
            : undefined
        }
      />
      <StatCard
        title="点赞率"
        value={likeRateText ?? "-"}
        desc={
          typeof likedModels === "number"
            ? `被点赞模型 ${likedModels.toLocaleString()} 个`
            : undefined
        }
      />
    </div>
  );
};

export default StatsCards;
