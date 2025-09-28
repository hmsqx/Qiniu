# app/utils/image_utils.py
import base64
import io
import cv2
import numpy as np
from PIL import Image
from typing import Tuple, Union

def base64_to_image(base64_string: str) -> np.ndarray:
    """将base64字符串转换为OpenCV图像"""
    # 移除base64前缀（如果有）
    if ',' in base64_string:
        base64_string = base64_string.split(',')[1]
    
    # 解码base64
    image_bytes = base64.b64decode(base64_string)
    
    # 转换为PIL图像
    pil_image = Image.open(io.BytesIO(image_bytes))
    
    # 转换为RGB（确保3通道）
    if pil_image.mode != 'RGB':
        pil_image = pil_image.convert('RGB')
    
    # 转换为numpy数组
    image_array = np.array(pil_image)
    
    # 转换为OpenCV格式（BGR）
    opencv_image = cv2.cvtColor(image_array, cv2.COLOR_RGB2BGR)
    
    return opencv_image

def image_to_base64(image: np.ndarray, format: str = 'JPEG') -> str:
    """将OpenCV图像转换为base64字符串"""
    # 转换为RGB
    if len(image.shape) == 3:
        image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
    else:
        image_rgb = image
    
    # 转换为PIL图像
    pil_image = Image.fromarray(image_rgb)
    
    # 保存到字节流
    buffer = io.BytesIO()
    pil_image.save(buffer, format=format, quality=95)
    
    # 编码为base64
    base64_string = base64.b64encode(buffer.getvalue()).decode('utf-8')
    
    return f"data:image/{format.lower()};base64,{base64_string}"

def resize_image(image: np.ndarray, max_size: int = 2048) -> np.ndarray:
    """调整图像大小，保持宽高比"""
    h, w = image.shape[:2]
    
    if max(h, w) <= max_size:
        return image
    
    if h > w:
        new_h = max_size
        new_w = int(w * max_size / h)
    else:
        new_w = max_size
        new_h = int(h * max_size / w)
    
    return cv2.resize(image, (new_w, new_h), interpolation=cv2.INTER_AREA)
