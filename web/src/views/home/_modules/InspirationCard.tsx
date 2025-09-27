import React, { useMemo, useState } from "react";
import { Card } from "@/components/ui/card";
import { Heart, Eye, Download, Loader2 } from "lucide-react";
import type { Inspiration } from "../data/type";
import { formatNumber } from "@/lib/utils";

interface InspirationCardProps {
  item: Inspiration;
}

export function InspirationCard({ item }: InspirationCardProps) {
  const [liked, setLiked] = useState(!!item.isLiked);
  const [popping, setPopping] = useState(false);
  const views = useMemo(
    () => item.views ?? Math.floor(3000 + Math.random() * 20000),
    [item.views]
  );

  const onLike = (e: React.MouseEvent<HTMLButtonElement>) => {
    e.preventDefault();
    setLiked((v) => !v);
    setPopping(true);
    setTimeout(() => setPopping(false), 180);
  };

  const hasImage = !!item.image;

  return (
    <Card
      className="group overflow-hidden rounded-xl border-none bg-transparent shadow-none transition"
      style={{ contentVisibility: "auto", contain: "content" }}
    >
      <div className="relative aspect-square overflow-hidden rounded-xl bg-slate-100 dark:bg-slate-800">
        {hasImage ? (
          <img
            src={item.image}
            alt={item.title}
            loading="lazy"
            decoding="async"
            className="block h-full w-full object-cover transition duration-300 ease-out group-hover:scale-[1.02]"
            style={{ willChange: "transform" }}
          />
        ) : (
          <div className="flex h-full w-full flex-col items-center justify-center gap-2 text-xs text-slate-400">
            <Loader2 className="h-5 w-5 animate-spin" />
            <span>无预览</span>
          </div>
        )}
        <div className="pointer-events-none absolute inset-0 bg-gradient-to-t from-black/20 via-black/0 to-transparent opacity-0 transition group-hover:opacity-100" />

        {/* status badge removed per request */}

        <div className="absolute bottom-3 left-3 z-20 flex translate-y-1 items-center gap-1 rounded-full bg-white/90 px-2 py-1 text-[11px] text-slate-700 opacity-0 shadow-sm ring-1 ring-slate-200 transition-all duration-200 ease-out group-hover:translate-y-0 group-hover:opacity-100 group-hover:backdrop-blur">
          <Eye className="h-4 w-4 text-slate-500" />
          <span>{formatNumber(views)}</span>
        </div>

        <div className="absolute bottom-3 right-3 z-20 flex translate-y-1 items-center gap-2 opacity-0 transition-all duration-200 ease-out group-hover:translate-y-0 group-hover:opacity-100">
          <div className="flex items-center gap-1 rounded-full bg-white/90 px-2 py-1 text-[11px] text-slate-700 shadow-sm ring-1 ring-slate-200 group-hover:backdrop-blur">
            <Download className="h-4 w-4 text-slate-500" />
            <span>{formatNumber(item.downloadCount || 0)}</span>
          </div>
          <button
            type="button"
            aria-label={liked ? "取消点赞" : "点赞"}
            aria-pressed={liked}
            onClick={onLike}
            className={`relative inline-flex h-8 w-8 items-center justify-center rounded-full bg-white/90 shadow-sm ring-1 ring-slate-200 backdrop-blur transition-all duration-200 hover:bg-white active:scale-95 focus:outline-none focus:ring-2 focus:ring-rose-400/40 ${
              popping ? "scale-110" : ""
            }`}
          >
            <Heart
              className="h-4 w-4 stroke-[2px] text-rose-500"
              style={{ fill: liked ? "currentColor" : "none" }}
            />
            {!!item.like && (
              <span className="absolute -bottom-1 -right-1 inline-flex h-4 min-w-[16px] items-center justify-center rounded-full bg-rose-500 px-1 text-[10px] font-medium text-white">
                {formatNumber(item.like)}
              </span>
            )}
          </button>
        </div>
      </div>
    </Card>
  );
}
