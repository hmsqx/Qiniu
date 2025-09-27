import { useEffect, useState } from "react";

/**
 * Observe and return the size of a container element.
 */
export default function useContainerSize(
  containerRef: React.RefObject<HTMLElement | null>
) {
  const [size, setSize] = useState({ width: 0, height: 0 });

  useEffect(() => {
    const el = containerRef.current;
    if (!el) return;

    const ro = new ResizeObserver(() => {
      setSize({ width: el.clientWidth, height: el.clientHeight });
    });

    ro.observe(el);
    setSize({ width: el.clientWidth, height: el.clientHeight });

    return () => ro.disconnect();
  }, [containerRef]);

  return size;
}
