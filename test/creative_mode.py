from fastapi import FastAPI, HTTPException
from typing import List
from datetime import datetime
import re
import os
from llm_client import QwenLLMClient, logger
from uitils import Text3DOptimizationRequest, Text3DOptimizationResponse


app = FastAPI(
    title="创意模式",
    description="文生3D提示词优化API",
    version="1.0.0"
)


class Text3DPromptOptimizer:
    """专门用于文生3D的提示词优化器"""
    
    def __init__(self, llm_client: QwenLLMClient):
        """
        初始化3D提示词优化器
        
        Args:
            llm_client: 千问LLM客户端实例
        """
        self.llm_client = llm_client
        
        # 3D相关的技术术语库
        self.technical_terms = {
            "lighting": ["volumetric lighting", "rim lighting", "ambient occlusion", 
                        "global illumination", "HDRI lighting", "soft shadows"],
            "materials": ["PBR materials", "metallic", "roughness", "subsurface scattering", 
                         "normal mapping", "bump mapping"],
            "quality": ["8K resolution", "highly detailed", "sharp focus", 
                       "professional 3D render", "ray tracing", "anti-aliasing"],
            "camera": ["cinematic angle", "depth of field", "bokeh effect", 
                      "wide angle", "close-up macro", "isometric view"],
            "style": ["photorealistic", "stylized", "low poly", "high poly", 
                     "voxel art", "clay render"]
        }
        
        # 风格对应的关键词
        self.style_keywords = {
            "realistic": ["photorealistic", "lifelike", "natural lighting", "detailed textures", "accurate proportions"],
            "cartoon": ["stylized", "colorful", "smooth surfaces", "exaggerated features", "cell shading"],
            "abstract": ["geometric", "minimalist", "artistic interpretation", "conceptual design", "non-representational"],
            "sci-fi": ["futuristic", "metallic surfaces", "neon lighting", "high-tech", "cyberpunk aesthetic"],
            "fantasy": ["magical", "ethereal", "mystical atmosphere", "enchanted", "otherworldly"],
            "minimalist": ["clean lines", "simple forms", "monochromatic", "negative space", "geometric shapes"]
        }
        
        logger.info("3D提示词优化器初始化完成")
    
    async def optimize_text_for_3d(self, request: Text3DOptimizationRequest) -> dict:
        """
        专门为文生3D优化提示词
        
        Args:
            request: 3D文本优化请求
            
        Returns:
            dict: 包含优化结果的字典
        """
        try:
            # 分析原始文本，判断是否需要大幅增强
            text_analysis = self._analyze_input_text(request.text)
            logger.info(f"文本分析结果: {text_analysis}")
            
            # 构建专门的3D提示词系统提示
            system_prompt = self._build_3d_system_prompt(request)
            user_prompt = self._build_3d_user_prompt(request, text_analysis)
            
            # 调用千问模型生成优化后的提示词
            optimized_result = await self.llm_client.chat_completion(
                system_prompt=system_prompt,
                user_prompt=user_prompt,
                max_tokens=request.max_tokens,
                temperature=request.temperature
            )
            
            # 后处理：添加技术增强
            enhanced_prompt = self._enhance_with_technical_terms(
                optimized_result, request.style, request.detail_level
            )
            
            # 生成建议和技术标签
            suggestions = self._generate_suggestions(request, text_analysis)
            technical_tags = self._extract_technical_tags(enhanced_prompt)
            
            return {
                "optimized_prompt": optimized_result,
                "enhanced_prompt": enhanced_prompt,
                "suggestions": suggestions,
                "technical_tags": technical_tags
            }
            
        except Exception as e:
            logger.error(f"3D提示词优化失败: {str(e)}")
            raise HTTPException(status_code=500, detail=f"提示词优化失败: {str(e)}")
    
    def _analyze_input_text(self, text: str) -> dict:
        """分析输入文本的复杂度和内容类型"""
        """"
            is_empty：判断文本是否为空
            is_simple：判断文本是否简单（长度≤10 或包含简单词汇）
            has_adjectives：判断是否包含特定形容词
            has_technical_terms：判断是否包含技术术语
            word_count：文本长度
            needs_enhancement：判断是否需要增强处理
            language：判断文本语言（中文 / 英文）
        """
        text = text.strip()
        
        # 中文简单词汇检测
        simple_chinese = ["生成", "创建", "制作", "做一个", "弄一个", "来一个"]
        
        analysis = {
            "is_empty": len(text) == 0,
            "is_simple": len(text) <= 10 or any(word in text for word in simple_chinese),
            "has_adjectives": bool(re.search(r'(美丽|漂亮|酷|好看|精美|华丽)', text)),
            "has_technical_terms": any(term in text.lower() for category in self.technical_terms.values() for term in category),
            "word_count": len(text),
            "needs_enhancement": len(text) <= 15 or any(word in text for word in simple_chinese),
            "language": "chinese" if re.search(r'[\u4e00-\u9fff]', text) else "english"
        }
        
        return analysis
    
    def _build_3d_system_prompt(self, request: Text3DOptimizationRequest) -> str:
        """构建专门用于3D生成的系统提示词"""
        return f"""你是一个专业的3D视觉艺术指导和提示词专家，专门为AI文生3D模型优化提示词。
核心任务：
将用户简单或空白的描述转换为详细、专业的3D生成提示词，确保AI能够生成高质量的3D模型。

优化策略：
1. 风格适配：针对{request.style}风格，添加相应的视觉特征描述
2. 场景定位：围绕{request.scene_type}类型，强化空间构图和比例关系
3. 细节层次：按照{request.detail_level}级别控制描述的精细程度
4. 技术规范：融入专业3D建模术语（光照、材质、渲染等）
5. 可视化导向：确保每个描述都具体可视化，避免抽象概念

输出规范：
- 使用中文输出，便于用户理解和后续编辑
- 结构：主体描述 + 材质纹理 + 光照环境 + 技术参数
- 长度：80-120个汉字
- 语言风格：专业而具体，避免模糊表述
- 如有英文专业术语，在括号内标注中文解释

特别注意：
- 对于空白或极简输入，要创造性地补充完整的场景描述
- 确保生成的提示词既专业又易于理解
- 保持中文表达的自然性和流畅度"""

    def _build_3d_user_prompt(self, request: Text3DOptimizationRequest, analysis: dict) -> str:
        """构建用户提示词"""
        
        base_prompt = f"""请优化以下3D生成提示词：
【原始输入】："{request.text}"
【目标风格】：{request.style}
【场景类型】：{request.scene_type}  
【细节级别】：{request.detail_level}

"""
        
        # 根据输入文本的复杂度给出不同的处理指导
        if analysis["is_empty"]:
            base_prompt += f"""【处理指导】：输入为空，请基于{request.scene_type}和{request.style}风格，创造一个完整且富有创意的3D场景描述。要包含具体的外观、材质、光照和构图细节。"""
        elif analysis["is_simple"] or analysis["needs_enhancement"]:
            base_prompt += f"""【处理指导】：输入较为简单，请大幅扩展细节描述。需要添加：外观特征、材质质感、环境光照、构图角度、技术参数等专业3D建模要素。"""
        else:
            base_prompt += f"""【处理指导】：在保持原意基础上，增加专业3D技术描述和视觉细节，确保提示词能指导AI生成高质量3D模型。"""
        
        base_prompt += """

【输出要求】：
请直接输出优化后的中文3D提示词，不需要额外解释。格式要规范、专业，适合3D生成工具使用。

优化后的3D提示词："""
        
        return base_prompt
    
    def _enhance_with_technical_terms(self, base_prompt: str, style: str, detail_level: str) -> str:
        """使用技术术语增强提示词"""
        enhanced = base_prompt.rstrip('，。')
        

        # 中文风格关键词
        chinese_style_keywords = {
            "realistic": ["写实风格", "逼真效果", "自然光照", "细致纹理", "精确比例"],
            "cartoon": ["卡通风格", "色彩鲜艳", "光滑表面", "夸张特征", "描边渲染"],
            "abstract": ["几何抽象", "极简主义", "艺术化诠释", "概念设计", "非具象表现"],
            "sci-fi": ["未来科技", "金属表面", "霓虹照明", "高科技感", "赛博朋克美学"],
            "fantasy": ["魔幻风格", "空灵质感", "神秘氛围", "魅惑效果", "超凡脱俗"],
            "minimalist": ["简洁线条", "简单形态", "单色调", "留白空间", "几何造型"]
        }
        
        # 根据风格添加对应的中文术语
        if style in chinese_style_keywords:
            style_terms = chinese_style_keywords[style][:2]
            enhanced += f"，{', '.join(style_terms)}"
        
        # 根据细节级别添加技术参数
        if detail_level == "high":
            tech_terms = ["8K分辨率", "高度细致", "专业3D渲染", "体积光照", "PBR材质"]
            enhanced += f"，{', '.join(tech_terms)}"
        elif detail_level == "medium":
            tech_terms = ["精细建模", "良好光照", "清晰几何"]
            enhanced += f"，{', '.join(tech_terms)}"
        else:  # low detail
            tech_terms = ["简洁设计", "优化建模"]
            enhanced += f"，{', '.join(tech_terms)}"
        
        # 添加通用的质量提升词汇
        quality_terms = ["清晰对焦", "专业品质"]
        enhanced += f"，{', '.join(quality_terms)}"
        
        return enhanced

    def _generate_suggestions(self, request: Text3DOptimizationRequest, analysis: dict) -> List[str]:
        """生成额外建议"""
        """基于输入文本分析结果，提供个性化建议"""
        suggestions = []
        
        if analysis["is_empty"] or analysis["is_simple"]:
            suggestions.append("建议添加更多具体的视觉细节，如颜色、质感、形状等")
            
        if request.style == "realistic":
            suggestions.append("对于写实风格，可以增加具体的材质描述（如金属、木材、皮革等）")
            
        if request.scene_type == "character":
            suggestions.append("角色类型可以详细描述表情、姿态、服装和配饰")
            
        if request.scene_type == "environment":
            suggestions.append("环境场景建议描述天气、时间、氛围和空间布局")
        
        if request.scene_type == "architecture":
            suggestions.append("建筑类型可以指定具体风格（现代、古典、未来派等）")
            
        if request.detail_level == "high":
            suggestions.append("高细节模式下，可以尝试添加特写镜头或局部细节描述")
        
        # 通用建议
        suggestions.extend([
            "尝试不同的相机角度（正面、侧面、俯视等）获得更好效果",
            "调整光照设置（自然光、人工光、环境光）可显著改善渲染质量",
            "考虑添加环境背景来增强整体视觉效果"
        ])
        
        return suggestions[:5]  # 限制建议数量
    
    def _extract_technical_tags(self, prompt: str) -> List[str]:
        """从提示词中提取技术标签"""
        tags = []
        prompt_lower = prompt.lower()
        
        # 检查技术术语
        for category, terms in self.technical_terms.items():
            for term in terms:
                if term.lower() in prompt_lower:
                    tags.append(term)
        
        # 添加风格标签
        for style, keywords in self.style_keywords.items():
            if any(keyword.lower() in prompt_lower for keyword in keywords):
                tags.append(f"{style}风格")
                break
        
        # 质量标签
        quality_indicators = {
            "高分辨率": ["8k", "4k", "high resolution", "detailed"],
            "专业渲染": ["professional", "ray tracing", "render"],
            "优质光照": ["lighting", "illumination", "shadows"]
        }
        
        for tag, indicators in quality_indicators.items():
            if any(indicator in prompt_lower for indicator in indicators):
                tags.append(tag)
        
        return list(set(tags))  # 去重并返回

