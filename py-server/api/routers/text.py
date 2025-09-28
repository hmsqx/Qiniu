from __future__ import annotations

from datetime import datetime
import random
from fastapi import APIRouter, Depends, HTTPException

from schemas.text import Text3DOptimizationRequest, Text3DOptimizationResponse
from core.deps import get_services
from core.logging import logger


router = APIRouter(prefix="/pro_txt", tags=["text"])


@router.post("/optimize-3d-prompt", response_model=Text3DOptimizationResponse)
async def optimize_3d_prompt(request: Text3DOptimizationRequest, services=Depends(get_services)):
    text_3d_optimizer = services.text_3d_optimizer
    if not text_3d_optimizer:
        raise HTTPException(status_code=503, detail="服务未正确初始化")

    logger.info(
        f"收到3D提示词优化请求: '{request.text}' - {request.style}风格 - {request.scene_type}场景"
    )

    original_text = request.text
    if not request.text or request.text.strip() == "":
        logger.info("检测到空输入，将自动生成创意描述")
        request.text = f"{request.style}风格的{request.scene_type}"

    optimization_result = await text_3d_optimizer.optimize_text_for_3d(request)

    response = Text3DOptimizationResponse(
        original_text=original_text,
        optimized_prompt=optimization_result["optimized_prompt"],
        enhanced_prompt=optimization_result["enhanced_prompt"],
        style=request.style,
        timestamp=datetime.now().isoformat(),
        success=True,
    )

    logger.info(
        f"3D提示词优化完成，生成{len(optimization_result['optimized_prompt'].split())}词提示词"
    )
    return response


@router.get("/model-info")
async def get_model_info(services=Depends(get_services)):
    qwen_client = services.qwen_client
    if not qwen_client:
        raise HTTPException(status_code=503, detail="服务未初始化")
    return qwen_client.get_model_info()


@router.get("/3d-styles")
async def get_3d_styles():
    return {
        "styles": [
            {
                "name": "realistic",
                "description": "照片级真实感风格",
                "best_for": "产品展示、建筑可视化、写实渲染",
                "keywords": ["photorealistic", "lifelike", "natural lighting"],
            },
            {
                "name": "cartoon",
                "description": "卡通/动画风格",
                "best_for": "角色设计、游戏资产、儿童内容",
                "keywords": ["stylized", "colorful", "cell shading"],
            },
            {
                "name": "abstract",
                "description": "抽象艺术风格",
                "best_for": "概念设计、艺术创作、装饰用途",
                "keywords": ["geometric", "minimalist", "conceptual"],
            },
            {
                "name": "sci-fi",
                "description": "科幻未来风格",
                "best_for": "未来场景、科技产品、游戏环境",
                "keywords": ["futuristic", "metallic", "neon lighting"],
            },
            {
                "name": "fantasy",
                "description": "奇幻魔法风格",
                "best_for": "魔法场景、神话角色、奇幻游戏",
                "keywords": ["magical", "ethereal", "mystical"],
            },
            {
                "name": "minimalist",
                "description": "极简主义风格",
                "best_for": "现代设计、产品展示、简洁场景",
                "keywords": ["clean lines", "simple forms", "negative space"],
            },
        ]
    }


@router.get("/scene-types")
async def get_scene_types():
    return {
        "scene_types": [
            {
                "type": "object",
                "description": "单个物体或产品",
                "examples": ["家具", "电子产品", "装饰品", "工具", "艺术品"],
                "optimization_focus": "形状细节、材质质感、产品特性",
            },
            {
                "type": "character",
                "description": "人物或生物角色",
                "examples": ["人物角色", "动物", "机器人", "怪物", "卡通形象"],
                "optimization_focus": "外观特征、表情姿态、服装配饰",
            },
            {
                "type": "environment",
                "description": "环境场景空间",
                "examples": ["室内空间", "自然景观", "城市场景", "虚拟世界"],
                "optimization_focus": "空间布局、环境氛围、光照效果",
            },
            {
                "type": "architecture",
                "description": "建筑结构设计",
                "examples": ["住宅建筑", "商业建筑", "桥梁", "纪念碑", "未来建筑"],
                "optimization_focus": "结构设计、建筑风格、空间比例",
            },
        ]
    }


@router.post("/quick-enhance")
async def quick_enhance_empty_prompt(
    style: str = "realistic", scene_type: str = "object", detail_level: str = "medium", services=Depends(get_services)
):
    text_3d_optimizer = services.text_3d_optimizer
    if not text_3d_optimizer:
        raise HTTPException(status_code=503, detail="服务未初始化")

    creative_bases = {
        "object": [
            "一个设计精美的装饰花瓶",
            "充满未来感的智能设备",
            "手工制作的精致工艺品",
            "现代简约的桌面摆件",
        ],
        "character": [
            "一个友善的卡通角色",
            "神秘的幻想生物",
            "机械感十足的机器人",
            "优雅的精灵形象",
        ],
        "environment": [
            "宁静的森林小径场景",
            "现代化的城市空间",
            "神秘的地下洞穴",
            "温馨的室内环境",
        ],
        "architecture": [
            "现代风格的住宅建筑",
            "古典欧式的城堡结构",
            "未来派的摩天大楼",
            "传统东方的亭台楼阁",
        ],
    }

    base_descriptions = creative_bases.get(scene_type, creative_bases["object"])
    selected_description = random.choice(base_descriptions)

    request = Text3DOptimizationRequest(
        text=selected_description,
        style=style,
        detail_level=detail_level,
        scene_type=scene_type,
        temperature=1.0,
    )

    logger.info(f"快速增强生成: {selected_description}")
    return await optimize_3d_prompt(request, services)
