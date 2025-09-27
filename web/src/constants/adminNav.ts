import type { LucideIcon } from "lucide-react";
import { LayoutGrid, Users, Boxes } from "lucide-react";

export type AdminMenuItem = {
  to: string;
  label: string;
  icon: LucideIcon;
};

export const adminSideMenus: AdminMenuItem[] = [
  { to: "/admin", label: "概览", icon: LayoutGrid },
  { to: "/admin/users", label: "用户管理", icon: Users },
  { to: "/admin/models", label: " 模型管理", icon: Boxes },
];
