from pydantic import BaseModel
from typing import Optional, List
# 读取环境变量
def load_env(file_path=".env"):
    env_vars = {}
    with open(file_path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            key, value = line.split("=", 1)
            env_vars[key] = value
    return env_vars


# 请求数据模型
class Text3DOptimizationRequest(BaseModel):
    text: str  # 用户输入的原始文本（可能很简单或空白）
    style: Optional[str] = "realistic"  # 3D风格：realistic, cartoon, abstract, sci-fi等
    detail_level: Optional[str] = "medium"  # 细节程度：low, medium, high
    scene_type: Optional[str] = "object"  # 场景类型：object, character, environment, architecture
    max_tokens: Optional[int] = 800
    temperature: Optional[float] = 0.9  # 创意度更高

# 响应数据模型
class Text3DOptimizationResponse(BaseModel):
    original_text: str  # 原始输入文本
    optimized_prompt: str  # 优化后的3D提示词
    enhanced_prompt: str  # 增强版提示词（包含技术参数）
    style: str  # 使用的风格
    suggestions: List[str]  # 额外建议
    technical_tags: List[str]  # 技术标签
    timestamp: str
    success: bool