import { ImageUploader } from "./ImageUploader";
import { GenerationOptions } from "./GenerationOptions";
import { useRef } from "react";
import ImagePreviewCard from "./image-to-3d/ImagePreviewCard";
import { useImageEnhancer } from "./image-to-3d/useImageEnhancer";

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

  return (
    <div className="space-y-6">
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

      <GenerationOptions
        action={action}
        onActionChange={onActionChange}
        format={format}
        onFormatChange={onFormatChange}
      />
    </div>
  );
};
