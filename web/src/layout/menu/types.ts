import type { LucideIcon } from "lucide-react";
import type { HTMLMotionProps } from "framer-motion";

export interface MenuItem {
  icon: LucideIcon | React.FC;
  label: string;
  href: string;
  gradient: string;
  iconColor: string;
}

export interface MenuBarProps extends Omit<HTMLMotionProps<"nav">, "children"> {
  items: MenuItem[];
  activeItem?: string;
  onItemClick?: (label: string) => void;
  onItemHover?: (label: string) => void;
}
