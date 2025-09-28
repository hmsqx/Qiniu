from pydantic import BaseModel
from typing import Optional


class Text3DOptimizationRequest(BaseModel):
    text: str
    style: Optional[str] = "realistic"
    detail_level: Optional[str] = "medium"
    scene_type: Optional[str] = "object"
    max_tokens: Optional[int] = 800
    temperature: Optional[float] = 0.9


class Text3DOptimizationResponse(BaseModel):
    original_text: str
    optimized_prompt: str
    enhanced_prompt: str
    style: str
    timestamp: str
    success: bool
