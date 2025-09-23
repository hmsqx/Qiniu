import { Card, CardContent } from "@/components/ui/card";
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/components/ui/tabs";
import { Button } from "@/components/ui/button";
import { Image, Wand2, Type, Loader2 } from "lucide-react";
import { useState } from "react";

import { TextTo3DTab } from "./TextTo3DTab";
import { ImageTo3DTab } from "./ImageTo3DTab";
import { useAuth } from "@/context/AuthContext";
import { useMessage } from "@/components/ui/message";
import { imageTo3D, textTo3D } from "@/api/upload";

export const GeneratorTabs = () => {
  const [prompt, setPrompt] = useState<string>("");
  const [imageBase64, setImageBase64] = useState<string | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [loading, setLoading] = useState(false);
  const [action, setAction] = useState<string>("SubmitHunyuanTo3DJob");
  const [format, setFormat] = useState<string>("OBJ");
  const { isAuthenticated, openLoginModal, user } = useAuth();
  const { show } = useMessage();
  const [activeTab, setActiveTab] = useState<"text-to-3d" | "image-to-3d">(
    "text-to-3d"
  );

  const validateAndSubmit = async () => {
    setError(null);

    const hasPrompt = !!prompt.trim();
    const hasImage = !!imageBase64;

    if (activeTab === "text-to-3d") {
      if (!hasPrompt) {
        setError("请填写 Prompt");
        return;
      }
    } else if (activeTab === "image-to-3d") {
      if (!hasImage) {
        setError("请上传图片");
        return;
      }
    }

    if (!isAuthenticated) {
      openLoginModal();
      return;
    }

    if (!user?.id) {
      setError("未获取到用户信息，请重新登录");
      return;
    }

    setLoading(true);
    try {
      if (activeTab === "text-to-3d" && hasPrompt) {
        await textTo3D({
          action,
          prompt: prompt.trim(),
          resultFormat: format,
          userId: user.id,
        });
      } else if (activeTab === "image-to-3d" && hasImage && imageBase64) {
        await imageTo3D({
          action,
          imageBase64,
          resultFormat: format,
          userId: user.id,
        });
      }

      show({ type: "success", content: "提交成功" });
    } catch (e: any) {
      const msg = e?.message || "生成失败，请稍后重试";
      setError(msg);
      show({ type: "error", content: msg });
    } finally {
      setLoading(false);
    }
  };
  return (
    <Card className="w-full max-w-md mx-auto">
      <div className="p-6">
        <Tabs
          value={activeTab}
          onValueChange={(v: any) => {
            setActiveTab(v);
            setError(null);
          }}
          className="w-full"
        >
          <TabsList className="grid w-full grid-cols-2 bg-muted p-1 h-auto rounded-lg">
            <TabsTrigger
              value="text-to-3d"
              className="flex items-center justify-center gap-2 data-[state=active]:bg-background data-[state=active]:shadow-sm data-[state=active]:text-foreground text-muted-foreground"
            >
              <Type className="h-4 w-4" />
              文生3D
            </TabsTrigger>
            <TabsTrigger
              value="image-to-3d"
              className="flex items-center justify-center gap-2 data-[state=active]:bg-background data-[state=active]:shadow-sm data-[state=active]:text-foreground text-muted-foreground"
            >
              <Image className="h-4 w-4" />
              图生3D
            </TabsTrigger>
          </TabsList>

          <CardContent className="p-0 pt-6">
            <TabsContent value="text-to-3d" className="m-0">
              <TextTo3DTab
                value={prompt}
                onChange={setPrompt}
                action={action}
                onActionChange={setAction}
                format={format}
                onFormatChange={setFormat}
              />
            </TabsContent>

            <TabsContent value="image-to-3d" className="m-0">
              <ImageTo3DTab
                imageBase64={imageBase64}
                onChangeImageBase64={(b: string | null) => setImageBase64(b)}
                action={action}
                onActionChange={setAction}
                format={format}
                onFormatChange={setFormat}
              />
            </TabsContent>

            <div className="mt-6">
              {error ? (
                <p className="text-sm text-red-600 mb-2">{error}</p>
              ) : null}
              <Button
                size="lg"
                className="w-full font-medium text-base shadow-sm hover:shadow-md transition-all duration-200 transform hover:-translate-y-0.5 active:translate-y-0"
                onClick={validateAndSubmit}
                disabled={loading}
              >
                {loading ? (
                  <Loader2 className="mr-2 h-4 w-4 animate-spin" />
                ) : (
                  <Wand2 className="mr-2 h-4 w-4" />
                )}
                {loading ? "生成中..." : "立即生成"}
              </Button>
            </div>
          </CardContent>
        </Tabs>
      </div>
    </Card>
  );
};

export default GeneratorTabs;
