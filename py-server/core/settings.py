from __future__ import annotations

import os
from pathlib import Path
from functools import lru_cache
from pydantic_settings import BaseSettings, SettingsConfigDict


class Settings(BaseSettings):
    # Server
    APP_NAME: str = "增强模式"
    APP_DESCRIPTION: str = "输入增强优化API"
    APP_VERSION: str = "5.0.0"
    HOST: str = "0.0.0.0"
    PORT: int = 8090
    RELOAD: bool = True

    # External APIs
    DASHSCOPE_API_KEY: str | None = None
    QWEN_MODEL: str = "qwen-plus"
    QWEN_IMAGE_EDIT_MODEL: str = "qwen-image-edit"

    # Image processing
    QUALITY_THRESHOLD: float = 0.7
    MAX_ENHANCEMENT_ITERATIONS: int = 3
    MAX_IMAGE_SIZE: int = 2048
    MIN_IMAGE_SIZE: int = 2048

    # OpenAI-compatible base for Qwen text
    QWEN_COMPATIBLE_BASE_URL: str = (
        "https://dashscope.aliyuncs.com/compatible-mode/v1"
    )

    # Dashscope API base
    DASHSCOPE_BASE_HTTP_API_URL: str = "https://dashscope.aliyuncs.com/api/v1"

    # Resolve env file: prefer APP_ENV_FILE, else default to app/api.env
    _pkg_root = Path(__file__).resolve().parent.parent
    _default_env = _pkg_root / "api.env"
    _env_file = os.environ.get("APP_ENV_FILE", str(_default_env))

    model_config = SettingsConfigDict(
        env_file=_env_file,
        env_file_encoding="utf-8",
        extra="ignore",
    )


@lru_cache()
def get_settings() -> Settings:
    """Cached settings factory."""
    return Settings()
