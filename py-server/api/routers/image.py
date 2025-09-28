from __future__ import annotations

from typing import Dict
from fastapi import APIRouter, Depends, HTTPException

from schemas.image import ImageRequest, QualityResponse
from core.deps import get_services
from img_pro.utils.image_utils import base64_to_image, image_to_base64, resize_image
from core.settings import get_settings
from core.logging import logger


router = APIRouter(prefix="/pro_pic", tags=["image"])


@router.post("/assess_only", response_model=Dict[str, float])
async def assess_image_quality(request: ImageRequest, services=Depends(get_services)):
    try:
        settings = get_settings()
        image = base64_to_image(request.image)
        image = resize_image(image, settings.MAX_IMAGE_SIZE)
        quality_scores = services.quality_assessor.assess_quality(image)
        return quality_scores
    except Exception as e:
        logger.error(f"Error assessing image quality: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Internal server error: {str(e)}")


@router.post("/assess_and_enhance", response_model=QualityResponse)
async def assess_and_enhance_image(request: ImageRequest, services=Depends(get_services)):
    try:
        settings = get_settings()
        image = base64_to_image(request.image)
        image = resize_image(image, settings.MAX_IMAGE_SIZE)
        quality_scores = services.quality_assessor.assess_quality(image)
        overall_score = quality_scores['overall']
        logger.info(f"Initial quality score: {overall_score:.3f}")

        enhanced_image = None
        iterations = 0
        current_image = image.copy()

        if overall_score < request.threshold:
            logger.info(
                f"Quality below threshold {request.threshold}, starting enhancement..."
            )
            for i in range(request.max_iterations):
                iterations = i + 1
                current_image = services.image_enhancer.enhance_image(current_image, quality_scores)
                new_quality_scores = services.quality_assessor.assess_quality(current_image)
                new_overall_score = new_quality_scores['overall']
                logger.info(
                    f"Enhancement iteration {iterations}: score {new_overall_score:.3f}"
                )
                if new_overall_score >= request.threshold or (new_overall_score - overall_score) < 0.05:
                    quality_scores = new_quality_scores
                    overall_score = new_overall_score
                    break
                quality_scores = new_quality_scores
                overall_score = new_overall_score
            enhanced_image = image_to_base64(current_image)

        return QualityResponse(
            overall_score=overall_score,
            detailed_scores=quality_scores,
            needs_enhancement=overall_score < request.threshold,
            enhanced_image=enhanced_image,
            enhancement_iterations=iterations,
            success=True,
            message=f"Processing completed. Final score: {overall_score:.3f}",
        )
    except Exception as e:
        logger.error(f"Error processing image: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Internal server error: {str(e)}")
