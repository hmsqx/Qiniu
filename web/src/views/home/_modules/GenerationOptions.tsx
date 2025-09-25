import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from "@/components/ui/select";
import { Info } from "lucide-react";

export const ActionOptions = {
  SubmitHunyuanTo3DJob: "2.5",
} as const;

type Props = {
  action: string;
  onActionChange: (v: string) => void;
  format: string;
  onFormatChange: (v: string) => void;
};

export const GenerationOptions = ({
  action,
  onActionChange,
  format,
  onFormatChange,
}: Props) => {
  // Render label for current action
  const renderActionLabel = (a: string) => {
    if (a === "SubmitHunyuanTo3DJob") return "3D生成-2.5";
    return a;
  };

  return (
    <div className="space-y-4">
      <div className="space-y-2">
        <label className="text-sm font-medium flex items-center text-foreground">
          选择模型
          <Info className="w-3 h-3 ml-1.5 text-muted-foreground" />
          <span className="ml-2 bg-primary text-primary-foreground text-xs px-2 py-0.5 rounded-full font-medium">
            New
          </span>
        </label>
        <Select value={action} onValueChange={onActionChange}>
          <SelectTrigger className="w-full">
            <SelectValue placeholder="选择一个模型">
              {renderActionLabel(action)}
            </SelectValue>
          </SelectTrigger>
          <SelectContent>
            <SelectItem value="SubmitHunyuanTo3DJob">3D生成-2.5</SelectItem>
          </SelectContent>
        </Select>
      </div>

      {/* 生成类型 */}
      <div className="space-y-2">
        <label className="text-sm font-medium text-foreground">生成类型</label>
        <Select value={format} onValueChange={onFormatChange}>
          <SelectTrigger className="w-full">
            <SelectValue placeholder="选择模型格式" />
          </SelectTrigger>
          <SelectContent>
            <SelectItem value="OBJ">OBJ</SelectItem>
            <SelectItem value="GLB">GLB</SelectItem>
            <SelectItem value="STL">STL</SelectItem>
            <SelectItem value="USDZ">USDZ</SelectItem>
            <SelectItem value="FBX">FBX</SelectItem>
            <SelectItem value="MP4">MP4</SelectItem>
          </SelectContent>
        </Select>
      </div>
    </div>
  );
};
