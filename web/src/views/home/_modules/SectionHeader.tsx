import React from "react";
import { Sparkles } from "lucide-react";

interface SectionHeaderProps {
  title: string;
  icon?: React.ReactNode;
}

export function SectionHeader({ title, icon }: SectionHeaderProps) {
  return (
    <div className=" flex flex-wrap items-center justify-between">
      <div className="flex items-center">
        {icon ?? <Sparkles className="mr-2 h-5 w-5 text-violet-500" />}
        <h2 className="text-2xl font-semibold tracking-tight text-slate-200">
          {title}
        </h2>
      </div>
    </div>
  );
}
