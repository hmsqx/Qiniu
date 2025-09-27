import React from "react";
import { cn } from "@/lib/utils";

type Variant = "page" | "panel" | "compact";

interface LoadingSkeletonProps {
  lines?: number;
  variant?: Variant;
  className?: string;
}

const LoadingSkeleton: React.FC<LoadingSkeletonProps> = ({
  lines,
  variant = "panel",
  className,
}) => {
  const resolvedLines =
    lines ?? (variant === "page" ? 10 : variant === "compact" ? 2 : 5);

  return (
    <div
      className={cn(
        "animate-pulse text-sm",
        variant === "page" && "p-6 space-y-4",
        variant === "panel" && "p-4 space-y-3",
        variant === "compact" && "p-2 space-y-2",
        className
      )}
    >
      <div
        className={cn(
          "rounded bg-muted/60",
          variant === "compact" ? "h-4 w-24" : "h-5 w-32"
        )}
      />
      {Array.from({ length: resolvedLines }).map((_, i) => (
        <div
          key={i}
          className={cn(
            "rounded bg-muted/40",
            variant === "page" && "h-4 w-full",
            variant === "panel" && "h-4 w-full",
            variant === "compact" && "h-3 w-5/6"
          )}
        />
      ))}
      {variant !== "compact" && (
        <div className="flex gap-2 pt-2">
          <div className="h-8 w-20 rounded bg-muted/50" />
          <div className="h-8 w-14 rounded bg-muted/30" />
        </div>
      )}
    </div>
  );
};

export default LoadingSkeleton;
