import React from "react";
import { Badge } from "@/components/ui/badge";
import { Sparkles } from "lucide-react";

interface SectionHeaderProps {
  title: string;
  icon?: React.ReactNode;
}

export function SectionHeader({ title, icon }: SectionHeaderProps) {
  return (
    <div className="mb-6 flex flex-wrap items-center justify-between gap-4">
      <div className="flex items-center">
        {icon ?? <Sparkles className="mr-2 h-5 w-5 text-violet-500" />}
        <h2 className="text-2xl font-semibold tracking-tight text-slate-200">
          {title}
        </h2>
      </div>
      <div className="flex flex-wrap items-center gap-2">
        <Badge variant="secondary">全部</Badge>
        <Badge variant="outline">UI</Badge>
        <Badge variant="outline">插画</Badge>
        <Badge variant="outline">动效</Badge>
        <Badge variant="outline">配色</Badge>
      </div>
    </div>
  );
}
