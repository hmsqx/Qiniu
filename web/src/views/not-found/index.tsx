import { Link } from "react-router-dom";
import { Button } from "@/components/ui/button";
import { SearchX } from "lucide-react";

export default function NotFound() {
  return (
    <div className="min-h-[60vh] flex flex-col items-center justify-center gap-6 text-center p-6">
      {/* Big 404 with icon in the middle */}
      <div className="flex items-end justify-center gap-4 select-none">
        <span className="text-[72px] md:text-[96px] font-extrabold leading-none tracking-tight bg-gradient-to-b from-foreground to-muted-foreground/70 bg-clip-text text-transparent">
          4
        </span>
        <div className="relative grid place-items-center">
          <div
            className="absolute inset-0 rounded-full bg-primary/20 blur-2xl"
            aria-hidden
          />
          <div className="relative rounded-full border border-border/60 bg-background/70 p-4 shadow-sm">
            <SearchX
              className="w-12 h-12 md:w-16 md:h-16 text-primary"
              strokeWidth={1.75}
            />
          </div>
        </div>
        <span className="text-[72px] md:text-[96px] font-extrabold leading-none tracking-tight bg-gradient-to-b from-foreground to-muted-foreground/70 bg-clip-text text-transparent">
          4
        </span>
      </div>

      <div>
        <h1 className="mt-1 text-2xl md:text-3xl font-semibold">页面未找到</h1>
        <p className="mt-3 max-w-xl text-sm md:text-base text-muted-foreground">
          抱歉，你访问的页面不存在或已被移动。请检查地址是否正确，或返回首页继续浏览。
        </p>
      </div>

      <div className="flex items-center gap-3 mt-1">
        <Button asChild>
          <Link to="/home">返回首页</Link>
        </Button>
        <Button variant="outline" onClick={() => window.history.back()}>
          返回上一页
        </Button>
      </div>
    </div>
  );
}
