import { Textarea } from "@/components/ui/textarea";
import { GenerationOptions } from "./GenerationOptions";

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
      <Textarea
        value={value}
        onChange={(e) => onChange(e.target.value)}
        placeholder="例如：一只戴着宇航员头盔的柯基犬，低多边形风格"
        className="min-h-[140px] text-base bg-slate-50 border-slate-200 focus-visible:ring-2 focus-visible:ring-primary/50 focus-visible:border-primary resize-none"
        rows={5}
      />
      <GenerationOptions
        action={action}
        onActionChange={onActionChange}
        format={format}
        onFormatChange={onFormatChange}
      />
    </div>
  );
};
