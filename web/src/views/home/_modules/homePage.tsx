import { FloatingImageZoom } from "./FloatingImageZoom";
import { InspirationCard } from "./InspirationCard";
import { SectionHeader } from "./SectionHeader";
import { useInfinitePublicModels } from "../hooks/useInfinitePublicModels";

const heroImage = "/homePage.jpg";

export interface HomePageProps {
  scrollRoot?: Element | null;
}

export default function HomePage({ scrollRoot }: HomePageProps) {
  const { items, loading, loadingMore, hasMore, observerRef, error } =
    useInfinitePublicModels({ pageSize: 24, root: scrollRoot });

  return (
    <main className=" mt-1">
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

        <section className="pt-4">
          <SectionHeader title="灵感广场" />
          {!!items.length && (
            <div className="px-1 pb-2 text-xs text-slate-500">
              共 {items.length} 个模型{loadingMore ? "，加载中…" : ""}
            </div>
          )}
          <div className="grid grid-cols-2 gap-x-4 gap-y-3 sm:grid-cols-2 md:grid-cols-3 lg:grid-cols-4 xl:grid-cols-5 2xl:grid-cols-6 contain-paint">
            {items.map((item, idx) => (
              <InspirationCard key={`${item.id}-${idx}`} item={item} />
            ))}
            {loading &&
              items.length === 0 &&
              Array.from({ length: 12 }).map((_, i) => (
                <div
                  key={i}
                  className="aspect-square animate-pulse rounded-xl bg-slate-200/40 dark:bg-slate-700/40"
                />
              ))}
          </div>
          {!loading && items.length === 0 && (
            <div className="mt-6 flex flex-col items-center justify-center gap-2 rounded-xl bg-slate-50 py-10 text-sm text-slate-500 dark:bg-slate-800/30">
              {error ? (
                <>
                  <span>加载失败：{error}</span>
                  <span className="text-xs text-slate-400">请稍后重试</span>
                </>
              ) : (
                <span>暂无数据，去生成一些作品吧～</span>
              )}
            </div>
          )}
          <div ref={observerRef as any} className="h-4 mt-2" />
          {loadingMore && (
            <div className="mt-2 text-center text-xs text-slate-500">
              加载中...
            </div>
          )}
          {!hasMore && items.length > 0 && (
            <div className="mt-3 text-center text-xs text-slate-400">
              没有更多了
            </div>
          )}
        </section>
      </div>
    </main>
  );
}
