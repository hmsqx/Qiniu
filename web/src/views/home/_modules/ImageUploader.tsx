import { useCallback, useImperativeHandle, useState, forwardRef } from "react";
import type { ForwardedRef } from "react";
import { useDropzone } from "react-dropzone";
import { UploadCloud } from "lucide-react";
import { cn } from "@/lib/utils";

type Props = {
  onChange?: (base64: string | null) => void;
};

export type ImageUploaderHandle = {
  openFileDialog: () => void;
};

export const ImageUploader = forwardRef(
  ({ onChange }: Props, ref: ForwardedRef<ImageUploaderHandle>) => {
    const [isDragOver, setIsDragOver] = useState(false);
    const [error, setError] = useState<string | null>(null);

    const MAX_FILE_SIZE = 8 * 1024 * 1024; // 8MB
    const MIN_SIDE = 128;
    const MAX_SIDE = 5000;
    const ALLOWED_EXT = ["png", "jpg", "jpeg", "webp"];

    const resetError = () => setError(null);

    const validateAndReadFile = async (file: File) => {
      resetError();

      const ext = (file.name.split(".").pop() || "").toLowerCase();
      if (!ALLOWED_EXT.includes(ext)) {
        setError("只支持 PNG / JPG / JPEG / WEBP 格式");
        onChange?.(null);
        return;
      }

      if (file.size > MAX_FILE_SIZE) {
        setError("文件大小不能超过 8MB（建议不超过6MB）");
        onChange?.(null);
        return;
      }

      const dataUrl = await new Promise<string>((resolve, reject) => {
        const reader = new FileReader();
        reader.onerror = () => reject(new Error("读取文件失败"));
        reader.onload = () => resolve(String(reader.result || ""));
        reader.readAsDataURL(file);
      });

      // 检查尺寸
      const img = await new Promise<HTMLImageElement>((resolve, reject) => {
        const i = new Image();
        i.onload = () => resolve(i);
        i.onerror = () => reject(new Error("图片加载失败"));
        i.src = dataUrl;
      });

      const { width, height } = img;
      const minSide = Math.min(width, height);
      if (minSide < MIN_SIDE || Math.max(width, height) > MAX_SIDE) {
        setError(`图片分辨率要求单边不小于 ${MIN_SIDE}，不大于 ${MAX_SIDE}`);
        onChange?.(null);
        return;
      }

      const commaIndex = dataUrl.indexOf(",");
      const base64 = commaIndex >= 0 ? dataUrl.slice(commaIndex + 1) : dataUrl;

      onChange?.(base64);
    };

    const onDrop = useCallback((acceptedFiles: File[]) => {
      setIsDragOver(false); // 重置拖拽状态
      if (!acceptedFiles || acceptedFiles.length === 0) return;
      validateAndReadFile(acceptedFiles[0]).catch((e) => {
        console.error(e);
        setError((e && (e as Error).message) || "处理图片失败");
        onChange?.(null);
      });
    }, []);

    const { getRootProps, getInputProps, isDragActive, open } = useDropzone({
      onDrop,
      accept: { "image/*": [".png", ".jpg", ".jpeg", ".webp"] },
      onDragEnter: () => setIsDragOver(true),
      onDragLeave: () => setIsDragOver(false),
      multiple: false,
    });

    useImperativeHandle(ref, () => ({ openFileDialog: () => open() }), [open]);

    return (
      <div>
        <div
          {...getRootProps()}
          className={cn(
            "relative flex flex-col items-center justify-center w-full h-48",
            "border-2 border-dashed rounded-xl cursor-pointer",
            "bg-muted hover:bg-muted/80 transition-colors duration-200",
            "group",
            isDragActive || isDragOver
              ? "border-primary bg-primary/10"
              : "border-border hover:border-input"
          )}
        >
          <input {...getInputProps()} />

          {/* 上传图标区域 */}
          <div
            className={cn(
              "mb-3 p-3 rounded-full transition-all duration-200",
              "bg-background border group-hover:border-input",
              isDragActive || isDragOver
                ? "border-primary/30 bg-primary/10"
                : "border-border"
            )}
          >
            <UploadCloud
              className={cn(
                "h-8 w-8 transition-colors duration-200",
                isDragActive || isDragOver
                  ? "text-primary"
                  : "text-muted-foreground"
              )}
            />
          </div>

          {/* 文字内容 */}
          <div className="text-center space-y-1">
            <p
              className={cn(
                "font-medium transition-colors duration-200",
                isDragActive || isDragOver ? "text-primary" : "text-foreground"
              )}
            >
              {isDragActive ? "释放文件上传" : "拖拽或点击上传"}
            </p>

            <div className="text-sm text-muted-foreground space-y-0.5">
              <p>支持 PNG / JPG / JPEG / WEBP 格式</p>
              <p>文件大小不超过 8MB（建议不超过6MB）</p>
              <p>单边分辨率要求：最小 128，最大 5000</p>
            </div>
          </div>
        </div>

        {error ? <p className="mt-2 text-sm text-red-600">{error}</p> : null}
      </div>
    );
  }
);
