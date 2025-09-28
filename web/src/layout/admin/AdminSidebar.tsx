import { adminSideMenus } from "@/constants/adminNav";
import SidebarContainer from "./SidebarContainer";
import SidebarNavItem from "./SidebarNavItem";

type Props = {
  variant?: "inline" | "drawer";
  onNavigate?: () => void;
  className?: string;
};

export default function AdminSidebar({
  variant = "inline",
  onNavigate,
  className,
}: Props) {
  return (
    <SidebarContainer variant={variant} className={className}>
      <nav className="flex-1 overflow-auto py-2 text-sm">
        <ul className="space-y-1.5 px-2">
          {adminSideMenus.map((m) => (
            <SidebarNavItem
              key={m.to}
              to={m.to}
              label={m.label}
              Icon={m.icon}
              onNavigate={onNavigate}
            />
          ))}
        </ul>
      </nav>
    </SidebarContainer>
  );
}
