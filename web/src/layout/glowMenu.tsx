import * as React from "react";
// 1. 从 framer-motion 导入 Variants 类型
import { motion, type Variants } from "framer-motion";

import { cn } from "@/lib/utils";
import type { LucideIcon } from "lucide-react";

import type { HTMLMotionProps } from "framer-motion";
export interface MenuItem {
  icon: LucideIcon | React.FC;
  label: string;
  href: string;
  gradient: string;
  iconColor: string;
}

interface MenuBarProps extends Omit<HTMLMotionProps<"nav">, "children"> {
  items: MenuItem[];
  activeItem?: string;
  onItemClick?: (label: string) => void;
  onItemHover?: (label: string) => void;
}
// 2. 为所有 variants 对象添加显式的 Variants 类型
const itemVariants: Variants = {
  initial: { rotateX: 0, opacity: 1 },
  hover: { rotateX: -90, opacity: 0 },
};

const backVariants: Variants = {
  initial: { rotateX: 90, opacity: 0 },
  hover: { rotateX: 0, opacity: 1 },
};

// 更柔和的发光动画，减少夸张的放大导致的“花”感
const glowVariants: Variants = {
  initial: { opacity: 0, scale: 0.9 },
  hover: {
    opacity: 0.9,
    scale: 1.35,
    transition: {
      opacity: { duration: 0.4, ease: [0.4, 0, 0.2, 1] },
      scale: { duration: 0.45, type: "spring", stiffness: 220, damping: 28 },
    },
  },
};

const navGlowVariants: Variants = {
  initial: { opacity: 0 },
  hover: {
    opacity: 1,
    transition: {
      duration: 0.5,
      ease: [0.4, 0, 0.2, 1],
    },
  },
};

const sharedTransition = {
  type: "spring",
  stiffness: 100,
  damping: 20,
  duration: 0.5,
} as const;

export const MenuBar = React.forwardRef<HTMLDivElement, MenuBarProps>(
  (
    { className, items, activeItem, onItemClick, onItemHover, ...props },
    ref
  ) => {
    return (
      <motion.nav
        ref={ref}
        className={cn(
          // 精简背景，添加一条半透明描边 + 轻微内阴影，保持导航与内容区分
          "p-1.5 bg-background/60 backdrop-blur supports-[backdrop-filter]:bg-background/50 relative overflow-hidden rounded-2xl border border-border/40 shadow-[0_0_0_1px_hsl(var(--border)/0.25),0_4px_12px_-2px_rgba(0,0,0,0.25),0_2px_4px_rgba(0,0,0,0.4)]",
          className
        )}
        initial="initial"
        whileHover="hover"
        {...props}
      >
        <motion.div
          className={cn(
            "absolute -inset-2 rounded-3xl z-0 pointer-events-none"
          )}
          variants={navGlowVariants}
        />
        <ul className="flex items-center gap-1.5 relative z-10">
          {items.map((item) => {
            const Icon = item.icon;
            const isActive = item.label === activeItem;

            return (
              <motion.li key={item.label} className="relative">
                <button
                  onClick={() => onItemClick?.(item.label)}
                  onMouseEnter={() => onItemHover?.(item.label)}
                  className="block w-full bg-transparent"
                >
                  <motion.div
                    className="block rounded-xl overflow-visible group relative"
                    style={{ perspective: "600px" }}
                    whileHover="hover"
                    initial="initial"
                  >
                    <motion.div
                      className={cn(
                        "absolute inset-0 z-0 pointer-events-none transition-all",
                        // 降低模糊值和透明度，避免一团光糊在一起
                        "blur-xl opacity-60"
                      )}
                      variants={glowVariants}
                      animate={isActive ? "hover" : "initial"}
                      style={{
                        background: item.gradient,
                        borderRadius: "14px",
                      }}
                    />
                    <motion.div
                      className={cn(
                        "flex items-center gap-1.5 px-4 py-2 relative z-10 transition-colors rounded-xl font-medium text-sm",
                        isActive
                          ? "text-foreground"
                          : "text-muted-foreground group-hover:text-foreground",
                        // Active 胶囊背景更柔和：半透明 + 1px 边框
                        isActive
                          ? "bg-white/4 dark:bg-white/5 border border-white/10 shadow-inner shadow-black/20"
                          : "hover:bg-white/3 dark:hover:bg-white/5"
                      )}
                      variants={itemVariants}
                      transition={sharedTransition}
                      style={{
                        transformStyle: "preserve-3d",
                        transformOrigin: "center bottom",
                      }}
                    >
                      <span
                        className={cn(
                          "transition-colors duration-300",
                          isActive
                            ? item.iconColor
                            : "text-foreground/80 group-hover:text-foreground",
                          `group-hover:${item.iconColor}`
                        )}
                      >
                        <Icon className="h-5 w-5" />
                      </span>
                      <span>{item.label}</span>
                    </motion.div>
                    <motion.div
                      className={cn(
                        "flex items-center gap-1.5 px-4 py-2 absolute inset-0 z-10 transition-colors rounded-xl font-medium text-sm",
                        isActive
                          ? "text-foreground"
                          : "text-muted-foreground group-hover:text-foreground",
                        isActive
                          ? "bg-white/4 dark:bg-white/5 border border-white/10 shadow-inner shadow-black/20"
                          : "hover:bg-white/3 dark:hover:bg-white/5"
                      )}
                      variants={backVariants}
                      transition={sharedTransition}
                      style={{
                        transformStyle: "preserve-3d",
                        transformOrigin: "center top",
                        rotateX: 90,
                      }}
                    >
                      <span
                        className={cn(
                          "transition-colors duration-300",
                          isActive
                            ? item.iconColor
                            : "text-foreground/80 group-hover:text-foreground",
                          `group-hover:${item.iconColor}`
                        )}
                      >
                        <Icon className="h-5 w-5" />
                      </span>
                      <span>{item.label}</span>
                    </motion.div>
                  </motion.div>
                </button>
              </motion.li>
            );
          })}
        </ul>
      </motion.nav>
    );
  }
);

MenuBar.displayName = "MenuBar";
