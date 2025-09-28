import { Sparkles } from "lucide-react";
import { Badge } from "@/components/ui/badge";
import type { ReactNode } from "react";

export type TokenBadgeProps = {
  tokenCount?: number;
  label?: string;
  formatValue?: (value: number | undefined) => ReactNode;
};

export const TokenBadge = ({
  tokenCount,
  label = "次数",
  formatValue,
}: TokenBadgeProps) => {
  const display =
    typeof formatValue === "function"
      ? formatValue(tokenCount)
      : tokenCount !== undefined
      ? tokenCount
      : "—";

  return (
    <Badge
      variant="secondary"
      className="flex items-center gap-2 py-1.5 px-3 rounded-lg"
    >
      <Sparkles className="h-4 w-4 text-primary" />
      <span className="hidden md:inline text-muted-foreground">{label}：</span>
      <span className="font-semibold text-foreground">{display}</span>
    </Badge>
  );
};

export default TokenBadge;
