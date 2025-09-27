import { useMemo } from "react";
import type { CSSProperties, PropsWithChildren } from "react";
import { useViewportScale } from "./useViewportScale";

type Props = PropsWithChildren<{
  baseWidth?: number; // 默认为 1920
  className?: string;
  style?: CSSProperties;
  minScale?: number;
  maxScale?: number;
  transitionMs?: number; // 观感过渡时长，默认 120ms
}>;
/**
 * 包裹应用，当视口宽度大于 baseWidth 时按比例缩放整个应用。
 * 使用 transform-origin: top left 保持布局可预测性。
 *
 * 结构说明：
 * - outer（外层）：填满视口（w-full h-full），并使用 overflow-hidden 避免额外滚动条。
 * - inner（内层）：固定基准宽度（baseWidth），高度使用 calc(100vh / scale)，
 *   使缩放后的高度与视口高度一致。
 */
export const ScaledContainer = ({
  baseWidth = 1920,
  className,
  style,
  children,
  minScale,
  maxScale,
  transitionMs = 120,
}: Props) => {
  const scale = useViewportScale({ baseWidth, minScale, maxScale });

  // 根据 scale 计算动态尺寸，保证缩放后的内容适配视口。
  const innerStyle = useMemo<CSSProperties>(() => {
    const s = scale;
    const transition = `${transitionMs}ms ease-out`;
    return {
      width: baseWidth,
      // 让缩放后的高度贴合视口高度：当 s<1（缩小）时，实际高度需要放大到 1/s
      height: `calc(100vh / ${s})`,
      transform: `scale(${s})`,
      transformOrigin: "top left",
      transition: `transform ${transition}, height ${transition}`,
    };
  }, [scale, baseWidth]);

  return (
    <div
      className={"w-full h-full overflow-hidden " + (className ?? "")}
      style={{ height: "100vh", width: "100vw", ...style }}
    >
      <div style={innerStyle}>{children}</div>
    </div>
  );
};

export default ScaledContainer;
