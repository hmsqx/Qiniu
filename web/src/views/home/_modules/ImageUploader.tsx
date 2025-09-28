import { useCallback, useEffect, useState } from "react";
import { useDropzone } from "react-dropzone";
import { UploadCloud } from "lucide-react";
import { cn } from "@/lib/utils";

type Props = {
  onChange?: (base64: string | null) => void;
  /** 注册打开文件选择框的方法 */
  onRegisterOpen?: (open: () => void) => void;
  disabled?: boolean; // 禁用交互（润色/优化进行中）
};

export const ImageUploader = ({
  onChange,
  onRegisterOpen,
  disabled,
}: Props) => {
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
    disabled: !!disabled,
  });

  useEffect(() => {
    onRegisterOpen?.(open);
  }, [open, onRegisterOpen]);

  return (
    <div>
      <div
        {...getRootProps()}
        className={cn(
          "relative flex flex-col items-center justify-center w-full h-44",
          "border border-dashed rounded-2xl cursor-pointer",
          "bg-muted/60 hover:bg-muted/70 transition-colors duration-200",
          "shadow-sm hover:shadow-md ring-1 ring-border/50",
          "transform transition-transform group",
          disabled && "opacity-60 cursor-not-allowed pointer-events-none",
          isDragActive || isDragOver
            ? "border-primary/60 bg-primary/[0.08] ring-primary/40"
            : "border-border/70 hover:border-input"
        )}
      >
        <input {...getInputProps()} />

        <div
          className={cn(
            "mb-2.5 p-2 rounded-full transition-all duration-200",
            "bg-background/80 backdrop-blur-sm border group-hover:border-input",
            isDragActive || isDragOver
              ? "border-primary/40 bg-primary/10"
              : "border-border/60"
          )}
        >
          <UploadCloud
            className={cn(
              "h-6 w-6 transition-colors duration-200",
              isDragActive || isDragOver
                ? "text-primary"
                : "text-muted-foreground/80"
            )}
          />
        </div>

        {/* 文字内容 */}
        <div className="text-center space-y-1.5">
          <p
            className={cn(
              "font-medium text-sm transition-colors duration-200",
              isDragActive || isDragOver ? "text-primary" : "text-foreground"
            )}
          >
            {isDragActive ? "释放文件上传" : "拖拽或点击上传"}
          </p>

          <div className="text-xs text-muted-foreground/80 space-y-0.5">
            <p>支持 PNG / JPG / JPEG / WEBP 格式</p>
            <p>文件大小不超过 8MB（建议不超过6MB）</p>
            <p>单边分辨率要求：最小 128，最大 5000</p>
          </div>
        </div>
      </div>

      {error ? <p className="mt-2 text-xs text-red-600/90">{error}</p> : null}
    </div>
  );
};
