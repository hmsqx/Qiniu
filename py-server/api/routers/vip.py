from __future__ import annotations

from fastapi import APIRouter, Depends, HTTPException

from schemas.vip import ImageOptimizeRequest, ImageOptimizeResponse
from core.deps import get_services


router = APIRouter(prefix="/pro_pic", tags=["vip"])


@router.post("/vip_optimize", response_model=ImageOptimizeResponse)
async def optimize_image_endpoint(request: ImageOptimizeRequest, services=Depends(get_services)):
    optimizer = services.optimizer
    if not optimizer:
        raise HTTPException(status_code=500, detail="优化器未初始化")

    if not request.image_base64:
        raise HTTPException(status_code=400, detail="图像base64不能为空")
    if request.num_variants < 1 or request.num_variants > 10:
        raise HTTPException(status_code=400, detail="变体数量必须在1-10之间")

    try:
        optimized_images = await optimizer.optimize_image(
            image_base64=request.image_base64, num_variants=request.num_variants
        )
        successful_count = len([img for img in optimized_images if img.success])
        return ImageOptimizeResponse(
            success=successful_count > 0,
            message=f"已自动应用所有3D优化策略，成功生成 {successful_count}/{request.num_variants} 个优化变体",
            images=optimized_images,
            total_generated=successful_count,
        )
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"优化失败: {str(e)}")
