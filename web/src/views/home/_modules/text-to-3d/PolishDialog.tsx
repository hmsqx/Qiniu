import { Textarea } from "@/components/ui/textarea";
import { Button } from "@/components/ui/button";
import { Label } from "@/components/ui/label";
import {
  Dialog,
  DialogContent,
  DialogFooter,
  DialogHeader,
  DialogTitle,
} from "@/components/ui/dialog";
import {
  Tooltip,
  TooltipContent,
  TooltipProvider,
  TooltipTrigger,
} from "@/components/ui/tooltip";
import { Copy, Trash2, Wand2 } from "lucide-react";
import type { ToastMessage } from "@/components/ui/use-toast";

type Props = {
  open: boolean;
  onOpenChange: (v: boolean) => void;
  sourceText: string;
  setSourceText: (v: string) => void;
  polishedText: string;
  setPolishedText: (v: string) => void;
  optimizing?: boolean;
  onOptimize: () => void;
  onApply: () => void;
  toast: (opts: ToastMessage) => string;
};

export function PolishDialog({
  open,
  onOpenChange,
  sourceText,
  setSourceText,
  polishedText,
  setPolishedText,
  optimizing = false,
  onOptimize,
  onApply,
  toast,
}: Props) {
  /**
   * PolishDialog
   * 对话框：左侧原文，右侧润色结果；支持清空、复制、自动润色与应用。
   */
  return (
    <Dialog open={open} onOpenChange={onOpenChange}>
      <DialogContent
        className="sm:max-w-3xl rounded-xl border border-white/10 bg-background/80 backdrop-blur-2xl shadow-2xl"
        onInteractOutside={(e) => e.preventDefault()}
      >
        <DialogHeader className="px-2 py-1">
          <DialogTitle className="text-base">润色内容</DialogTitle>
        </DialogHeader>
        <div className="relative grid grid-cols-1 gap-4 sm:grid-cols-2">
          <div className="flex flex-col gap-2">
            <div className="flex h-8 items-center justify-between">
              <Label htmlFor="polish-source">原文</Label>
              <div className="invisible flex items-center gap-1.5">
                <Button
                  type="button"
                  variant="ghost"
                  size="sm"
                  className="h-8 px-2"
                >
                  <span className="sr-only">占位</span>
                </Button>
                <Button
                  type="button"
                  variant="ghost"
                  size="sm"
                  className="h-8 px-2"
                >
                  <span className="sr-only">占位</span>
                </Button>
              </div>
            </div>
            <Textarea
              id="polish-source"
              value={sourceText}
              onChange={(e) => setSourceText(e.target.value)}
              placeholder="在此粘贴或编辑原文"
              className="h-56 resize-none"
            />
          </div>
          <div className="flex flex-col gap-2">
            <div className="flex h-8 items-center justify-between">
              <Label htmlFor="polish-result">润色结果</Label>
              <div className="flex items-center gap-1.5">
                <Button
                  type="button"
                  variant="ghost"
                  size="sm"
                  className="h-8 px-2 text-muted-foreground hover:text-foreground"
                  onClick={() => {
                    if (!polishedText) return;
                    setPolishedText("");
                    toast({ title: "已清空", variant: "success" });
                  }}
                  aria-label="清空润色结果"
                >
                  <Trash2 className="size-4" />
                </Button>
                <Button
                  type="button"
                  variant="ghost"
                  size="sm"
                  className="h-8 px-2 text-muted-foreground hover:text-foreground"
                  onClick={async () => {
                    try {
                      await navigator.clipboard.writeText(polishedText || "");
                      toast({ title: "已复制到剪贴板", variant: "success" });
                    } catch (err) {
                      toast({
                        title: "复制失败",
                        description: String(err),
                        variant: "error",
                      });
                    }
                  }}
                  aria-label="复制润色结果"
                >
                  <Copy className="size-4" />
                </Button>
              </div>
            </div>
            <TooltipProvider>
              <Tooltip open={optimizing ? true : undefined}>
                <TooltipTrigger asChild>
                  <div>
                    <Textarea
                      id="polish-result"
                      value={polishedText}
                      onChange={(e) => setPolishedText(e.target.value)}
                      placeholder="在此撰写或生成润色后的内容"
                      className="h-56 resize-none"
                      readOnly={optimizing}
                    />
                  </div>
                </TooltipTrigger>
                {optimizing && (
                  <TooltipContent>生成中，暂不可编辑</TooltipContent>
                )}
              </Tooltip>
            </TooltipProvider>
          </div>
          {/* 中间操作按钮 */}
          <div className="pointer-events-none absolute left-1/2 top-1/2 z-10 -translate-x-1/2 -translate-y-1/2">
            <TooltipProvider>
              <Tooltip>
                <TooltipTrigger asChild>
                  <Button
                    type="button"
                    size="icon"
                    variant="secondary"
                    className={`pointer-events-auto size-9 rounded-full border border-white/10 backdrop-blur shadow-md transition-colors ${
                      optimizing
                        ? "bg-muted text-muted-foreground cursor-not-allowed"
                        : "bg-background/90 hover:bg-background"
                    }`}
                    onClick={onOptimize}
                    disabled={optimizing}
                    aria-label="自动润色"
                    aria-busy={optimizing}
                  >
                    {optimizing ? (
                      <span
                        className="inline-block size-5 animate-spin rounded-full border-2 border-current/30 border-t-current"
                        aria-hidden
                      />
                    ) : (
                      <Wand2 className="size-5" />
                    )}
                  </Button>
                </TooltipTrigger>
                <TooltipContent>
                  {optimizing ? "生成中..." : "自动润色"}
                </TooltipContent>
              </Tooltip>
            </TooltipProvider>
          </div>
        </div>
        <DialogFooter className="pt-2">
          <Button
            type="button"
            variant="outline"
            onClick={() => onOpenChange(false)}
          >
            取消
          </Button>
          <Button type="button" onClick={onApply}>
            应用
          </Button>
        </DialogFooter>
      </DialogContent>
    </Dialog>
  );
}

export default PolishDialog;
