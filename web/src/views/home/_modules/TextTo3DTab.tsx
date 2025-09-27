import { useState } from "react";
import { GenerationOptions } from "./GenerationOptions";
import { useToast } from "@/components/ui/use-toast";
import { optimize3DPrompt } from "@/api/prompt";
import PromptTextarea from "./text-to-3d/PromptTextarea";
import PolishDialog from "./text-to-3d/PolishDialog";

type Props = {
  value: string;
  onChange: (v: string) => void;
  action: string;
  onActionChange: (v: string) => void;
  format: string;
  onFormatChange: (v: string) => void;
};

export const TextTo3DTab = ({
  value,
  onChange,
  action,
  onActionChange,
  format,
  onFormatChange,
}: Props) => {
  const [polishOpen, setPolishOpen] = useState(false);
  const [sourceText, setSourceText] = useState<string>(value ?? "");
  const [polishedText, setPolishedText] = useState<string>(value ?? "");
  const { toast, updateToast } = useToast();
  const [optimizing, setOptimizing] = useState(false);

  const openPolish = () => {
    setSourceText(value ?? "");
    setPolishedText(value ?? "");
    setPolishOpen(true);
  };

  const applyPolish = () => {
    onChange(polishedText);
    setPolishOpen(false);
  };

  const handleOptimize = async () => {
    const text = (sourceText || "").trim();
    if (!text) {
      toast({ title: "请先输入原文", variant: "error" });
      return;
    }
    if (optimizing) return;
    setOptimizing(true);
    const id = toast({ title: "润色中...", variant: "loading" });
    try {
      const res = await optimize3DPrompt({ text });
      setPolishedText(res.text);
      updateToast(id, { title: "润色完成", variant: "success" });
    } catch (err: any) {
      updateToast(id, {
        title: "润色失败",
        description: err?.message || "请求失败，请稍后重试",
        variant: "error",
      });
    } finally {
      setOptimizing(false);
    }
  };

  return (
    <div className="space-y-6">
      <PromptTextarea
        value={value}
        onChange={onChange}
        onOpenPolish={openPolish}
      />
      <PolishDialog
        open={polishOpen}
        onOpenChange={setPolishOpen}
        sourceText={sourceText}
        setSourceText={setSourceText}
        polishedText={polishedText}
        setPolishedText={setPolishedText}
        optimizing={optimizing}
        onOptimize={handleOptimize}
        onApply={applyPolish}
        toast={toast}
      />
      <GenerationOptions
        action={action}
        onActionChange={onActionChange}
        format={format}
        onFormatChange={onFormatChange}
      />
    </div>
  );
};
