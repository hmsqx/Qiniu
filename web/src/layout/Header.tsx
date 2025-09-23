import { useState, useEffect } from "react";
// 导入 react-router-dom 的钩子
import { useLocation, useNavigate } from "react-router-dom";
import {
  Hexagon,
  Mail,
  Gift,
  Sparkles,
  Home,
  LayoutGrid,
  Settings,
  Menu,
} from "lucide-react";

import { cn } from "@/lib/utils";
import { Button } from "@/components/ui/button";
import { Badge } from "@/components/ui/badge";
import { Avatar, AvatarFallback, AvatarImage } from "@/components/ui/avatar";
import { Separator } from "@/components/ui/separator";
import { useAuth } from "@/context/AuthContext";

import { MenuBar, type MenuItem } from "./glowMenu";

// 定义菜单项数据 (这部分无需改动)
const menuItems: MenuItem[] = [
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
  {
    icon: Settings,
    label: "设置",
    href: "/settings",
    gradient: "radial-gradient(circle, #d1eaff, transparent 60%)",
    iconColor: "text-sky-400",
  },
];

export const Header = () => {
  const navigate = useNavigate();
  const location = useLocation();

  const [activeItem, setActiveItem] = useState("");

  useEffect(() => {
    const currentItem = menuItems.find((item) =>
      location.pathname.startsWith(item.href)
    );
    if (currentItem) {
      setActiveItem(currentItem.label);
    } else {
      setActiveItem("");
    }
  }, [location.pathname]);

  const handleMenuItemClick = (label: string) => {
    const item = menuItems.find((i) => i.label === label);
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
            items={menuItems}
            // 5. 将动态计算的 activeItem 和新的点击处理器传入
            activeItem={activeItem}
            onItemClick={handleMenuItemClick}
          />
        </div>
      </div>

      {/* 右侧区域: 操作和用户信息 (这部分基本不变) */}
      <div className="flex items-center gap-2 sm:gap-4">
        <div className="hidden lg:flex items-center gap-1">
          <Button variant="ghost" size="sm" className="flex items-center gap-2">
            <Mail className="h-4 w-4" />
            联系我们
          </Button>
          <Button variant="ghost" size="sm" className="flex items-center gap-2">
            <Gift className="h-4 w-4" />
            邀请好友
          </Button>
        </div>

        <Separator orientation="vertical" className="h-6 hidden lg:block" />

        <Badge
          variant="secondary"
          className="flex items-center gap-2 py-1.5 px-3 rounded-lg"
        >
          <Sparkles className="h-4 w-4 text-primary" />
          <span className="hidden md:inline text-muted-foreground">次数：</span>
          <span className="font-semibold text-foreground">16</span>
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
    <div className="flex items-center gap-2">
      <Avatar className="h-9 w-9">
        {user?.avatar ? (
          <AvatarImage src={user.avatar} alt={user.username} />
        ) : (
          <AvatarFallback>
            {user?.username?.[0]?.toUpperCase() || "U"}
          </AvatarFallback>
        )}
      </Avatar>
      <div className="hidden sm:block">
        <div className="text-sm font-medium">{user?.username}</div>
        <button className="text-xs text-muted-foreground" onClick={logout}>
          登出
        </button>
      </div>
    </div>
  );
};
