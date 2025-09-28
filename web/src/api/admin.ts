import { get } from "@/utils/request";
import type { ApiEnvelope } from "./statistics";

// 概览数据：字段以服务端为准，这里定义常见的可选项并允许扩展
export interface AdminOverview {
  ok?: boolean;
  totalUsers?: number;
  totalModels?: number;
  downloadedModels?: number;
  downloadRate?: number; // 0..1
  likedModels?: number;
  likeRate?: number; // 0..1
  userGrowthRate?: number; // 0..1（昨日相对前日增长率）
  yesterdayNewUsers?: number;
  dayBeforeNewUsers?: number;
  // 可选：每日访问量序列
  dailyViews?: Array<{
    date: string; // e.g. "2025-09-15"
    views: number;
  }>;
}

// 管理员概览接口
// GET /api/admin/overview
// 说明：request.ts 会自动从 localStorage 读取 Session-Token 并注入到请求头
export async function getAdminOverview(params?: {
  days?: number;
}): Promise<ApiEnvelope<AdminOverview> | AdminOverview> {
  const qs = params?.days ? `?days=${encodeURIComponent(params.days)}` : "";
  return get<AdminOverview | ApiEnvelope<AdminOverview>>(
    `/api/admin/overview${qs}`
  );
}

export default {
  getAdminOverview,
};
