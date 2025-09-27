import { Badge } from "@/components/ui/badge";
import { Button } from "@/components/ui/button";
import {
  Tooltip,
  TooltipContent,
  TooltipProvider,
  TooltipTrigger,
} from "@/components/ui/tooltip";
import { Loader2, Trash2, Wand2 } from "lucide-react";
import React from "react";
import useImageBase64Info from "./useImageBase64Info";

type Props = {
  base64: string; // 无 data: 前缀
  onRemove: () => void;
  onReselect?: () => void;
  onOptimize?: () => void;
  optimizing?: boolean;
};

export const ImagePreviewCard: React.FC<Props> = ({
  base64,
  onRemove,
  onReselect,
  onOptimize,
  optimizing,
}) => {
  const { dataUrl, dimensions, readableSize } = useImageBase64Info(base64);

  return (
    <div className="mt-2 rounded-xl border bg-background/70 backdrop-blur-md p-3 shadow-sm">
      <div className="flex items-start gap-4">
        <div className="relative w-36 h-36 md:w-40 md:h-40 rounded-lg overflow-hidden ring-1 ring-white/10 bg-muted/30">
          <img
            src={dataUrl ?? undefined}
            alt="已上传图片预览"
            className="w-full h-full object-contain bg-black/10"
          />
          {dimensions && (
            <div className="absolute left-2 top-2">
              <Badge variant="secondary">
                {dimensions.w}×{dimensions.h}
              </Badge>
            </div>
          )}
          <div className="absolute right-2 top-2">
            <TooltipProvider>
              <Tooltip>
                <TooltipTrigger asChild>
                  <Button
                    type="button"
                    size="icon"
                    variant="ghost"
                    className="h-7 w-7 rounded-full bg-background/70 backdrop-blur border border-white/10 hover:bg-background"
                    onClick={onRemove}
                    aria-label="移除图片"
                  >
                    <Trash2 className="h-3.5 w-3.5" />
                  </Button>
                </TooltipTrigger>
                <TooltipContent>移除图片</TooltipContent>
              </Tooltip>
            </TooltipProvider>
          </div>
        </div>
        <div className="flex-1">
          <p className="text-sm text-muted-foreground">已上传图片（预览）</p>
          <div className="mt-2 flex flex-wrap items-center gap-3 text-xs text-muted-foreground/90">
            {dimensions && (
              <span>
                分辨率：{dimensions.w}×{dimensions.h}
              </span>
            )}
            <span>大小：{readableSize}</span>
          </div>
          <div className="mt-3 flex flex-wrap gap-2 items-center">
            {onReselect && (
              <Button size="sm" variant="outline" onClick={onReselect}>
                重新选择
              </Button>
            )}
            {onOptimize && (
              <TooltipProvider>
                <Tooltip>
                  <TooltipTrigger asChild>
                    <Button
                      size="icon"
                      variant="outline"
                      className="h-8 w-8"
                      onClick={onOptimize}
                      disabled={optimizing}
                    >
                      {optimizing ? (
                        <Loader2 className="h-4 w-4 animate-spin" />
                      ) : (
                        <Wand2 className="h-4 w-4" />
                      )}
                    </Button>
                  </TooltipTrigger>
                  <TooltipContent>
                    {optimizing ? "优化中..." : "图片优化"}
                  </TooltipContent>
                </Tooltip>
              </TooltipProvider>
            )}
          </div>
        </div>
      </div>
    </div>
  );
};

export default ImagePreviewCard;
