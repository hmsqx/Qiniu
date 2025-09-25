from fastapi import FastAPI, HTTPException
from typing import List
from datetime import datetime
import os
from pydantic import BaseModel
from typing import Optional, List

import sys
from pathlib import Path

# 获取当前文件（main.py）的绝对路径
current_file = Path(__file__).resolve()
project_root = current_file.parent.parent.parent  # 核心：调整到正确的根目录

# 将项目根目录添加到 Python 搜索路径
if str(project_root) not in sys.path:
    sys.path.append(str(project_root))

# 改用绝对导入（基于项目根目录）
from app.txt_pro.models.qwen import QwenLLMClient, logger
from app.txt_pro.models.txt_optimizer import Text3DPromptOptimizer
from app.txt_pro.utils import load_env
env = load_env(os.path.join(os.path.dirname(__file__), "api.env"))

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





app = FastAPI(
    title="创意模式",
    description="文生3D提示词优化API",
    version="1.0.0"
)



# 全局变量：初始化服务
qwen_client = None
text_3d_optimizer = None

@app.on_event("startup")
async def startup_event():
    """应用启动时初始化服务"""
    global qwen_client, text_3d_optimizer
    
    try:
        # 初始化千问客户端
        qwen_client = QwenLLMClient(model="qwen-plus", env=env)
        
        # 初始化3D提示词优化器
        text_3d_optimizer = Text3DPromptOptimizer(qwen_client)
        
        logger.info("文生3D提示词优化服务启动成功！")
        logger.info(f"当前使用模型: {qwen_client.get_model_info()}")
        
    except Exception as e:
        logger.error(f"服务启动失败: {str(e)}")
        raise

@app.get("/")
async def root():
    """根路径，返回API信息"""
    model_info = qwen_client.get_model_info() if qwen_client else {"model": "未初始化"}
    
    return {
        "message": "文生3D提示词优化API服务",
        "version": "2.0.0",
        "status": "running",
        "description": "基于阿里云千问大模型的专业3D提示词优化服务",
        "model_info": model_info,
        "capabilities": [
            "空白输入智能补全",
            "简单描述专业增强", 
            "多风格适配优化",
            "技术参数自动添加"
        ]
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
            suggestions=optimization_result["suggestions"],
            technical_tags=optimization_result["technical_tags"],
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


if __name__ == "__main__":
    import uvicorn
    
    # 确保环境变量设置提醒
    if not os.getenv("DASHSCOPE_API_KEY"):
        print("警告: 请设置 DASHSCOPE_API_KEY 环境变量")
        print("   export DASHSCOPE_API_KEY=your-api-key")
    
    print("启动文生3D提示词优化服务...")
    uvicorn.run(
        "main:app",
        host="0.0.0.0",
        port=8090,
        reload=True,
        log_level="info"
    )
