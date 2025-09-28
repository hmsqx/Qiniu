# models/quality_assessor.py
import cv2
import numpy as np
from typing import Dict

class ImageQualityAssessor:
    """图像质量评估器"""
    
    def __init__(self):
        self.face_cascade = cv2.CascadeClassifier(
            cv2.data.haarcascades + 'haarcascade_frontalface_default.xml'
        )
    
    def assess_quality(self, image: np.ndarray) -> Dict[str, float]:
        """综合评估图像质量"""
        scores = {}
        
        # 1. 分辨率评分
        scores['resolution'] = self._assess_resolution(image)
        
        # 2. 清晰度评分
        scores['sharpness'] = self._assess_sharpness(image)
        
        # 3. 噪声水平评分
        scores['noise'] = self._assess_noise(image)
        
        # 4. 对比度评分
        scores['contrast'] = self._assess_contrast(image)
        
        # 5. 光照均衡性评分
        scores['lighting'] = self._assess_lighting(image)
        
        # 6. 边缘完整度评分
        scores['edge_completeness'] = self._assess_edge_completeness(image)
        
        # 7. 人脸检测评分（如果存在人脸）
        scores['face_quality'] = self._assess_face_quality(image)
        
        # 计算综合得分
        overall_score = self._calculate_overall_score(scores)
        scores['overall'] = overall_score
        
        return scores
    
    def _assess_resolution(self, image: np.ndarray) -> float:
        """评估分辨率"""
        h, w = image.shape[:2]
        total_pixels = h * w
        
        # 基于像素数量评分
        if total_pixels >= 1920 * 1080:  # Full HD或更高
            return 1.0
        elif total_pixels >= 1280 * 720:  # HD
            return 0.8
        elif total_pixels >= 640 * 480:   # VGA
            return 0.6
        elif total_pixels >= 320 * 240:   # QVGA
            return 0.4
        else:
            return 0.2
    
    def _assess_sharpness(self, image: np.ndarray) -> float:
        """评估清晰度（基于Laplacian方差）"""
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        laplacian_var = cv2.Laplacian(gray, cv2.CV_64F).var()
        
        # 归一化到0-1范围
        # 经验阈值：>100为清晰，<50为模糊
        normalized_score = min(laplacian_var / 200.0, 1.0)
        return normalized_score
    
    def _assess_noise(self, image: np.ndarray) -> float:
        """评估噪声水平"""
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        
        # 使用高斯滤波后的差异来估计噪声
        blurred = cv2.GaussianBlur(gray, (5, 5), 0)
        noise_estimate = np.std(gray.astype(float) - blurred.astype(float))
        
        # 噪声越小越好，分数越高
        noise_score = max(0, 1.0 - noise_estimate / 20.0)
        return noise_score
    
    def _assess_contrast(self, image: np.ndarray) -> float:
        """评估对比度"""
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        
        # 使用RMS对比度
        rms_contrast = np.std(gray) / 255.0
        
        # 对比度在0.2-0.5之间较好
        if rms_contrast < 0.1:
            return rms_contrast / 0.1 * 0.5
        elif rms_contrast < 0.5:
            return 0.5 + (rms_contrast - 0.1) / 0.4 * 0.5
        else:
            return max(0.5, 1.0 - (rms_contrast - 0.5) / 0.3 * 0.5)
    
    def _assess_lighting(self, image: np.ndarray) -> float:
        """评估光照均衡性"""
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        
        # 计算图像的亮度分布
        mean_brightness = np.mean(gray)
        brightness_std = np.std(gray)
        
        # 检查过暗或过亮
        darkness_penalty = 0
        if mean_brightness < 50:  # 过暗
            darkness_penalty = (50 - mean_brightness) / 50 * 0.3
        elif mean_brightness > 200:  # 过亮
            darkness_penalty = (mean_brightness - 200) / 55 * 0.3
        
        # 检查对比度分布
        contrast_score = min(brightness_std / 50.0, 1.0)
        
        lighting_score = max(0, contrast_score - darkness_penalty)
        return lighting_score
    
    def _assess_edge_completeness(self, image: np.ndarray) -> float:
        """评估边缘完整度"""
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        
        # 使用Canny边缘检测
        edges = cv2.Canny(gray, 50, 150)
        
        # 计算边缘密度
        edge_density = np.sum(edges > 0) / (edges.shape[0] * edges.shape[1])
        
        # 边缘密度在0.05-0.15之间较好
        if edge_density < 0.02:
            return edge_density / 0.02 * 0.5
        elif edge_density < 0.15:
            return 0.5 + (edge_density - 0.02) / 0.13 * 0.5
        else:
            return max(0.5, 1.0 - (edge_density - 0.15) / 0.1 * 0.5)
    
    def _assess_face_quality(self, image: np.ndarray) -> float:
        """评估人脸质量"""
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        
        # 检测人脸
        faces = self.face_cascade.detectMultiScale(
            gray, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30)
        )
        
        if len(faces) == 0:
            return 1.0  # 无人脸，不影响评分
        
        # 评估人脸质量
        face_scores = []
        for (x, y, w, h) in faces:
            face_roi = gray[y:y+h, x:x+w]
            
            # 人脸大小评分
            face_size = w * h
            if face_size >= 100 * 100:
                size_score = 1.0
            elif face_size >= 50 * 50:
                size_score = 0.8
            else:
                size_score = 0.5
            
            # 人脸清晰度评分
            face_sharpness = cv2.Laplacian(face_roi, cv2.CV_64F).var()
            sharpness_score = min(face_sharpness / 100.0, 1.0)
            
            face_score = (size_score + sharpness_score) / 2
            face_scores.append(face_score)
        
        return np.mean(face_scores) if face_scores else 1.0
    
    def _calculate_overall_score(self, scores: Dict[str, float]) -> float:
        """计算综合得分"""
        weights = {
            'resolution': 0.15,
            'sharpness': 0.25,
            'noise': 0.15,
            'contrast': 0.15,
            'lighting': 0.15,
            'edge_completeness': 0.10,
            'face_quality': 0.05
        }
        
        overall = 0.0
        for metric, weight in weights.items():
            if metric in scores:
                overall += scores[metric] * weight
        
        return overall
