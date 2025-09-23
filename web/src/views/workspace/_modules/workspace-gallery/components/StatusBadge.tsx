import { Badge } from "@/components/ui/badge";

export function StatusBadge({ status }: { status?: string }) {
  const s = status || "";
  if (s.includes("完成"))
    return (
      <Badge className="bg-emerald-500/20 text-emerald-300 border-emerald-500/30">
        完成
      </Badge>
    );
  if (s.includes("处理中") || s.includes("进行"))
    return (
      <Badge
        variant="secondary"
        className="bg-amber-500/20 text-amber-300 border-amber-500/30"
      >
        处理中
      </Badge>
    );
  return (
    <Badge variant="outline" className="border-slate-600 text-slate-300">
      排队中
    </Badge>
  );
}
