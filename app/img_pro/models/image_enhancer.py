# /models/image_enhancer.py
import cv2
import numpy as np
from typing import Dict

class ImageEnhancer:
    """图像增强器"""
    
    def __init__(self):
        pass
    
    def enhance_image(self, image: np.ndarray, quality_scores: Dict[str, float]) -> np.ndarray:
        """根据质量分数增强图像"""
        enhanced_image = image.copy()
        
        # 基于具体的质量问题进行针对性增强
        if quality_scores.get('sharpness', 1.0) < 0.6:
            enhanced_image = self._sharpen_image(enhanced_image)
        
        if quality_scores.get('noise', 1.0) < 0.6:
            enhanced_image = self._denoise_image(enhanced_image)
        
        if quality_scores.get('contrast', 1.0) < 0.6:
            enhanced_image = self._enhance_contrast(enhanced_image)
        
        if quality_scores.get('lighting', 1.0) < 0.6:
            enhanced_image = self._enhance_lighting(enhanced_image)
        
        if quality_scores.get('resolution', 1.0) < 0.6:
            enhanced_image = self._super_resolution(enhanced_image)
        
        return enhanced_image
    
    def _sharpen_image(self, image: np.ndarray) -> np.ndarray:
        """锐化图像"""
        # 创建锐化核
        kernel = np.array([[-1, -1, -1],
                          [-1,  9, -1],
                          [-1, -1, -1]], dtype=np.float32)
        
        # 应用锐化
        sharpened = cv2.filter2D(image, -1, kernel)
        
        # 混合原图和锐化图像
        alpha = 0.6
        result = cv2.addWeighted(image, alpha, sharpened, 1 - alpha, 0)
        
        return result
    
    def _denoise_image(self, image: np.ndarray) -> np.ndarray:
        """去噪"""
        # 使用Non-local Means去噪
        denoised = cv2.fastNlMeansDenoisingColored(image, None, 10, 10, 7, 21)
        return denoised
    
    def _enhance_contrast(self, image: np.ndarray) -> np.ndarray:
        """增强对比度"""
        # 使用CLAHE（对比度限制自适应直方图均衡化）
        lab = cv2.cvtColor(image, cv2.COLOR_BGR2LAB)
        l_channel, a, b = cv2.split(lab)
        
        # 创建CLAHE对象
        clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8, 8))
        l_channel = clahe.apply(l_channel)
        
        # 合并通道
        lab = cv2.merge((l_channel, a, b))
        enhanced = cv2.cvtColor(lab, cv2.COLOR_LAB2BGR)
        
        return enhanced
    
    def _enhance_lighting(self, image: np.ndarray) -> np.ndarray:
        """增强光照"""
        # 使用伽马校正
        gamma = self._calculate_optimal_gamma(image)
        
        # 创建查找表
        inv_gamma = 1.0 / gamma
        table = np.array([((i / 255.0) ** inv_gamma) * 255
                         for i in np.arange(0, 256)]).astype("uint8")
        
        # 应用伽马校正
        enhanced = cv2.LUT(image, table)
        
        return enhanced
    
    def _calculate_optimal_gamma(self, image: np.ndarray) -> float:
        """计算最优伽马值"""
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        mean_brightness = np.mean(gray)
        
        # 根据平均亮度调整伽马值
        if mean_brightness < 80:
            return 0.7  # 提亮暗图像
        elif mean_brightness > 180:
            return 1.3  # 压暗亮图像
        else:
            return 1.0  # 保持不变
    
    def _super_resolution(self, image: np.ndarray) -> np.ndarray:
        """简单的超分辨率（双三次插值）"""
        height, width = image.shape[:2]
        
        # 如果图像太小，进行2倍放大
        if max(height, width) < 512:
            new_height, new_width = height * 2, width * 2
            upsampled = cv2.resize(image, (new_width, new_height), 
                                 interpolation=cv2.INTER_CUBIC)
            return upsampled
        
        return image
    
    def _edge_enhancement(self, image: np.ndarray) -> np.ndarray:
        """边缘增强"""
        # 使用unsharp mask
        gaussian = cv2.GaussianBlur(image, (0, 0), 2.0)
        unsharp_mask = cv2.addWeighted(image, 1.5, gaussian, -0.5, 0)
        return unsharp_mask
