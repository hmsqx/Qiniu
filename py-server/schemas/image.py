from pydantic import BaseModel
from typing import Optional, Dict
from core.settings import get_settings


_settings = get_settings()


class ImageRequest(BaseModel):
    image: str
    threshold: Optional[float] = _settings.QUALITY_THRESHOLD
    max_iterations: Optional[int] = _settings.MAX_ENHANCEMENT_ITERATIONS


class QualityResponse(BaseModel):
    needs_enhancement: bool
    enhanced_image: Optional[str] = None
    overall_score: float
    detailed_scores: Dict[str, float]
    enhancement_iterations: int
    success: bool
    message: str
