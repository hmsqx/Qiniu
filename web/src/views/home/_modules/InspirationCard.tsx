import React, { useState } from "react";
import { Card } from "@/components/ui/card";
import { Heart, Download, Loader2 } from "lucide-react";
import type { Inspiration } from "../data/type";
import { formatNumber } from "@/lib/utils";
import { likeModel } from "@/api/like";
import { useToast } from "@/components/ui/use-toast";
import { useAuth } from "@/context/AuthContext";
import { Link } from "react-router-dom";
import { getExt, toProxiedUrl } from "@/utils/url";

interface InspirationCardProps {
  item: Inspiration;
}

export function InspirationCard({ item }: InspirationCardProps) {
  const { toast } = useToast();
  const { user, openLoginModal } = useAuth();

  const [liked, setLiked] = useState(!!item.islike);
  const [likeSubmitting, setLikeSubmitting] = useState(false);
  const [popping, setPopping] = useState(false);

  const onLike = async (e: React.MouseEvent<HTMLButtonElement>) => {
    e.preventDefault();
    // 必须登录才允许点赞：无 token 则弹出登录
    if (!user?.sessionToken) {
      openLoginModal();
      return;
    }
    if (likeSubmitting) return;
    const nextLiked = !liked;
    setLiked(nextLiked);
    setPopping(true);
    setTimeout(() => setPopping(false), 180);

    try {
      setLikeSubmitting(true);
      await likeModel(item.jobId);
    } catch (err: any) {
      setLiked((v) => !v);
      toast({
        title: "操作失败",
        description: err?.message || "点赞失败，请稍后重试",
        variant: "error",
      });
    } finally {
      setLikeSubmitting(false);
    }
  };

  const imageUrl = item.previewImages || "";
  const modelUrl = item.fileurl || "";
  const jobId = item.jobId || "";

  const hasImage = !!imageUrl;
  const canPreview = !!modelUrl;
  const viewerUrl = canPreview
    ? `/viewer?url=${encodeURIComponent(toProxiedUrl(modelUrl))}&format=${
        item.resultFormat || getExt(modelUrl) || ""
      }&jobId=${encodeURIComponent(jobId)}`
    : "";

  return (
    <Card
      className="group overflow-hidden rounded-xl border-none bg-transparent shadow-none transition"
      style={{ contentVisibility: "auto", contain: "content" }}
    >
      <div className="relative aspect-square overflow-hidden rounded-xl bg-slate-100 dark:bg-slate-800">
        {hasImage ? (
          <img
            src={imageUrl}
            alt={item.prompt}
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
        {canPreview && (
          <Link
            to={viewerUrl}
            className="absolute inset-0 z-10"
            aria-label="打开预览"
          />
        )}
        <div className="pointer-events-none absolute inset-0 bg-gradient-to-t from-black/20 via-black/0 to-transparent opacity-0 transition group-hover:opacity-100" />
        <div className="absolute bottom-3 left-3 z-20 flex translate-y-1 items-center gap-1 rounded-full bg-white/90 px-2 py-1 text-[11px] text-slate-700 opacity-0 shadow-sm ring-1 ring-slate-200 transition-all duration-200 ease-out group-hover:translate-y-0 group-hover:opacity-100 group-hover:backdrop-blur">
          <Download className="h-4 w-4 text-slate-500" />
          <span>{formatNumber(item.downloadCount || 0)}</span>
        </div>

        <div className="absolute bottom-3 right-3 z-20 flex translate-y-1 items-center gap-2 opacity-0 transition-all duration-200 ease-out group-hover:translate-y-0 group-hover:opacity-100">
          <button
            type="button"
            aria-label={liked ? "取消点赞" : "点赞"}
            aria-pressed={liked}
            onClick={onLike}
            className={`relative z-20 inline-flex h-8 w-8 items-center justify-center rounded-full bg-white/90 shadow-sm ring-1 ring-slate-200 backdrop-blur transition-all duration-200 hover:bg-white active:scale-95 focus:outline-none focus:ring-2 focus:ring-rose-400/40 ${
              popping ? "scale-110" : ""
            }`}
          >
            <Heart
              className="h-4 w-4 stroke-[2px] text-rose-500"
              style={{ fill: liked ? "currentColor" : "none" }}
            />
            {(item.like || liked) && (
              <span className="absolute -bottom-1 -right-1 inline-flex h-4 min-w-[16px] items-center justify-center rounded-full bg-rose-500 px-1 text-[10px] font-medium text-white">
                {formatNumber(
                  (item.like || 0) + (liked && !item.islike ? 1 : 0)
                )}
              </span>
            )}
          </button>
        </div>
      </div>
    </Card>
  );
}
