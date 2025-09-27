import { useMemo } from "react";
import type { CSSProperties, PropsWithChildren } from "react";
import { useViewportScale } from "./useViewportScale";

type Props = PropsWithChildren<{
  baseWidth?: number; // 默认为 1920
  className?: string;
  style?: CSSProperties;
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
}: Props) => {
  const scale = useViewportScale(baseWidth);

  // 根据 scale 计算动态尺寸，保证缩放后的内容适配视口。
  const innerStyle = useMemo<CSSProperties>(() => {
    const s = scale;
    return {
      width: baseWidth,
      height: `calc(100vh / ${s})`,
      transform: s === 1 ? undefined : `scale(${s})`,
      transformOrigin: "top left",
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
