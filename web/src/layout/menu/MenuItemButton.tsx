import { motion } from "framer-motion";
import { cn } from "@/lib/utils";
import {
  backVariants,
  glowVariants,
  itemVariants,
  sharedTransition,
} from "./variants";
import type { MenuItem } from "./types";

type Props = {
  item: MenuItem;
  isActive: boolean;
  onClick?: (label: string) => void;
  onHover?: (label: string) => void;
};

export default function MenuItemButton({
  item,
  isActive,
  onClick,
  onHover,
}: Props) {
  const Icon = item.icon as any;
  return (
    <motion.li className="relative">
      <button
        onClick={() => onClick?.(item.label)}
        onMouseEnter={() => onHover?.(item.label)}
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
              "blur-xl opacity-60"
            )}
            variants={glowVariants}
            animate={isActive ? "hover" : "initial"}
            style={{ background: item.gradient, borderRadius: "14px" }}
          />
          <motion.div
            className={cn(
              "flex items-center gap-1.5 px-4 py-2 relative z-10 transition-colors rounded-xl font-medium text-sm",
              isActive
                ? "text-foreground"
                : "text-muted-foreground group-hover:text-foreground",
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
}
