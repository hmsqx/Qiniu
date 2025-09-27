# config.py
from pydantic_settings import BaseSettings

class Settings(BaseSettings):
    # 质量评估阈值
    QUALITY_THRESHOLD: float = 0.7
    
    # 最大优化循环次数
    MAX_ENHANCEMENT_ITERATIONS: int = 3
    
    # 图像处理参数
    MAX_IMAGE_SIZE: int = 2048
    MIN_IMAGE_SIZE: int = 2048
    
    
    class Config:
        env_file = ".env"

settings = Settings()
