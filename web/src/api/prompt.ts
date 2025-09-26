import { post } from "@/utils/request2";

export type Optimize3DPromptParams = {
  text: string;
  style?: "realistic" | "cartoon" | "stylized" | string;
  scene_type?: "object" | "scene" | string;
  detail_level?: "low" | "medium" | "high" | string;
};

export type Optimize3DPromptResponse = {
  text: string; // enhanced prompt
  raw?: any;
};

export async function optimize3DPrompt(
  params: Optimize3DPromptParams
): Promise<Optimize3DPromptResponse> {
  const {
    text,
    style = "realistic",
    scene_type = "object",
    detail_level = "medium",
  } = params;

  try {
    const res = await post<any>("/pro_txt/optimize-3d-prompt", { text });
    const candidates = [
      res?.enhanced_prompt,
      res?.optimized_prompt,
      res?.optimized_text,
      res?.text,
      res?.data,
    ];
    let out: string | undefined;
    for (const c of candidates) {
      if (typeof c === "string" && c.trim()) {
        out = c;
        break;
      }
    }
    if (!out && candidates[4] && typeof candidates[4] !== "string") {
      out = JSON.stringify(candidates[4]);
    }
    return {
      text: out || "",
      raw: res,
    };
  } catch (e) {
    const mock = `优化后的提示：${text}（风格: ${style}，类型: ${scene_type}，细节: ${detail_level}）`;
    return { text: mock, raw: { mocked: true, error: String(e) } };
  }
}
