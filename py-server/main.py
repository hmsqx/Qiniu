from fastapi import FastAPI
from contextlib import asynccontextmanager
import os

from api.routers import text as text_router
from api.routers import image as image_router
from api.routers import vip as vip_router
from core.settings import get_settings
from core.logging import logger
from txt_pro.models.qwen import QwenLLMClient
from txt_pro.models.txt_optimizer import Text3DPromptOptimizer
from img_pro.models.quality_assessor import ImageQualityAssessor
from img_pro.models.image_enhancer import ImageEnhancer
from img_pro.models.qwen_imge_editor import Qwen3DOptimizer


class Services:
    def __init__(self):
        self.qwen_client: QwenLLMClient | None = None
        self.text_3d_optimizer: Text3DPromptOptimizer | None = None
        self.quality_assessor: ImageQualityAssessor | None = None
        self.image_enhancer: ImageEnhancer | None = None
        self.optimizer: Qwen3DOptimizer | None = None


@asynccontextmanager
async def lifespan(app: FastAPI):
    settings = get_settings()
    app.state.services = Services()
    try:
        app.state.services.qwen_client = QwenLLMClient(model=settings.QWEN_MODEL)
        app.state.services.text_3d_optimizer = Text3DPromptOptimizer(app.state.services.qwen_client)
        logger.info("文生3D提示词优化服务启动成功！")
        logger.info(f"当前使用模型: {app.state.services.qwen_client.get_model_info()}")
        app.state.services.quality_assessor = ImageQualityAssessor()
        app.state.services.image_enhancer = ImageEnhancer()
        logger.info("图片修复优化服务启动成功！")
        app.state.services.optimizer = Qwen3DOptimizer()
        logger.info("vip图片修复服务启动成功！")
    except Exception as e:
        logger.error(f"服务启动失败: {str(e)}")
        raise
    yield


settings = get_settings()
app = FastAPI(
    title=settings.APP_NAME,
    description=settings.APP_DESCRIPTION,
    version=settings.APP_VERSION,
    lifespan=lifespan,
)


@app.get("/")
async def root():
    return {"message": settings.APP_NAME + "服务", "version": settings.APP_VERSION}


app.include_router(text_router.router)
app.include_router(image_router.router)
app.include_router(vip_router.router)


if __name__ == "__main__":
    import uvicorn
    if not os.getenv("DASHSCOPE_API_KEY") and not get_settings().DASHSCOPE_API_KEY:
        print("警告: 请设置 DASHSCOPE_API_KEY 环境变量 或在 api.env 中配置")
    print("启动文生3D提示词优化服务...")
    uvicorn.run(
        "main:app",
        host=settings.HOST,
        port=settings.PORT,
        reload=settings.RELOAD,
        log_level="info",
    )
