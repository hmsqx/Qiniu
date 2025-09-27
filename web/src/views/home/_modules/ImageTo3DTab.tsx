import { ImageUploader } from "./ImageUploader";
import { GenerationOptions } from "./GenerationOptions";
import { useRef } from "react";
import ImagePreviewCard from "./image-to-3d/ImagePreviewCard";
import { useImageEnhancer } from "./image-to-3d/useImageEnhancer";
import VipPolishPanel from "./image-to-3d/VipPolishPanel";
import { useAuth } from "@/context/AuthContext";
// import { Button } from "@/components/ui/button";

type Props = {
  imageBase64: string | null;
  onChangeImageBase64: (b: string | null) => void;
  action: string;
  onActionChange: (v: string) => void;
  format: string;
  onFormatChange: (v: string) => void;
};

export const ImageTo3DTab = ({
  imageBase64,
  onChangeImageBase64,
  action,
  onActionChange,
  format,
  onFormatChange,
}: Props) => {
  const openFileDialogRef = useRef<null | (() => void)>(null);
  const { optimizing, optimize } = useImageEnhancer();
  const { user } = useAuth();
  const role = (user?.role || "").toLowerCase();
  const canVip = role === "admin" || role === "pro";

  return (
    <div className="space-y-6">
      {canVip ? (
        !imageBase64 ? (
          <ImageUploader
            onChange={(b) => onChangeImageBase64(b)}
            onRegisterOpen={(open) => (openFileDialogRef.current = open)}
          />
        ) : (
          <>
            <ImagePreviewCard
              base64={imageBase64}
              onRemove={() => onChangeImageBase64(null)}
              onReselect={() => openFileDialogRef.current?.()}
              onOptimize={async () => {
                if (!imageBase64) return;
                const result = await optimize(imageBase64);
                if (result.updated && result.base64) {
                  onChangeImageBase64(result.base64);
                }
              }}
              optimizing={optimizing}
            />

            <VipPolishPanel
              base64={imageBase64}
              count={2}
              onSelect={(b64) => onChangeImageBase64(b64)}
            />
          </>
        )
      ) : (
        <>
          <ImageUploader
            onChange={(b) => onChangeImageBase64(b)}
            onRegisterOpen={(open) => (openFileDialogRef.current = open)}
          />

          {imageBase64 ? (
            <ImagePreviewCard
              base64={imageBase64}
              onRemove={() => onChangeImageBase64(null)}
              onReselect={() => openFileDialogRef.current?.()}
              onOptimize={async () => {
                if (!imageBase64) return;
                const result = await optimize(imageBase64);
                if (result.updated && result.base64) {
                  onChangeImageBase64(result.base64);
                }
              }}
              optimizing={optimizing}
            />
          ) : null}
        </>
      )}

      {/* 删除重复的 VipPolishPanel（仅保留上方的两图预览）*/}

      <GenerationOptions
        action={action}
        onActionChange={onActionChange}
        format={format}
        onFormatChange={onFormatChange}
      />
    </div>
  );
};
