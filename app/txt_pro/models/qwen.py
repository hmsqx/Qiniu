from fastapi import HTTPException
import logging
from openai import OpenAI


# 配置日志
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("AugmentService")
class QwenLLMClient:
    """千问大模型客户端封装类"""
    
    def __init__(self, api_key: str = None, model: str = "qwen-plus", env=None):
        """
        初始化千问客户端
        
        Args:
            api_key: API密钥，如果为None则从环境变量获取
            model: 使用的模型名称
        """
        self.api_key = api_key or env["DASHSCOPE_API_KEY"]
        if not self.api_key:
            raise ValueError("请设置DASHSCOPE_API_KEY环境变量或传入api_key参数")
        
        self.model = model
        self.client = OpenAI(
            api_key=self.api_key,
            base_url="https://dashscope.aliyuncs.com/compatible-mode/v1",
        )
        
        logger.info(f"千问客户端初始化完成，使用模型: {self.model}")
    
    async def chat_completion(self, system_prompt: str, user_prompt: str, 
                            max_tokens: int = 1000, temperature: float = 0.7) -> str:
        """
        调用千问模型进行对话
        
        Args:
            system_prompt: 系统提示词
            user_prompt: 用户提示词
            max_tokens: 最大token数
            temperature: 温度参数
            
        Returns:
            str: 模型回复内容
        """
        try:
            messages = [
                {"role": "system", "content": system_prompt},
                {"role": "user", "content": user_prompt}
            ]
            
            completion = self.client.chat.completions.create(
                model=self.model,
                messages=messages,
                max_tokens=max_tokens,
                temperature=temperature,
                # 取消流式输出和思考模式
                stream=False,
                extra_body={"enable_thinking": False}
            )
            
            response_text = completion.choices[0].message.content.strip()
            logger.info(f"千问API调用成功，响应长度: {len(response_text)}")
            
            return response_text
            
        except Exception as e:
            logger.error(f"千问API调用失败: {str(e)}")
            raise HTTPException(
                status_code=500, 
                detail=f"千问模型调用失败: {str(e)}"
            )
    
    def get_model_info(self) -> dict:
        """获取当前使用的模型信息"""
        return {
            "model": self.model,
            "base_url": "https://dashscope.aliyuncs.com/compatible-mode/v1",
            "provider": "阿里云千问"
        }
