from fastapi import HTTPException
import re
from app.txt_pro.models.qwen import QwenLLMClient, logger



class Text3DPromptOptimizer:
    """专门用于文生3D的提示词优化器"""
    
    def __init__(self, llm_client: QwenLLMClient):
        """
        初始化3D提示词优化器
        
        Args:
            llm_client: 千问LLM客户端实例
        """
        self.llm_client = llm_client
        
        # 注重描述性特征而非技术参数
        # 【修改2】优化风格关键词 - 强调描述性特征
        self.style_descriptors = {
            "realistic": {
                "subject_features": ["逼真外观", "自然比例", "细致入微"],
                "visual_characteristics": ["真实材质", "自然光影", "丰富细节"],
                "style_elements": ["写实风格", "照片级效果", "自然呈现"]
            },
            "cartoon": {
                "subject_features": ["可爱造型", "夸张比例", "简化线条"],
                "visual_characteristics": ["鲜艳色彩", "光滑表面", "清晰轮廓"],
                "style_elements": ["卡通风格", "动画效果", "童趣表达"]
            },
            "abstract": {
                "subject_features": ["几何形态", "抽象造型", "简化结构"],
                "visual_characteristics": ["纯色块面", "线条构成", "形状组合"],
                "style_elements": ["抽象艺术", "现代设计", "概念表达"]
            },
            "sci-fi": {
                "subject_features": ["科技造型", "流线外观", "现代设计"],
                "visual_characteristics": ["金属质感", "发光元素", "冷色调"],
                "style_elements": ["科幻风格", "未来感", "科技美学"]
            },
            "fantasy": {
                "subject_features": ["奇幻造型", "优雅形态", "梦幻外观"],
                "visual_characteristics": ["柔和光效", "神秘色彩", "空灵质感"],
                "style_elements": ["奇幻风格", "魔法氛围", "童话感"]
            },
            "minimalist": {
                "subject_features": ["简洁造型", "纯净线条", "基础形态"],
                "visual_characteristics": ["单纯色彩", "干净表面", "留白空间"],
                "style_elements": ["极简风格", "简约美学", "纯粹设计"]
            }
        }
        
        logger.info("3D提示词优化器初始化完成")
    
    async def optimize_text_for_3d(self, request) -> dict:
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
            enhanced_prompt = self._enhance_with_descriptive_features(
                optimized_result, request.style, request.detail_level
            )


            return {
                "optimized_prompt": optimized_result,
                "enhanced_prompt": enhanced_prompt,
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
        """
        text = text.strip()
        
        # 中文简单词汇检测
        descriptive_words = ["颜色", "形状", "材质", "风格", "外观", "质感", "光泽", "纹理"]
        simple_chinese = ["生成", "创建", "制作", "做一个", "弄一个", "来一个"]

        analysis = {
            "is_empty": len(text) == 0,
            "is_simple": len(text) <= 10 or any(word in text for word in simple_chinese),
            "has_adjectives": bool(re.search(r'(美丽|漂亮|酷|好看|精美|华丽)', text)),
            "has_descriptive_words": any(word in text for word in descriptive_words), 
            "word_count": len(text),
            "needs_enhancement": len(text) <= 15 or any(word in text for word in simple_chinese),
        }
        
        return analysis
    
    def _build_3d_system_prompt(self, request) -> str:
        """构建专门用于3D生成的系统提示词"""
        return f"""你是专业的3D视觉描述专家，专门将简单描述转换为详细的视觉特征描述。

核心原则：
1. 主体描述：明确描述3D模型的主要对象和基本形态
2. 特征细化：详细描述外观特征、材质质感、色彩搭配
3. 风格融合：融入{request.style}风格的典型视觉元素
4. 场景适配：结合{request.scene_type}类型的空间特点

描述结构：
- 主体对象 + 基本形态特征
- 材质质感 + 色彩描述  
- 光影氛围 + 空间构图
- 风格特色 + 视觉亮点

语言要求：
- 使用具体的视觉形容词，避免抽象概念
- 每个描述都要可视化，能够指导3D建模
- 保持中文表达自然流畅
- 长度控制在80-120字

注意事项：
- 不使用技术参数术语（如8K、ray tracing等）
- 重点描述"看起来什么样"而非"如何制作"
- 确保每个元素都有视觉可感知的描述"""

    def _build_3d_user_prompt(self, request, analysis: dict) -> str:
        """构建用户提示词"""
        
        base_prompt = f"""请优化以下3D生成提示词：
【原始输入】："{request.text}"
【目标风格】：{request.style}
【场景类型】：{request.scene_type}  
【细节级别】：{request.detail_level}

"""
        
        # 根据输入文本的复杂度给出不同的处理指导
        if analysis["is_empty"]:
            base_prompt += f"""【处理方式】：基于{request.scene_type}创造一个{request.style}风格的3D对象，完整描述其外观、材质、色彩和整体视觉效果。"""
        elif analysis["is_simple"] or analysis["needs_enhancement"]:
            base_prompt += f"""【处理方式】：大幅扩展视觉细节描述，包括：具体外观、材质质感、色彩搭配、光影效果、构图特点等。"""
        else:
            base_prompt += f"""【处理方式】：在保持原意基础上，丰富视觉描述细节，使其更适合3D建模参考。"""
        
        base_prompt += """

【输出格式】：
请直接输出优化后的中文视觉描述，按照"主体+特征+风格"的结构组织。

优化后的3D描述："""
        
        return base_prompt
    
    def _enhance_with_descriptive_features(self, base_prompt: str, style: str, detail_level: str) -> str:
        """使用技术术语增强提示词"""
        enhanced = base_prompt.rstrip('，。')
        
        # 根据风格添加描述性特征
        if style in self.style_descriptors:
            style_desc = self.style_descriptors[style]
            
            # 添加风格特征描述
            if detail_level == "high":
                features = style_desc["visual_characteristics"][:2] + style_desc["style_elements"][:1]
            elif detail_level == "medium":
                features = style_desc["visual_characteristics"][:1] + style_desc["style_elements"][:1]
            else:  # low
                features = style_desc["style_elements"][:1]
            
            if features:
                enhanced += f"，{', '.join(features)}"
        
        # 根据细节级别添加视觉描述
        if detail_level == "high":
            visual_features = ["精致细节", "丰富层次", "清晰纹理"]
            enhanced += f"，{', '.join(visual_features)}"
        elif detail_level == "medium":
            visual_features = ["清晰轮廓", "适度细节"]
            enhanced += f"，{', '.join(visual_features)}"
        
        return enhanced