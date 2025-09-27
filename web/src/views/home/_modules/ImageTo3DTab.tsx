import { ImageUploader } from "./ImageUploader";
import { GenerationOptions } from "./GenerationOptions";
import { Button } from "@/components/ui/button";
import { Badge } from "@/components/ui/badge";
import { Trash2 } from "lucide-react";
import { useEffect, useMemo, useState, useRef } from "react";

type Props = {
  imageBase64: string | null;
  onChangeImageBase64: (b: string | null) => void;
  action: string;
  onActionChange: (v: string) => void;
  format: string;
  onFormatChange: (v: string) => void;
};

export const ImageTo3DTab = ({
  imageBase64,
  onChangeImageBase64,
  action,
  onActionChange,
  format,
  onFormatChange,
}: Props) => {
  const openFileDialogRef = useRef<null | (() => void)>(null);
  const [imgSize, setImgSize] = useState<{ w: number; h: number } | null>(null);

  useEffect(() => {
    if (!imageBase64) {
      setImgSize(null);
      return;
    }
    const img = new Image();
    img.onload = () =>
      setImgSize({ w: img.naturalWidth, h: img.naturalHeight });
    img.src = `data:image/jpeg;base64,${imageBase64}`;
  }, [imageBase64]);

  const approxBytes = useMemo(
    () => (imageBase64 ? Math.ceil((imageBase64.length * 3) / 4) : 0),
    [imageBase64]
  );

  const readableSize = useMemo(() => {
    if (!approxBytes) return "-";
    if (approxBytes < 1024) return `${approxBytes} B`;
    if (approxBytes < 1024 * 1024)
      return `${(approxBytes / 1024).toFixed(1)} KB`;
    return `${(approxBytes / (1024 * 1024)).toFixed(2)} MB`;
  }, [approxBytes]);

  return (
    <div className="space-y-6">
      <ImageUploader
        onChange={(b) => onChangeImageBase64(b)}
        onRegisterOpen={(open) => (openFileDialogRef.current = open)}
      />

      {imageBase64 ? (
        <div className="mt-2 rounded-xl border bg-background/70 backdrop-blur-md p-3 shadow-sm">
          <div className="flex items-start gap-4">
            <div className="relative w-36 h-36 md:w-40 md:h-40 rounded-lg overflow-hidden ring-1 ring-white/10 bg-muted/30">
              <img
                src={`data:image/jpeg;base64,${imageBase64}`}
                alt="已上传图片预览"
                className="w-full h-full object-contain bg-black/10"
              />
              {imgSize && (
                <div className="absolute left-2 top-2">
                  <Badge variant="secondary">
                    {imgSize.w}×{imgSize.h}
                  </Badge>
                </div>
              )}
            </div>
            <div className="flex-1">
              <p className="text-sm text-muted-foreground">
                已上传图片（预览）
              </p>
              <div className="mt-2 flex flex-wrap items-center gap-3 text-xs text-muted-foreground/90">
                {imgSize && (
                  <span>
                    分辨率：{imgSize.w}×{imgSize.h}
                  </span>
                )}
                <span>大小：{readableSize}</span>
              </div>
              <div className="mt-3 flex flex-wrap gap-2">
                <Button
                  size="sm"
                  variant="destructive"
                  onClick={() => onChangeImageBase64(null)}
                >
                  <Trash2 className="size-4" /> 移除图片
                </Button>
                <Button
                  size="sm"
                  variant="outline"
                  onClick={() => openFileDialogRef.current?.()}
                >
                  重新选择
                </Button>
              </div>
            </div>
          </div>
        </div>
      ) : null}

      <GenerationOptions
        action={action}
        onActionChange={onActionChange}
        format={format}
        onFormatChange={onFormatChange}
      />
    </div>
  );
};
