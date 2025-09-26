from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from typing import Dict, Optional
import logging

from config import settings
from models.quality_assessor import ImageQualityAssessor
from models.image_enhancer import ImageEnhancer
from utils.image_utils import base64_to_image, image_to_base64, resize_image

# 配置日志
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

app = FastAPI(
    title="Image Quality Assessment and Enhancement API",
    description="图像质量评估与优化服务",
    version="1.0.0"
)

# 配置CORS
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# 初始化模型
quality_assessor = ImageQualityAssessor()
image_enhancer = ImageEnhancer()

# 请求/响应字段
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

@app.get("/")
async def root():
    """健康检查接口"""
    return {"message": "Image Quality Assessment and Enhancement Service is running"}

@app.post("/pro_pic/assess_and_enhance", response_model=QualityResponse)
async def assess_and_enhance_image(request: ImageRequest):
    """
    图像质量评估与优化主接口
    """
    try:
        # 解码图像
        image = base64_to_image(request.image)
        
        # 调整图像大小
        image = resize_image(image, settings.MAX_IMAGE_SIZE)
        
        # 初始质量评估
        quality_scores = quality_assessor.assess_quality(image)
        overall_score = quality_scores['overall']
        
        logger.info(f"Initial quality score: {overall_score:.3f}")
        
        enhanced_image = None
        iterations = 0
        current_image = image.copy()
        
        # 如果质量不达标，进行优化
        if overall_score < request.threshold:
            logger.info(f"Quality below threshold {request.threshold}, starting enhancement...")
            
            for i in range(request.max_iterations):
                iterations = i + 1
                
                # 图像增强
                current_image = image_enhancer.enhance_image(current_image, quality_scores)
                
                # 重新评估质量
                new_quality_scores = quality_assessor.assess_quality(current_image)
                new_overall_score = new_quality_scores['overall']
                
                logger.info(f"Enhancement iteration {iterations}: score {new_overall_score:.3f}")
                
                # 如果质量达标或没有显著提升，停止优化
                if new_overall_score >= request.threshold or \
                   (new_overall_score - overall_score) < 0.05:
                    quality_scores = new_quality_scores
                    overall_score = new_overall_score
                    break
                
                quality_scores = new_quality_scores
                overall_score = new_overall_score
            
            # 转换增强后的图像为base64
            enhanced_image = image_to_base64(current_image)
        
        return QualityResponse(
            overall_score=overall_score,
            detailed_scores=quality_scores,
            needs_enhancement=overall_score < request.threshold,
            enhanced_image=enhanced_image,
            enhancement_iterations=iterations,
            success=True,
            message=f"Processing completed. Final score: {overall_score:.3f}"
        )
        
    except Exception as e:
        logger.error(f"Error processing image: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Internal server error: {str(e)}")

@app.post("/pro_pic/assess_only", response_model=Dict[str, float])
async def assess_image_quality(request: ImageRequest):
    """
    仅进行图像质量评估
    """
    try:
        # 解码图像
        image = base64_to_image(request.image)
        
        # 调整图像大小
        image = resize_image(image, settings.MAX_IMAGE_SIZE)
        
        # 质量评估
        quality_scores = quality_assessor.assess_quality(image)
        
        return quality_scores
        
    except Exception as e:

        logger.error(f"Error assessing image quality: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Internal server error: {str(e)}")

if __name__ == "__main__":
    import uvicorn
    uvicorn.run("main_pic:app", host="0.0.0.0", port=8091)