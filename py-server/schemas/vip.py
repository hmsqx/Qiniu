from pydantic import BaseModel
from typing import Optional, List


class ImageOptimizeRequest(BaseModel):
    image_base64: str
    num_variants: int = 1


class OptimizedImage(BaseModel):
    variant_number: int
    image_base64: str
    success: bool
    error: Optional[str] = None


class ImageOptimizeResponse(BaseModel):
    success: bool
    message: str
    images: List[OptimizedImage]
    total_generated: int
