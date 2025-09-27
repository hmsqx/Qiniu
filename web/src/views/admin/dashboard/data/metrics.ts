// 更语义化的 key，避免使用与含义不符的旧占位符(revenue/customers/...)
export type MetricKey =
  | "averageDownloadRate" // 模型平均下载率
  | "newUsers" // 新增用户
  | "activeAccounts" // 活跃账户
  | "modelLikes"; // 模型点赞情况（可表示点赞率/点赞数，后续可再细化）

export interface MetricItem {
  key: MetricKey;
  title: string;
  value: string;
  trend?: string;
  up?: boolean;
  desc?: string;
}

export const metrics: MetricItem[] = [
  {
    key: "averageDownloadRate",
    title: "模型平均下载率",
    value: "78.9%", // TODO: 后续接入实际指标(例如: 下载数/曝光数)
    trend: "+12.5%",
    up: true,
    desc: "最近 6 个月访客趋势向上",
  },
  {
    key: "newUsers",
    title: "新增用户",
    value: "1,234", // TODO: 当期新增用户数
    trend: "-20%",
    up: false,
    desc: "本期拉新下滑，需要关注获客",
  },
  {
    key: "activeAccounts",
    title: "活跃账户",
    value: "45,678", // TODO: 活跃定义可为 7/30 日活去重
    trend: "+12.5%",
    up: true,
    desc: "用户留存表现强劲",
  },
  {
    key: "modelLikes",
    title: "模型点赞情况",
    value: "4.5%", // TODO: 可改为点赞率或新增点赞数
    trend: "+4.5%",
    up: true,
    desc: "整体增长符合预期",
  },
];
