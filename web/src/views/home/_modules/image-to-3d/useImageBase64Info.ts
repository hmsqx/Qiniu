import { useEffect, useMemo, useState } from "react";

/**
 * 从 base64（无 data: 前缀）图片字符串中派生常用信息。
 * 返回图片的宽高、近似字节数和可读的大小标签。
 */
export function useImageBase64Info(
  base64: string | null,
  opts?: { mime?: string }
) {
  const mime = opts?.mime ?? "image/jpeg";

  const [dimensions, setDimensions] = useState<{
    w: number;
    h: number;
  } | null>(null);

  const dataUrl = useMemo(() => {
    if (!base64) return null;
    // 注意：无法确定原始格式；此处预览默认使用 jpeg。
    return `data:${mime};base64,${base64}`;
  }, [base64, mime]);

  useEffect(() => {
    let cancelled = false;
    if (!dataUrl) {
      setDimensions(null);
      return;
    }
    const img = new Image();
    img.onload = () => {
      if (!cancelled)
        setDimensions({ w: img.naturalWidth, h: img.naturalHeight });
    };
    img.onerror = () => {
      if (!cancelled) setDimensions(null);
    };
    img.src = dataUrl;
    return () => {
      cancelled = true;
    };
  }, [dataUrl]);

  const approxBytes = useMemo(() => {
    return base64 ? Math.ceil((base64.length * 3) / 4) : 0;
  }, [base64]);

  const readableSize = useMemo(() => {
    const bytes = approxBytes;
    if (!bytes) return "-";
    if (bytes < 1024) return `${bytes} B`;
    if (bytes < 1024 * 1024) return `${(bytes / 1024).toFixed(1)} KB`;
    return `${(bytes / (1024 * 1024)).toFixed(2)} MB`;
  }, [approxBytes]);

  return { dimensions, approxBytes, readableSize, dataUrl } as const;
}

export default useImageBase64Info;
