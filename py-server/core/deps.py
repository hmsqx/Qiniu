from __future__ import annotations

from fastapi import Depends, Request, HTTPException
from .settings import get_settings, Settings


def get_app_settings() -> Settings:
    return get_settings()


def get_services(request: Request):
    services = getattr(request.app.state, "services", None)
    if services is None:
        raise HTTPException(status_code=503, detail="服务未初始化")
    return services
