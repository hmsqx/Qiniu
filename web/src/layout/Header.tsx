import { useState, useEffect, useMemo } from "react";
// 导入 react-router-dom 的钩子
import { useLocation, useNavigate } from "react-router-dom";
import { Hexagon, Sparkles, Home, LayoutGrid, Settings, Menu, Gift } from "lucide-react";

import { cn } from "@/lib/utils";
import { Button } from "@/components/ui/button";
import { Badge } from "@/components/ui/badge";
import { Avatar, AvatarFallback, AvatarImage } from "@/components/ui/avatar";
import { Separator } from "@/components/ui/separator";
import { useAuth } from "@/context/AuthContext";
import RechargeModal from "@/components/RechargeModal";

import { MenuBar, type MenuItem } from "./glowMenu";

// 预加载函数映射（与懒加载路由 chunk 对应）
const preloadMap: Record<string, (() => void) | undefined> = {
  工作台: () => import("@/views/workspace"),
  控制台: () => import("@/layout/AdminLayout"),
  首页: () => import("@/views/home"),
};

// 基础菜单项（所有用户）
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

export const Header = () => {
  const navigate = useNavigate();
  const location = useLocation();
  const { user } = useAuth();

  const [activeItem, setActiveItem] = useState("");

  const derivedMenuItems: MenuItem[] = useMemo(() => {
    if (user?.role === "admin") {
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
        <div className="flex items-center gap-3">
          <Hexagon className="h-7 w-7 text-primary" />
          <h1 className="text-xl font-bold tracking-wider text-foreground hidden sm:block">
            Gen3D
          </h1>
        </div>

        {/* 中间菜单区域 (现在是左侧区域的一部分) */}
        <div className="hidden md:block">
          <MenuBar
            items={derivedMenuItems}
            activeItem={activeItem}
            onItemClick={handleMenuItemClick}
            onItemHover={(label) => preloadMap[label]?.()}
          />
        </div>
      </div>

      {/* 右侧区域: 操作和用户信息 (这部分基本不变) */}
      <div className="flex items-center gap-2 sm:gap-4">
        <div className="hidden lg:flex items-center gap-1">
          <RechargeEntry />
        </div>

        <Separator orientation="vertical" className="h-6 hidden lg:block" />

        <Badge
          variant="secondary"
          className="flex items-center gap-2 py-1.5 px-3 rounded-lg"
        >
          <Sparkles className="h-4 w-4 text-primary" />
          <span className="hidden md:inline text-muted-foreground">次数：</span>
          <span className="font-semibold text-foreground">
            {user?.tokenCount !== undefined ? user.tokenCount : "—"}
          </span>
        </Badge>

        <div>
          <AuthArea />
        </div>

        <div className="md:hidden">
          <Button variant="ghost" size="icon">
            <Menu className="h-6 w-6" />
          </Button>
        </div>
      </div>
    </header>
  );
};

const RechargeEntry: React.FC = () => {
  const { isAuthenticated, openLoginModal } = useAuth();
  const [open, setOpen] = useState(false);

  const onClick = () => {
    if (!isAuthenticated) {
      openLoginModal();
      return;
    }
    setOpen(true);
  };

  return (
    <>
      <Button
        size="sm"
        onClick={onClick}
        className="bg-gradient-to-r from-purple-500/80 to-fuchsia-500/80 text-white hover:from-purple-500 hover:to-fuchsia-500 shadow-sm"
      >
        <Gift className="h-4 w-4 mr-1" /> 充值
      </Button>
      <RechargeModal open={open} onOpenChange={setOpen} />
    </>
  );
};

const AuthArea: React.FC = () => {
  const { isAuthenticated, user, openLoginModal, logout } = useAuth();

  if (!isAuthenticated) {
    return (
      <Button onClick={openLoginModal} variant="ghost">
        登录
      </Button>
    );
  }

  return (
    <button className="flex items-center gap-2 focus:outline-none group">
      <Avatar className="h-9 w-9 ring-1 ring-border group-hover:ring-primary transition">
        {user?.avatar ? (
          <AvatarImage src={user.avatar} alt={user.username} />
        ) : (
          <AvatarFallback>
            {user?.username?.[0]?.toUpperCase() || "U"}
          </AvatarFallback>
        )}
      </Avatar>
      <div>
        <div className="text-sm font-medium max-w-[120px] truncate">
          {user?.username}
        </div>
        <div
          className="text-sm font-medium max-w-[120px] truncate font-size-xs text-foreground/70 group-hover:text-primary"
          onClick={logout}
        >
          登出
        </div>
      </div>
    </button>
  );
};
