import { useEffect, useMemo, useState } from "react";

/**
 * 计算一个用于整个应用缩放的比例，使得“基准宽度（baseWidth）”在任何情况下都能贴合可视区域。
 *
 * 为什么需要它：
 * - Windows 系统缩放（125%/150%）会改变 CSS 像素与物理像素的对应关系，导致 window.innerWidth 变小，
 *   如果只在 w > baseWidth 时才放大，内容可能被裁剪。
 * - 使用 visualViewport 可获得“真正的可视宽度”（考虑系统缩放/浏览器缩放/设备 DPR）。
 *
 * 策略：
 * - 始终按比例缩放：scale = viewportWidth / baseWidth（既可放大也可缩小）。
 * - 监听 window.resize 与 visualViewport.resize，确保系统缩放或浏览器缩放时即时更新。
 */
export type ViewportScaleOptions = {
  baseWidth?: number;
  minScale?: number; // 最小缩放，默认不限制
  maxScale?: number; // 最大缩放，默认不限制
};

const EPS = 1e-4;

function clamp(n: number, min?: number, max?: number) {
  if (typeof min === "number") n = Math.max(min, n);
  if (typeof max === "number") n = Math.min(max, n);
  return n;
}

export function useViewportScale(
  baseWidthOrOptions: number | ViewportScaleOptions = 1920
) {
  const { baseWidth, minScale, maxScale } = useMemo(() => {
    if (typeof baseWidthOrOptions === "number") {
      return {
        baseWidth: baseWidthOrOptions,
        minScale: undefined,
        maxScale: undefined,
      };
    }
    const { baseWidth = 1920, minScale, maxScale } = baseWidthOrOptions ?? {};
    return { baseWidth, minScale, maxScale };
  }, [baseWidthOrOptions]);
  const [scale, setScale] = useState(1);

  useEffect(() => {
    if (typeof window === "undefined") return;

    let frame = 0;

    const compute = () => {
      // 优先使用 visualViewport 的宽度，它能反映系统缩放/浏览器缩放。
      const vv = (window as any).visualViewport as VisualViewport | undefined;
      const viewportWidth =
        vv?.width ?? window.innerWidth ?? document.documentElement.clientWidth;
      // 允许缩小与放大：保证 baseWidth 内容在视觉上总是铺满宽度
      const raw = viewportWidth / baseWidth;
      const next = clamp(raw, minScale, maxScale);
      setScale((prev) => (Math.abs(prev - next) > EPS ? next : prev));
    };

    const onResize = () => {
      cancelAnimationFrame(frame);
      frame = requestAnimationFrame(compute);
    };

    compute();
    window.addEventListener("resize", onResize);
    // 监听 visualViewport 的变化（系统缩放/浏览器缩放/移动端软键盘等）
    const vv = (window as any).visualViewport as VisualViewport | undefined;
    vv?.addEventListener("resize", onResize);
    vv?.addEventListener("scroll", onResize); // 某些设备上缩放会触发 scroll

    return () => {
      cancelAnimationFrame(frame);
      window.removeEventListener("resize", onResize);
      vv?.removeEventListener("resize", onResize);
      vv?.removeEventListener("scroll", onResize);
    };
  }, [baseWidth, minScale, maxScale]);

  return scale;
}
