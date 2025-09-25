import { FloatingImageZoom } from "./FloatingImageZoom";
import { InspirationCard } from "./InspirationCard";
import { SectionHeader } from "./SectionHeader";
import { useInfinitePublicModels } from "./hooks/useInfinitePublicModels";
import { Button } from "@/components/ui/button";

const heroImage = "/homePage.jpg";

export default function HomePage() {
  // [修改] 解构出 retry 函数
  const { items, loading, loadingMore, error, hasMore, observerRef, retry } =
    useInfinitePublicModels({ pageSize: 24 });

  return (
    <main className="min-h-screen ">
      <div className="mx-auto">
        <div className="flex px-2 gap-2 ">
          <FloatingImageZoom
            src={heroImage}
            className="h-[260px] md:h-[360px]"
          />
          <FloatingImageZoom
            src={heroImage}
            className="hidden h-[260px] md:block md:h-[360px]"
          />
        </div>

        <section className="mt-8 lg:mt-12">
          <SectionHeader title="灵感广场" />
          <div className="grid grid-cols-2 gap-x-4 sm:grid-cols-2 md:grid-cols-3 lg:grid-cols-4 xl:grid-cols-5 2xl:grid-cols-6">
            {items.map((item, idx) => (
              <InspirationCard key={idx} item={item} />
            ))}
            {loading &&
              items.length === 0 &&
              Array.from({ length: 12 }).map((_, i) => (
                <div
                  key={i}
                  className="aspect-[16/11] animate-pulse rounded-xl bg-slate-200/40 dark:bg-slate-700/40"
                />
              ))}
          </div>
          {error && (
            <div className="mt-4 flex flex-col items-center gap-2 text-sm text-red-500">
              <span>加载失败：{error}</span>
              {/* [修改] onClick 调用 retry */}
              <Button size="sm" variant="outline" onClick={retry}>
                重试
              </Button>
            </div>
          )}
          <div ref={observerRef as any} className="h-6" />
          {loadingMore && (
            <div className="mt-4 text-center text-xs text-slate-500">
              加载中...
            </div>
          )}
          {!hasMore && items.length > 0 && (
            <div className="mt-6 text-center text-xs text-slate-400">
              没有更多了
            </div>
          )}
        </section>
      </div>
    </main>
  );
}
