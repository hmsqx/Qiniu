import type { LucideIcon } from "lucide-react";
import { LayoutGrid, Users } from "lucide-react";

export type AdminMenuItem = {
  to: string;
  label: string;
  icon: LucideIcon;
};

export const adminSideMenus: AdminMenuItem[] = [
  { to: "/admin", label: "概览", icon: LayoutGrid },
  { to: "/admin/users", label: "用户管理", icon: Users },
];
