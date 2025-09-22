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
  // --- 改动开始 ---

  // 1. 获取 navigate 函数和 location 对象
  const navigate = useNavigate();
  const location = useLocation();

  // 2. 使用 useState 来管理 activeItem 的状态
  const [activeItem, setActiveItem] = useState("");

  // 3. 使用 useEffect 监听路由变化，并更新 activeItem
  useEffect(() => {
    // 查找当前路径匹配的菜单项
    const currentItem = menuItems.find((item) =>
      location.pathname.startsWith(item.href)
    );
    if (currentItem) {
      setActiveItem(currentItem.label);
    } else {
      // 如果没有匹配的，可以设置一个默认值或清空
      setActiveItem("");
    }
  }, [location.pathname]); // 依赖项是 location.pathname

  // 4. 修改点击事件处理函数，实现路由跳转
  const handleMenuItemClick = (label: string) => {
    // 找到被点击菜单项的路由信息
    const item = menuItems.find((i) => i.label === label);
    if (item) {
      // 更新 active 状态（为了立即反馈，虽然 useEffect 也会做）
      setActiveItem(item.label);
      // 执行路由跳转
      navigate(item.href);
    }
  };

  // --- 改动结束 ---

  return (
    <header
      className={cn(
        "fixed top-0 left-0 right-0 z-50",
        "flex items-center justify-between", // 整体依然是左右布局
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

        <Avatar className="h-9 w-9">
          <AvatarImage
            src="https://i.pravatar.cc/150?u=a042581f4e29026704d"
            alt="@username"
          />
          <AvatarFallback>U</AvatarFallback>
        </Avatar>

        <div className="md:hidden">
          <Button variant="ghost" size="icon">
            <Menu className="h-6 w-6" />
          </Button>
        </div>
      </div>
    </header>
  );
};
