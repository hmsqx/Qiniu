import * as React from "react";
import { motion } from "framer-motion";
import { cn } from "@/lib/utils";
import { navGlowVariants } from "./variants";
import MenuItemButton from "./MenuItemButton";
import type { MenuBarProps } from "./types";

export const MenuBar = React.forwardRef<HTMLDivElement, MenuBarProps>(
  (
    { className, items, activeItem, onItemClick, onItemHover, ...props },
    ref
  ) => {
    return (
      <motion.nav
        ref={ref}
        className={cn(
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
          {items.map((item) => (
            <MenuItemButton
              key={item.label}
              item={item}
              isActive={item.label === activeItem}
              onClick={onItemClick}
              onHover={onItemHover}
            />
          ))}
        </ul>
      </motion.nav>
    );
  }
);

MenuBar.displayName = "MenuBar";

export default MenuBar;
