import { useState, useCallback } from "react";
import { assessAndEnhanceImage } from "@/api/image";
import { useToast } from "@/components/ui/use-toast";

export type EnhanceOptions = {
  threshold?: number;
  max_iterations?: number;
};

/**
 * 使用一个简洁的命令式 optimize() 调用对图片增强 API 进行封装，
 * 同时提供加载状态标记，并通过 toast 显示进度与结果。
 */
export function useImageEnhancer() {
  const [optimizing, setOptimizing] = useState(false);
  const { toast, updateToast } = useToast();

  const optimize = useCallback(
    async (base64: string, options?: EnhanceOptions) => {
      if (!base64 || optimizing) return { updated: false as const };
      setOptimizing(true);
      const id = toast({ title: "图片优化中...", variant: "loading" });
      try {
        const res = await assessAndEnhanceImage({
          image: base64,
          threshold: options?.threshold ?? 0.9,
          max_iterations: options?.max_iterations ?? 3,
        });
        if (!res?.success) throw new Error(res?.message || "优化失败");
        if (res.needs_enhancement && res.enhanced_image) {
          const enhanced = res.enhanced_image.replace(
            /^data:\s*image\/[a-zA-Z0-9.+-]+;base64,/,
            ""
          );
          updateToast(id, { title: "优化完成", variant: "success" });
          return { updated: true as const, base64: enhanced };
        }
        updateToast(id, { title: "无需优化", variant: "success" });
        return { updated: false as const };
      } catch (err: any) {
        updateToast(id, {
          title: "优化失败",
          description: String(err?.message || err),
          variant: "error",
        });
        return { updated: false as const };
      } finally {
        setOptimizing(false);
      }
    },
    [optimizing, toast, updateToast]
  );

  return { optimizing, optimize } as const;
}

export default useImageEnhancer;
