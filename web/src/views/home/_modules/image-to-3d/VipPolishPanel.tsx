import React, { useRef, useState } from "react";
import { vipOptimizeImages } from "@/api/image";
import { Button } from "@/components/ui/button";
import { Badge } from "@/components/ui/badge";
import { Loader2, Wand2 } from "lucide-react";

type Props = {
  base64: string; // 无 data: 前缀
  count?: number; // 默认 3
  onSelect?: (b64: string) => void;
};

const VipPolishPanel: React.FC<Props> = ({ base64, count = 2, onSelect }) => {
  const [busy, setBusy] = useState(false);
  const [variants, setVariants] = useState<string[]>([]);

  const run = async () => {
    if (busy) return;
    setBusy(true);
    try {
      const res = await vipOptimizeImages({
        image_base64: base64,
        num_variants: count,
      });
      if (!res?.success) throw new Error(res?.message || "生成失败");
      const imgs = (res.images || [])
        .filter((it) => it && it.success && it.image_base64)
        .map((it) => it.image_base64);
      setVariants(imgs.slice(0, count));
    } catch (e) {
      console.warn("vipOptimizeImages failed", e);
    } finally {
      setBusy(false);
    }
  };

  return (
    <div className="space-y-2">
      <div className="flex items-center justify-between">
        <div className="flex items-center gap-2">
          <span className="text-sm text-foreground/90">专享润色</span>
          <Badge variant="secondary">Pro</Badge>
        </div>
        <Button size="sm" variant="outline" disabled={busy} onClick={run}>
          {busy ? (
            <Loader2 className="h-4 w-4 mr-1 animate-spin" />
          ) : (
            <Wand2 className="h-4 w-4 mr-1" />
          )}
          {variants.length ? "重新生成" : `生成 ${count} 张预览`}
        </Button>
      </div>

      <div className="grid grid-cols-1 sm:grid-cols-2 md:grid-cols-2 gap-3">
        {(busy && variants.length === 0
          ? Array.from({ length: count })
          : variants
        ).map((b64, i) => (
          <VariantTile
            key={i}
            index={i}
            busy={busy && variants.length === 0}
            variantB64={String(b64 || "")}
            originalB64={base64}
            onReplace={() => {
              const selected = variants[i];
              if (!selected) return;
              const prevMain = base64;
              // 先通知父级替换主图
              onSelect?.(selected);
              // 再在面板中用旧主图顶替该预览，实现“交换到下面”
              setVariants((arr) => {
                const next = arr.slice();
                next[i] = prevMain;
                return next;
              });
            }}
          />
        ))}
      </div>
    </div>
  );
};

export default VipPolishPanel;

// 内部组件：支持按住对比原图 + 点击替换（已移除悬停放大）
const VariantTile: React.FC<{
  index: number;
  busy?: boolean;
  variantB64: string;
  originalB64: string;
  onReplace?: () => void;
}> = ({ index, busy, variantB64, originalB64, onReplace }) => {
  const containerRef = useRef<HTMLDivElement | null>(null);
  const [compare, setCompare] = useState(false); // 按住鼠标左键显示原图
  // 移除悬浮放大相关状态与计算

  return (
    <div className="relative overflow-hidden rounded-xl border bg-background/60 ring-1 ring-border/50 shadow-sm cursor-pointer">
      <div
        ref={containerRef}
        className="aspect-[4/3] w-full bg-muted/40 flex items-center justify-center group"
        onMouseLeave={() => {
          setCompare(false);
        }}
        onMouseDown={() => setCompare(true)}
        onMouseUp={() => setCompare(false)}
        onClick={() => {
          if (!busy && variantB64) onReplace?.();
        }}
      >
        {busy ? (
          <Loader2 className="h-5 w-5 animate-spin text-muted-foreground" />
        ) : (
          <img
            src={`data:image/*;base64,${compare ? originalB64 : variantB64}`}
            alt={`润色 ${index + 1}`}
            className="w-full h-full object-contain select-none"
            draggable={false}
          />
        )}

        {/* 顶部提示：按住对比原图 */}
        {!busy && (
          <div className="pointer-events-none absolute right-2 top-2 text-[11px] px-2 py-0.5 rounded-full bg-background/70 border text-muted-foreground">
            {compare ? "对比：原图" : "按住对比 · 点击替换"}
          </div>
        )}
      </div>

      <div className="absolute left-2 top-2">
        <Badge variant="outline">#{index + 1}</Badge>
      </div>
      {/* 去掉底部按钮，点击卡片直接替换 */}
    </div>
  );
};
