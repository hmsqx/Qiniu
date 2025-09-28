"""Backward-compatible settings export for image module.

This module now reuses the centralized app.core.settings.Settings to avoid duplication.
"""
from ..core.settings import get_settings

settings = get_settings()
