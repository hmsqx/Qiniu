export interface MetricItem {
  key: string;
  title: string;
  value: string;
  trend?: string;
  up?: boolean;
  desc?: string;
}

export const metrics: MetricItem[] = [
  {
    key: "revenue",
    title: "总收入",
    value: "$1,250.00",
    trend: "+12.5%",
    up: true,
    desc: "最近 6 个月访客趋势向上",
  },
  {
    key: "customers",
    title: "新增用户",
    value: "1,234",
    trend: "-20%",
    up: false,
    desc: "本期拉新下滑，需要关注获客",
  },
  {
    key: "accounts",
    title: "活跃账户",
    value: "45,678",
    trend: "+12.5%",
    up: true,
    desc: "用户留存表现强劲",
  },
  {
    key: "growth",
    title: "增长率",
    value: "4.5%",
    trend: "+4.5%",
    up: true,
    desc: "整体增长符合预期",
  },
];
