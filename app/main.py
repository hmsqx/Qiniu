from fastapi import FastAPI, HTTPException
from datetime import datetime
import os
from contextlib import asynccontextmanager
import sys
from typing import Dict

# 将项目根目录添加到 Python 搜索路径
project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if str(project_root) not in sys.path:
    sys.path.append(str(project_root))

from app.body import Text3DOptimizationRequest, Text3DOptimizationResponse, \
    ImageRequest, QualityResponse, ImageOptimizeRequest, ImageOptimizeResponse

from app.txt_pro.models.qwen import QwenLLMClient, logger
from app.txt_pro.models.txt_optimizer import Text3DPromptOptimizer
from app.txt_pro.utils import load_env

from app.img_pro.models.quality_assessor import ImageQualityAssessor
from app.img_pro.models.image_enhancer import ImageEnhancer
from app.img_pro.utils.image_utils import base64_to_image, image_to_base64, resize_image
from app.img_pro.config import settings
from app.img_pro.models.qwen_imge_editor import Qwen3DOptimizer
env = load_env(os.path.join(os.path.dirname(__file__), "api.env"))



# 全局变量：初始化服务
qwen_client = None
text_3d_optimizer = None
quality_assessor = None
image_enhancer = None
optimizer = None

@asynccontextmanager
async def lifespan(app: FastAPI):
    """应用启动时初始化服务"""
    global qwen_client, text_3d_optimizer,quality_assessor, image_enhancer
    global optimizer
    try:
        
        qwen_client = QwenLLMClient(model="qwen-plus", env=env)  # 初始化千问客户端
        text_3d_optimizer = Text3DPromptOptimizer(qwen_client)  # 初始化3D提示词优化器
        logger.info("文生3D提示词优化服务启动成功！")
        logger.info(f"当前使用模型: {qwen_client.get_model_info()}")
        quality_assessor = ImageQualityAssessor()
        image_enhancer = ImageEnhancer()
        logger.info("图片修复优化服务启动成功！")
        optimizer = Qwen3DOptimizer(env["DASHSCOPE_API_KEY"])
        logger.info("vip图片修复服务启动成功！")
        
    except Exception as e:
        logger.error(f"服务启动失败: {str(e)}")
        raise
    yield  # yield之后是服务运行中，yield之前是startup，之后是shutdown



app = FastAPI(
    title="增强模式",
    description="输入增强优化API",
    version="5.0.0",
    lifespan=lifespan
)


@app.get("/")
async def root():
    """根路径，返回API信息"""
    return {
        "message": "增强优化API服务",
        "version": "5.0.0",
    }

@app.post("/pro_txt/optimize-3d-prompt", response_model=Text3DOptimizationResponse)
async def optimize_3d_prompt(request: Text3DOptimizationRequest):
    """
    3D提示词优化接口 - 专为文生3D场景设计
    
    Args:
        request: 3D提示词优化请求
        
    Returns:
        Text3DOptimizationResponse: 优化结果，包含多层次的提示词
    """
    try:
        if not text_3d_optimizer:
            raise HTTPException(status_code=503, detail="服务未正确初始化")
        
        logger.info(f"收到3D提示词优化请求: '{request.text}' - {request.style}风格 - {request.scene_type}场景")
        
        # 特殊处理空输入
        original_text = request.text
        if not request.text or request.text.strip() == "":
            logger.info("检测到空输入，将自动生成创意描述")
            request.text = f"{request.style}风格的{request.scene_type}"
        
        # 调用优化器
        optimization_result = await text_3d_optimizer.optimize_text_for_3d(request)
        
        # 构建响应
        response = Text3DOptimizationResponse(
            original_text=original_text,
            optimized_prompt=optimization_result["optimized_prompt"],
            enhanced_prompt=optimization_result["enhanced_prompt"],
            style=request.style,
            timestamp=datetime.now().isoformat(),
            success=True
        )
        
        logger.info(f"3D提示词优化完成，生成{len(optimization_result['optimized_prompt'].split())}词提示词")
        return response
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"3D提示词优化失败: {str(e)}")
        raise HTTPException(status_code=500, detail=f"优化失败: {str(e)}")

@app.get("/pro_txt/model-info")
async def get_model_info():
    """获取当前使用的模型信息"""
    if not qwen_client:
        raise HTTPException(status_code=503, detail="服务未初始化")
    
    return qwen_client.get_model_info()

@app.get("/pro_txt/3d-styles")
async def get_3d_styles():
    """获取支持的3D风格"""
    return {
        "styles": [
            {
                "name": "realistic", 
                "description": "照片级真实感风格",
                "best_for": "产品展示、建筑可视化、写实渲染",
                "keywords": ["photorealistic", "lifelike", "natural lighting"]
            },
            {
                "name": "cartoon", 
                "description": "卡通/动画风格",
                "best_for": "角色设计、游戏资产、儿童内容",
                "keywords": ["stylized", "colorful", "cell shading"]
            },
            {
                "name": "abstract", 
                "description": "抽象艺术风格",
                "best_for": "概念设计、艺术创作、装饰用途",
                "keywords": ["geometric", "minimalist", "conceptual"]
            },
            {
                "name": "sci-fi", 
                "description": "科幻未来风格",
                "best_for": "未来场景、科技产品、游戏环境",
                "keywords": ["futuristic", "metallic", "neon lighting"]
            },
            {
                "name": "fantasy", 
                "description": "奇幻魔法风格",
                "best_for": "魔法场景、神话角色、奇幻游戏",
                "keywords": ["magical", "ethereal", "mystical"]
            },
            {
                "name": "minimalist", 
                "description": "极简主义风格",
                "best_for": "现代设计、产品展示、简洁场景",
                "keywords": ["clean lines", "simple forms", "negative space"]
            }
        ]
    }