# 全局变量：初始化服务
qwen_client = None
text_3d_optimizer = None

@app.on_event("startup")
async def startup_event():
    """应用启动时初始化服务"""
    global qwen_client, text_3d_optimizer
    
    try:
        # 初始化千问客户端
        qwen_client = QwenLLMClient(model="qwen-plus")
        
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

@app.post("/optimize-3d-prompt", response_model=Text3DOptimizationResponse)
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

@app.get("/model-info")
async def get_model_info():
    """获取当前使用的模型信息"""
    if not qwen_client:
        raise HTTPException(status_code=503, detail="服务未初始化")
    
    return qwen_client.get_model_info()

@app.get("/3d-styles")
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

@app.get("/scene-types")
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

@app.post("/quick-enhance")
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
        
        logger.info(f"🎲 快速增强生成: {selected_description}")
        return await optimize_3d_prompt(request)
        
    except Exception as e:
        logger.error(f"❌ 快速增强失败: {str(e)}")
        raise HTTPException(status_code=500, detail=f"快速增强失败: {str(e)}")


if __name__ == "__main__":
    import uvicorn
    
    # 确保环境变量设置提醒
    if not os.getenv("DASHSCOPE_API_KEY"):
        print("警告: 请设置 DASHSCOPE_API_KEY 环境变量")
        print("   export DASHSCOPE_API_KEY=your-api-key")
    
    print("启动文生3D提示词优化服务...")
    uvicorn.run(
        "creative_mode:app",
        host="0.0.0.0",
        port=8000,
        reload=True,
        log_level="info"
    )
