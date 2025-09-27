import { useState } from "react";
import { Dialog, DialogContent, DialogHeader, DialogTitle, DialogDescription } from "@/components/ui/dialog";
import { Button } from "@/components/ui/button";
import PricingCard from "@/components/PricingCard";
import { useAuth } from "@/context/AuthContext";
import { incrTokenCount } from "@/api/recharge";
import { useToast } from "@/components/ui/use-toast";

export type RechargeModalProps = {
  open: boolean;
  onOpenChange: (open: boolean) => void;
};

const FREE_PLAN = {
  title: "免费版",
  price: "¥0",
  description: "适合刚起步的创作者和爱好者",
  features: ["每月 10 次生成", "标准质量导出", "访问核心模型", "社区支持"],
};

const PRO_PLAN = {
  title: "专业版",
  price: "¥99",
  priceUnit: "/月",
  description: "适合专业设计师和高频用户",
  features: [
    "每月 200 次生成",
    "高分辨率导出",
    "访问所有高级模型",
    "优先队列生成",
    "商业使用许可",
    "邮件支持",
  ],
  popular: true,
};

export default function RechargeModal({ open, onOpenChange }: RechargeModalProps) {
  const { user, openLoginModal, refreshUser } = useAuth();
  const [selected, setSelected] = useState<"free" | "pro">("free");
  const PRO_DELTA = 100;
  const [loading, setLoading] = useState(false);
  const { toast, updateToast } = useToast();

  const handleRecharge = async () => {
    if (!user) {
      onOpenChange(false);
      openLoginModal();
      return;
    }
    if (selected === "free") {
      // 免费版无需充值，直接关闭
      onOpenChange(false);
      return;
    }
    try {
      const id = toast({ id: "recharge", title: "处理中", description: "正在充值，请稍候…", variant: "loading" });
      setLoading(true);
  await incrTokenCount(PRO_DELTA);
      await refreshUser();
      updateToast(id, { title: "充值成功", description: "已更新剩余次数。", variant: "success" });
      onOpenChange(false);
    } catch (e: any) {
      toast({ title: "充值失败", description: e?.message || "请稍后重试", variant: "error" });
    } finally {
      setLoading(false);
    }
  };

  return (
    <Dialog open={open} onOpenChange={onOpenChange}>
      <DialogContent className="sm:max-w-2xl">
        <DialogHeader>
          <DialogTitle>选择套餐</DialogTitle>
          <DialogDescription>选择适合你的使用方案。专业版开通后一次性到账 100 次生成额度。</DialogDescription>
        </DialogHeader>
        <div className="grid grid-cols-1 sm:grid-cols-2 gap-3">
          <PricingCard
            {...FREE_PLAN}
            selected={selected === "free"}
            onClick={() => setSelected("free")}
          />
          <PricingCard
            {...PRO_PLAN}
            selected={selected === "pro"}
            onClick={() => setSelected("pro")}
          />
        </div>
        <div className="mt-2 text-xs sm:text-sm text-muted-foreground">
          {selected === "free" ? (
            <span>免费版无需充值，包含每月基础额度。</span>
          ) : (
            <span>专业版一次性增加 {PRO_DELTA} 次额度，立刻生效。</span>
          )}
        </div>

        <div className="mt-4 flex justify-end gap-2">
          <Button variant="ghost" onClick={() => onOpenChange(false)} disabled={loading}>
            取消
          </Button>
          <Button onClick={handleRecharge} disabled={loading}>
            {selected === "free" ? "使用免费版" : loading ? "开通中…" : `开通专业版（+${PRO_DELTA} 次）`}
          </Button>
        </div>
      </DialogContent>
    </Dialog>
  );
}
