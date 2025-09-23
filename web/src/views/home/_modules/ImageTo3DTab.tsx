import { ImageUploader } from "./ImageUploader";
import { GenerationOptions } from "./GenerationOptions";
import { Button } from "@/components/ui/button";

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
  // 校验在父组件按 Tab 处理，这里无需读取 prompt

  return (
    <div className="space-y-6">
      {/* 已改为按 Tab 校验，不再根据 prompt 禁用上传 */}

      <ImageUploader onChange={(b) => onChangeImageBase64(b)} />

      {imageBase64 ? (
        <div className="flex items-center gap-4 mt-2">
          <div className="w-24 h-24 rounded overflow-hidden border">
            <img
              src={`data:image/jpeg;base64,${imageBase64}`}
              alt="preview"
              className="w-full h-full object-cover"
            />
          </div>
          <div className="flex-1">
            <p className="text-sm text-muted-foreground">已上传图片（预览）</p>
            <div className="mt-2">
              <Button
                size="sm"
                variant="ghost"
                onClick={() => onChangeImageBase64(null)}
              >
                移除图片
              </Button>
            </div>
          </div>
        </div>
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
