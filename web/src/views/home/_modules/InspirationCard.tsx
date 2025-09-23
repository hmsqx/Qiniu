import React, { useMemo, useState } from "react";
import { Card } from "@/components/ui/card";
import { Heart, Eye } from "lucide-react";
import type { Inspiration } from "./type";
import { formatNumber } from "@/lib/utils";

interface InspirationCardProps {
  item: Inspiration;
}

export function InspirationCard({ item }: InspirationCardProps) {
  const [liked, setLiked] = useState(false);
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

  return (
    <Card className="group overflow-hidden rounded-xl border-none bg-transparent shadow-none transition">
      <div className="relative aspect-[16/11] overflow-hidden rounded-xl">
        <img
          src={item.image}
          alt={item.title}
          className="block h-full w-full object-cover transition duration-300 ease-out group-hover:scale-[1.02]"
        />
        <div className="pointer-events-none absolute inset-0 bg-gradient-to-t from-black/10 to-transparent opacity-0 transition group-hover:opacity-100" />

        <div className="absolute bottom-3 left-3 z-20 flex translate-y-1 items-center gap-1 rounded-full bg-white/90 px-2 py-1 text-xs text-slate-700 opacity-0 shadow-sm ring-1 ring-slate-200 backdrop-blur transition-all duration-200 ease-out group-hover:translate-y-0 group-hover:opacity-100">
          <Eye className="h-4 w-4 text-slate-500" />
          <span>{formatNumber(views)}</span>
        </div>

        <div className="absolute bottom-3 right-3 z-20 translate-y-1 opacity-0 transition-all duration-200 ease-out group-hover:translate-y-0 group-hover:opacity-100">
          <button
            type="button"
            aria-label={liked ? "取消点赞" : "点赞"}
            aria-pressed={liked}
            onClick={onLike}
            className={`relative inline-flex h-9 w-9 items-center justify-center rounded-full bg-white/90 shadow-sm ring-1 ring-slate-200 backdrop-blur transition-all duration-200 hover:bg-white active:scale-95 focus:outline-none focus:ring-2 focus:ring-rose-400/40 ${
              popping ? "scale-110" : ""
            }`}
          >
            <Heart
              className="h-5 w-5 stroke-[2px] text-rose-500"
              style={{ fill: liked ? "currentColor" : "none" }}
            />
          </button>
        </div>
      </div>
    </Card>
  );
}
