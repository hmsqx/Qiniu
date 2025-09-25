import { Textarea } from "@/components/ui/textarea";
import { GenerationOptions } from "./GenerationOptions";
import { Star } from "lucide-react";
import { Button } from "@/components/ui/button";
import {
  Tooltip,
  TooltipContent,
  TooltipProvider,
  TooltipTrigger,
} from "@/components/ui/tooltip";

type Props = {
  value: string;
  onChange: (v: string) => void;
  action: string;
  onActionChange: (v: string) => void;
  format: string;
  onFormatChange: (v: string) => void;
};

export const TextTo3DTab = ({
  value,
  onChange,
  action,
  onActionChange,
  format,
  onFormatChange,
}: Props) => {
  return (
    <div className="space-y-6">
      <div className="relative">
        <Textarea
          value={value}
          onChange={(e) => onChange(e.target.value)}
          placeholder="例如：一只戴着宇航员头盔的柯基犬，低多边形风格"
          className="min-h-[140px] text-base bg-slate-50 focus-visible:ring-2 focus-visible:ring-primary/50 focus-visible:border-primary resize-none pr-10 pb-10"
          rows={5}
        />
        <TooltipProvider>
          <Tooltip>
            <TooltipTrigger asChild>
              <Button
                type="button"
                variant="ghost"
                size="sm"
                aria-label={value.trim() ? "润色内容" : "随机生成文本"}
                className="absolute left-1.5 bottom-1.5 size-7 p-0 hover:text-primary text-primary/80"
              >
                <Star className="size-3.5" fill="currentColor" stroke="none" />
              </Button>
            </TooltipTrigger>
            <TooltipContent side="top" className="px-2 py-1 text-[10px]">
              {value.trim() ? "润色内容" : "随机生成文本"}
            </TooltipContent>
          </Tooltip>
        </TooltipProvider>
      </div>
      <GenerationOptions
        action={action}
        onActionChange={onActionChange}
        format={format}
        onFormatChange={onFormatChange}
      />
    </div>
  );
};
