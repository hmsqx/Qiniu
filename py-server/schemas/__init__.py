"""Pydantic schemas grouped by domain (text, image, vip)."""

from .text import Text3DOptimizationRequest, Text3DOptimizationResponse
from .image import ImageRequest, QualityResponse
from .vip import ImageOptimizeRequest, ImageOptimizeResponse, OptimizedImage

__all__ = [
    "Text3DOptimizationRequest",
    "Text3DOptimizationResponse",
    "ImageRequest",
    "QualityResponse",
    "ImageOptimizeRequest",
    "ImageOptimizeResponse",
    "OptimizedImage",
]
