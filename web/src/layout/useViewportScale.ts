import { useEffect, useState } from "react";

/**
 * 根据视口宽度相对于基准宽度计算缩放比例。
 * 当视口宽度小于等于基准宽度时，返回 1（不缩放）。
 */
export function useViewportScale(baseWidth = 1920) {
  const [scale, setScale] = useState(1);

  useEffect(() => {
    if (typeof window === "undefined") return;

    let frame = 0;
    const compute = () => {
      const w = window.innerWidth || document.documentElement.clientWidth;
      const next = w > baseWidth ? w / baseWidth : 1;
      setScale((prev) => (Math.abs(prev - next) > 0.0001 ? next : prev));
    };

    const onResize = () => {
      // 使用 requestAnimationFrame 合并快速的 resize 事件
      cancelAnimationFrame(frame);
      frame = requestAnimationFrame(compute);
    };

    compute();
    window.addEventListener("resize", onResize);
    return () => {
      cancelAnimationFrame(frame);
      window.removeEventListener("resize", onResize);
    };
  }, [baseWidth]);

  return scale;
}
