export interface VisitorPoint {
  label: string;
  value: number;
}

// 简单生成模拟数据
export function generateVisitors(days: number): VisitorPoint[] {
  const out: VisitorPoint[] = [];
  const now = new Date();
  for (let i = days - 1; i >= 0; i--) {
    const d = new Date(now);
    d.setDate(now.getDate() - i);
    const base = 50 + Math.sin(i / 3) * 20 + Math.random() * 25;
    out.push({
      label: `${d.getMonth() + 1}/${d.getDate()}`,
      value: Math.round(base),
    });
  }
  return out;
}
