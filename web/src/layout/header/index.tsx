import { useState, useEffect, useMemo } from "react";
// 导入 react-router-dom 的钩子
import { useLocation, useNavigate } from "react-router-dom";
import { cn } from "@/lib/utils";
import { Separator } from "@/components/ui/separator";
import { useAuth } from "@/context/AuthContext";
import type { MenuItem } from "../menu";
import HeaderLogo from "./HeaderLogo";
import HeaderMenu from "./HeaderMenu";
import TokenBadge from "./TokenBadge";
import RechargeEntry from "./RechargeEntry";
import AuthArea from "./AuthArea";
import MobileMenuButton from "./MobileMenuButton";
import { deriveMenuItems } from "./constants";

export const Header = () => {
  const navigate = useNavigate();
  const location = useLocation();
  const { user } = useAuth();

  const [activeItem, setActiveItem] = useState("");

  const derivedMenuItems: MenuItem[] = useMemo(() => {
    const role = user?.role ?? undefined;
    return deriveMenuItems(role);
  }, [user?.role]);

  useEffect(() => {
    const currentItem = derivedMenuItems.find((item) =>
      location.pathname.startsWith(item.href)
    );
    if (currentItem) {
      setActiveItem(currentItem.label);
    } else {
      setActiveItem("");
    }
  }, [location.pathname, derivedMenuItems]);

  const handleMenuItemClick = (label: string) => {
    const item = derivedMenuItems.find((i) => i.label === label);
    if (item) {
      setActiveItem(item.label);
      navigate(item.href);
    }
  };

  return (
    <header
      className={cn(
        "flex items-center justify-between",
        "px-4 sm:px-6 py-3",
        "bg-background/80 backdrop-blur-lg",
        "border-b border-border/60 shadow-sm"
      )}
    >
      <div className="flex items-center gap-4">
        <HeaderLogo />
        <HeaderMenu
          items={derivedMenuItems}
          activeItem={activeItem}
          onItemClick={handleMenuItemClick}
        />
      </div>

      {/* 右侧区域: 操作和用户信息 */}
      <div className="flex items-center gap-2 sm:gap-4">
        <div className="hidden lg:flex items-center gap-1">
          <RechargeEntry />
        </div>

        <Separator orientation="vertical" className="h-6 hidden lg:block" />

        <TokenBadge tokenCount={user?.tokenCount} />

        <div>
          <AuthArea />
        </div>

        <MobileMenuButton />
      </div>
    </header>
  );
};
