import type { MenuItem } from "../menu";
import { Home, LayoutGrid, Settings } from "lucide-react";

// 预加载函数映射（与懒加载路由 chunk 对应）
export const preloadMap: Record<string, (() => void) | undefined> = {
  工作台: () => import("@/views/workspace"),
  控制台: () => import("@/layout/admin"),
  首页: () => import("@/views/home"),
};

const baseMenuItems: MenuItem[] = [
  {
    icon: Home,
    label: "首页",
    href: "/home",
    gradient: "radial-gradient(circle, #ffdde1, transparent 60%)",
    iconColor: "text-rose-400",
  },
  {
    icon: LayoutGrid,
    label: "工作台",
    href: "/workspace",
    gradient: "radial-gradient(circle, #c8f7dc, transparent 60%)",
    iconColor: "text-emerald-400",
  },
];

export function deriveMenuItems(role?: string): MenuItem[] {
  if (role === "admin") {
    return [
      ...baseMenuItems,
      {
        icon: Settings,
        label: "控制台",
        href: "/admin",
        gradient: "radial-gradient(circle, #d0e4ff, transparent 60%)",
        iconColor: "text-sky-500",
      },
    ];
  }
  return baseMenuItems;
}
