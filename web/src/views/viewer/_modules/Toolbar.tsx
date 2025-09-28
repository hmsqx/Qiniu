import { Download, Loader2 } from "lucide-react";
import React from "react";
import { Button } from "@/components/ui/button";
import { Label } from "@/components/ui/label";
import { Input } from "@/components/ui/input";

type Props = {
  format: string;
  width: number;
  height: number;
  downloading: boolean;
  onDownload: (e: React.MouseEvent) => void;
  onScreenshot: () => void;
  onToggleRotate: () => void;
  onReset: () => void;
  stlColor: string;
  onStlColor: (color: string) => void;
};

const Toolbar: React.FC<Props> = ({
  format,
  width,
  height,
  downloading,
  onDownload,
  onScreenshot,
  onToggleRotate,
  onReset,
  stlColor,
  onStlColor,
}) => {
  return (
    <div className="absolute right-4 top-2 text-slate-400 text-xs font-mono flex items-center gap-3">
      <span>
        {format.toUpperCase()} | {Math.round(width)}x{Math.round(height)}
      </span>
      <div className="flex items-center gap-2 bg-black/30 px-2 py-1 rounded">
        <Button
          onClick={onDownload}
          disabled={downloading}
          variant="ghost"
          size="sm"
          className="h-7 px-2 py-1 text-emerald-300 hover:text-emerald-200 disabled:opacity-60"
          title="下载模型"
        >
          {downloading ? (
            <Loader2 className="w-3.5 h-3.5 animate-spin" />
          ) : (
            <>
              <Download className="w-3.5 h-3.5" />
              <span className="ml-1">下载</span>
            </>
          )}
        </Button>
        <Button
          onClick={onScreenshot}
          variant="ghost"
          size="sm"
          className="h-7 px-2 py-1 text-sky-300 hover:text-sky-200"
          title="截图"
        >
          截图
        </Button>
        <Button
          onClick={onToggleRotate}
          variant="ghost"
          size="sm"
          className="h-7 px-2 py-1 text-orange-300 hover:text-orange-200"
          title="旋转"
        >
          旋转
        </Button>
        <Button
          onClick={onReset}
          variant="ghost"
          size="sm"
          className="h-7 px-2 py-1 text-pink-300 hover:text-pink-200"
          title="重置"
        >
          重置
        </Button>
        {format === "stl" && (
          <div className="flex items-center gap-1 text-slate-300">
            <Label className="text-slate-300" title="模型颜色">
              颜色
            </Label>
            <Input
              type="color"
              value={stlColor}
              onChange={(e) => onStlColor(e.target.value)}
              className="w-5 h-5 p-0 bg-transparent border-0"
            />
          </div>
        )}
      </div>
    </div>
  );
};

export default Toolbar;
