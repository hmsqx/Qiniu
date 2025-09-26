import React, { useState } from "react";
import { cn } from "@/lib/utils";

interface FloatingImageZoomProps {
  src: string;
  className?: string;
  scale?: number;
}

export function FloatingImageZoom({
  src,
  className = "",
  scale = 1.08,
}: FloatingImageZoomProps) {
  const [hover, setHover] = useState(false);
  const [origin, setOrigin] = useState({ x: 50, y: 50 });

  const onMove = (e: React.MouseEvent<HTMLDivElement>) => {
    const rect = (e.currentTarget as HTMLDivElement).getBoundingClientRect();
    const x = ((e.clientX - rect.left) / rect.width) * 100;
    const y = ((e.clientY - rect.top) / rect.height) * 100;
    setOrigin({ x, y });
  };

  const onEnter = () => setHover(true);
  const onLeave = () => {
    setHover(false);
    setOrigin({ x: 50, y: 50 });
  };

  return (
    <div
      className={cn(
        "relative overflow-hidden rounded-[28px] bg-white shadow-sm ring-1 ring-slate-200/70",
        className
      )}
      onMouseEnter={onEnter}
      onMouseLeave={onLeave}
      onMouseMove={onMove}
    >
      <div className="pointer-events-none absolute -inset-8 rounded-[40px] bg-gradient-to-br from-emerald-100/60 via-sky-100/50 to-transparent blur-2xl" />
      <img
        src={src}
        alt="hero"
        style={{
          transformOrigin: `${origin.x}% ${origin.y}%`,
          transform: `scale(${hover ? scale : 1})`,
        }}
        className="relative z-10 h-full w-full rounded-[22px] object-cover transition-transform duration-200 ease-out will-change-transform"
      />
      <div className="pointer-events-none absolute right-8 top-6 z-10 h-24 w-24 rounded-full bg-white/20 blur-3xl" />
    </div>
  );
}
