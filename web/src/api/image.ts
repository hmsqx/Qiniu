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

// ===== Pro/Admin: 多变体图片润色 =====
export interface VipOptimizeParams {
  image_base64: string; // 输入图片 base64（不含 data: 前缀）
  num_variants?: number; // 生成变体数量，默认 1
}

export interface VipOptimizeImageItem {
  variant_number: number;
  image_base64: string; // 输出图片 base64（不含 data: 前缀）
  success: boolean;
  error?: string | null;
}

export interface VipOptimizeResponse {
  success: boolean;
  message?: string;
  images: VipOptimizeImageItem[];
  total_generated?: number;
}

/**
 * Pro 专享：一次生成多张优化后的图片变体。
 * POST /pro_pic/vip_optimize
 */
export async function vipOptimizeImages(
  params: VipOptimizeParams
): Promise<VipOptimizeResponse> {
  const res = await post<VipOptimizeResponse>("/pro_pic/vip_optimize", params);
  return res as VipOptimizeResponse;
}
