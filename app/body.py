from pydantic import BaseModel
from typing import Optional, List, Dict
from app.img_pro.config import settings

# 文本请求/响应数据模型
class Text3DOptimizationRequest(BaseModel):
    text: str  # 用户输入的原始文本（可能很简单或空白）
    style: Optional[str] = "realistic"  # 3D风格：realistic, cartoon, abstract, sci-fi等
    detail_level: Optional[str] = "medium"  # 细节程度：low, medium, high
    scene_type: Optional[str] = "object"  # 场景类型：object, character, environment, architecture
    max_tokens: Optional[int] = 800
    temperature: Optional[float] = 0.9  # 创意度更高


class Text3DOptimizationResponse(BaseModel):
    original_text: str  # 原始输入文本
    optimized_prompt: str  # 优化后的3D提示词
    enhanced_prompt: str  # 增强版提示词（包含技术参数）
    style: str  # 使用的风格

    timestamp: str
    success: bool

# 图片请求/响应数据模型
class ImageRequest(BaseModel):
    image: str  # base64编码的图像(必须)
    threshold: Optional[float] = settings.QUALITY_THRESHOLD  # 质量评估阈值（可选）
    max_iterations: Optional[int] = settings.MAX_ENHANCEMENT_ITERATIONS  # 最大增强迭代次数，防止超时（可选）

class QualityResponse(BaseModel):
    needs_enhancement: bool     # 是否增强了
    enhanced_image: Optional[str] = None  # 增强后的图像（base64编码）
    overall_score: float    # 质量得分（隐藏）
    detailed_scores: Dict[str, float]   # 各项质量得分（隐藏）
    enhancement_iterations: int  # 增强迭代次数 （隐藏）
    success: bool  # 是否成功
    message: str # 额外信息（预留）


# 图片vip优化请求/响应数据模型
class ImageOptimizeRequest(BaseModel):
    """图像优化请求模型"""
    image_base64: str
    num_variants: int = 1

class OptimizedImage(BaseModel):
    """优化后的图像"""
    variant_number: int
    image_base64: str
    success: bool
    error: Optional[str] = None

class ImageOptimizeResponse(BaseModel):
    """图像优化响应模型"""
    success: bool
    message: str
    images: List[OptimizedImage]
    total_generated: int