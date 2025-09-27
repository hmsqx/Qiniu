import { post } from "@/utils/request2";

export type AssessAndEnhanceParams = {
  image: string; // base64 字符串（不含 data: 前缀）
  threshold?: number; // 可选的质量阈值
  max_iterations?: number; // 可选的最大增强迭代次数
};

export type AssessAndEnhanceResponse = {
  needs_enhancement: boolean;
  enhanced_image?: string; // base64 编码的增强后图像（不含 data: 前缀）
  overall_score?: number;
  detailed_scores?: Record<string, number>;
  enhancement_iterations?: number;
  success: boolean;
  message?: string;
};

/**
 * 评估图像质量并可选择性增强。
 * POST /pro_pic/assess_and_enhance
 */
export async function assessAndEnhanceImage(
  params: AssessAndEnhanceParams
): Promise<AssessAndEnhanceResponse> {
  const res = await post<AssessAndEnhanceResponse>(
    "/pro_pic/assess_and_enhance",
    params
  );
  return res as AssessAndEnhanceResponse;
}

export default {
  assessAndEnhanceImage,
};
