import React, { useState } from "react";
import { Card, CardContent } from "@/components/ui/card";
import { Input } from "@/components/ui/input";
import { Button } from "@/components/ui/button";
import { Search, RotateCcw, Eraser, Loader2 } from "lucide-react";

export interface ModelFiltersState {
  minLike?: number;
  maxLike?: number;
  minDownload?: number;
  maxDownload?: number;
}

interface ModelFiltersProps {
  initial: ModelFiltersState;
  loading: boolean;
  onApply: (state: ModelFiltersState) => void;
  onRefresh: () => void;
}

export const ModelFilters: React.FC<ModelFiltersProps> = ({
  initial,
  loading,
  onApply,
  onRefresh,
}) => {
  const [minLike, setMinLike] = useState<string>(
    typeof initial.minLike === "number" ? String(initial.minLike) : ""
  );
  const [maxLike, setMaxLike] = useState<string>(
    typeof initial.maxLike === "number" ? String(initial.maxLike) : ""
  );
  const [minDownload, setMinDownload] = useState<string>(
    typeof initial.minDownload === "number" ? String(initial.minDownload) : ""
  );
  const [maxDownload, setMaxDownload] = useState<string>(
    typeof initial.maxDownload === "number" ? String(initial.maxDownload) : ""
  );

  const apply = () => {
    onApply({
      minLike: minLike === "" ? undefined : Number(minLike),
      maxLike: maxLike === "" ? undefined : Number(maxLike),
      minDownload: minDownload === "" ? undefined : Number(minDownload),
      maxDownload: maxDownload === "" ? undefined : Number(maxDownload),
    });
  };

  const reset = () => {
    setMinLike("");
    setMaxLike("");
    setMinDownload("");
    setMaxDownload("");
    onApply({});
  };

  return (
    <Card className="backdrop-blur supports-[backdrop-filter]:bg-background/70">
      <CardContent className="pt-4">
        <div className="flex flex-col gap-4 md:flex-row md:items-center md:flex-wrap">
          <div className="flex gap-3 flex-col sm:flex-row">
            <div className="flex items-center gap-2">
              <Input
                type="number"
                placeholder="最小点赞"
                value={minLike}
                onChange={(e) => setMinLike(e.target.value)}
                className="w-[120px] input-no-spin"
                min={0}
              />
              <span className="text-muted-foreground">-</span>
              <Input
                type="number"
                placeholder="最大点赞"
                value={maxLike}
                onChange={(e) => setMaxLike(e.target.value)}
                className="w-[120px] input-no-spin"
                min={0}
              />
            </div>
            <div className="flex items-center gap-2">
              <Input
                type="number"
                placeholder="最小下载"
                value={minDownload}
                onChange={(e) => setMinDownload(e.target.value)}
                className="w-[120px] input-no-spin"
                min={0}
              />
              <span className="text-muted-foreground">-</span>
              <Input
                type="number"
                placeholder="最大下载"
                value={maxDownload}
                onChange={(e) => setMaxDownload(e.target.value)}
                className="w-[120px] input-no-spin"
                min={0}
              />
            </div>
          </div>
          <div className="flex gap-2">
            <Button onClick={apply} disabled={loading} className="gap-1">
              {loading ? (
                <Loader2 className="w-4 h-4 animate-spin" />
              ) : (
                <Search className="w-4 h-4" />
              )}
              搜索
            </Button>

            <Button
              type="button"
              variant="outline"
              onClick={reset}
              disabled={loading}
              className="gap-1"
            >
              {loading ? (
                <Loader2 className="w-4 h-4 animate-spin" />
              ) : (
                <Eraser className="w-4 h-4" />
              )}
              重置
            </Button>
            <Button
              type="button"
              variant="ghost"
              onClick={onRefresh}
              disabled={loading}
              className="gap-1"
            >
              {loading ? (
                <Loader2 className="w-4 h-4 animate-spin" />
              ) : (
                <RotateCcw className="w-4 h-4" />
              )}
              刷新
            </Button>
          </div>
        </div>
      </CardContent>
    </Card>
  );
};