@app.get("/pro_txt/scene-types")
async def get_scene_types():
    """获取支持的场景类型"""
    return {
        "scene_types": [
            {
                "type": "object",
                "description": "单个物体或产品",
                "examples": ["家具", "电子产品", "装饰品", "工具", "艺术品"],
                "optimization_focus": "形状细节、材质质感、产品特性"
            },
            {
                "type": "character", 
                "description": "人物或生物角色",
                "examples": ["人物角色", "动物", "机器人", "怪物", "卡通形象"],
                "optimization_focus": "外观特征、表情姿态、服装配饰"
            },
            {
                "type": "environment",
                "description": "环境场景空间", 
                "examples": ["室内空间", "自然景观", "城市场景", "虚拟世界"],
                "optimization_focus": "空间布局、环境氛围、光照效果"
            },
            {
                "type": "architecture",
                "description": "建筑结构设计",
                "examples": ["住宅建筑", "商业建筑", "桥梁", "纪念碑", "未来建筑"],
                "optimization_focus": "结构设计、建筑风格、空间比例"
            }
        ]
    }

@app.post("/pro_txt/quick-enhance")
async def quick_enhance_empty_prompt(
    style: str = "realistic",
    scene_type: str = "object", 
    detail_level: str = "medium"
):
    """
     快速增强接口 - 为完全没有创意想法的用户提供即时灵感
    """
    try:
        if not text_3d_optimizer:
            raise HTTPException(status_code=503, detail="服务未初始化")
        
        # 预设的创意基础描述库
        creative_bases = {
            "object": [
                "一个设计精美的装饰花瓶",
                "充满未来感的智能设备", 
                "手工制作的精致工艺品",
                "现代简约的桌面摆件"
            ],
            "character": [
                "一个友善的卡通角色",
                "神秘的幻想生物",
                "机械感十足的机器人",
                "优雅的精灵形象"
            ], 
            "environment": [
                "宁静的森林小径场景",
                "现代化的城市空间",
                "神秘的地下洞穴",
                "温馨的室内环境"
            ],
            "architecture": [
                "现代风格的住宅建筑",
                "古典欧式的城堡结构", 
                "未来派的摩天大楼",
                "传统东方的亭台楼阁"
            ]
        }
        
        import random
        base_descriptions = creative_bases.get(scene_type, creative_bases["object"])
        selected_description = random.choice(base_descriptions)
        
        request = Text3DOptimizationRequest(
            text=selected_description,
            style=style,
            detail_level=detail_level,
            scene_type=scene_type,
            temperature=1.0  # 最高创意度
        )
        
        logger.info(f"快速增强生成: {selected_description}")
        return await optimize_3d_prompt(request)
        
    except Exception as e:
        logger.error(f"快速增强失败: {str(e)}")
        raise HTTPException(status_code=500, detail=f"快速增强失败: {str(e)}")
# <<<<

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
# >>>>

# <<< vip 图片优化接口
@app.post("/pro_pic/vip_optimize", response_model=ImageOptimizeResponse)
async def optimize_image_endpoint(request: ImageOptimizeRequest):
    """图像优化接口 - 自动应用所有策略"""
    if not optimizer:
        raise HTTPException(status_code=500, detail="优化器未初始化")
    
    # 验证参数
    if not request.image_base64:
        raise HTTPException(status_code=400, detail="图像base64不能为空")
    
    if request.num_variants < 1 or request.num_variants > 10:
        raise HTTPException(status_code=400, detail="变体数量必须在1-10之间")
    
    try:
        # 执行优化
        optimized_images = await optimizer.optimize_image(
            image_base64=request.image_base64,
            num_variants=request.num_variants
        )
        
        # 统计成功数量
        successful_count = len([img for img in optimized_images if img.success])
        
        return ImageOptimizeResponse(
            success=successful_count > 0,
            message=f"已自动应用所有3D优化策略，成功生成 {successful_count}/{request.num_variants} 个优化变体",
            images=optimized_images,
            total_generated=successful_count
        )
        
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"优化失败: {str(e)}")
# <<< 

if __name__ == "__main__":
    import uvicorn
    
    # 确保环境变量设置提醒
    if not os.getenv("DASHSCOPE_API_KEY") and not env.get("DASHSCOPE_API_KEY"):
        print("警告: 请设置 DASHSCOPE_API_KEY 环境变量")
    
    print("启动文生3D提示词优化服务...")
    uvicorn.run(
        "app.main:app",
        host="0.0.0.0",
        port=8090,
        reload=True,
        log_level="info"
    )
