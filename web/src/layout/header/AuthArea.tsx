import { useState } from "react";
import { Loader2 } from "lucide-react";
import { cn } from "@/lib/utils";
import { Button } from "@/components/ui/button";
import { Avatar, AvatarFallback, AvatarImage } from "@/components/ui/avatar";
import { useAuth } from "@/context/AuthContext";
import { useToast } from "@/components/ui/use-toast";

const AuthArea: React.FC = () => {
  const { isAuthenticated, user, openLoginModal, logout } = useAuth();

  if (!isAuthenticated) {
    return (
      <Button onClick={openLoginModal} variant="ghost">
        登录
      </Button>
    );
  }

  const { toast } = useToast();
  const [loggingOut, setLoggingOut] = useState(false);

  const handleLogout = async () => {
    if (loggingOut) return;
    setLoggingOut(true);
    try {
      await logout();
      toast({ title: "已退出登录", variant: "success" });
    } catch (error: any) {
      toast({
        variant: "error",
        title: "退出登录失败",
        description:
          error?.message || "退出登录请求失败，但本地已清除登录信息。",
      });
    } finally {
      setLoggingOut(false);
    }
  };

  return (
    <div className="flex items-center gap-2 focus:outline-none group">
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
        <Button
          type="button"
          onClick={handleLogout}
          disabled={loggingOut}
          variant="link"
          size="sm"
          className={cn(
            "h-auto p-0 text-sm font-medium max-w-[120px] truncate text-foreground/70",
            !loggingOut && "group-hover:text-primary"
          )}
        >
          {loggingOut ? (
            <Loader2 className="mr-1 h-3 w-3 animate-spin" />
          ) : (
            "登出"
          )}
        </Button>
      </div>
    </div>
  );
};

export default AuthArea;
