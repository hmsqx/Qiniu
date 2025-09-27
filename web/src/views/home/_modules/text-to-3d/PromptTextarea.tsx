import { Textarea } from "@/components/ui/textarea";
import { Button } from "@/components/ui/button";
import {
  Tooltip,
  TooltipContent,
  TooltipProvider,
  TooltipTrigger,
} from "@/components/ui/tooltip";
import { Star } from "lucide-react";

type Props = {
  value: string;
  onChange: (v: string) => void;
  onOpenPolish: () => void;
};

/**
 * PromptTextarea
 * 展示与编辑用于生成 3D 的提示词输入框，并提供打开“润色”弹窗的入口。
 */
export function PromptTextarea({ value, onChange, onOpenPolish }: Props) {
  return (
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
              aria-label="润色内容"
              className="absolute left-1.5 bottom-1.5 size-7 p-0 hover:text-primary text-primary/80"
              onClick={onOpenPolish}
            >
              <Star className="size-3.5" fill="currentColor" stroke="none" />
            </Button>
          </TooltipTrigger>
          <TooltipContent side="top" className="px-2 py-1 text-[10px]">
            润色内容
          </TooltipContent>
        </Tooltip>
      </TooltipProvider>
    </div>
  );
}

export default PromptTextarea;
