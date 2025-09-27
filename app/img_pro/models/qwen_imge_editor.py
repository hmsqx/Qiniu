import base64
import io
from typing import List
from dashscope import MultiModalConversation
import dashscope
import requests
from PIL import Image
from app.body import OptimizedImage


# 配置DashScope
dashscope.base_http_api_url = 'https://dashscope.aliyuncs.com/api/v1'



class Qwen3DOptimizer:
    """3D图像优化器 - 自动应用所有优化策略"""
    
    def __init__(self, api_key):
        self.api_key = api_key
        if not self.api_key:
            raise ValueError("请设置DASHSCOPE_API_KEY环境变量")
        
        # 综合所有优化策略的指令
        self.comprehensive_instruction = """
为3D生成和重建任务全面优化这张图像，请同时应用以下所有优化策略：
1. 清晰度和细节增强：增强图像清晰度和细节，提高边缘定义，改善纹理清晰度，优化对比度以获得更好的深度感知，减少噪音同时保留精细细节，使其更适合3D重建。
2. 光照和深度优化：优化光照和深度线索用于3D建模，平衡亮度和阴影，增强深度信息可见性，改善表面法线定义，调整对比度以获得更好的3D扫描兼容性。
3. 几何和结构优化：增强几何特征，强化结构边缘，改善物体边界定义，优化视角以获得更好的3D重建效果，减少背景干扰。
4. 纹理和材质优化：增强表面纹理和材质细节，改善材质反射特性，优化纹理映射质量，增强材质边界清晰度，为3D材质重建提供更丰富的信息。
5. 色彩和饱和度优化：优化色彩平衡和饱和度，增强色彩对比度，改善颜色准确性，优化色彩梯度过渡，为3D颜色重建提供更准确的色彩信息。
6. 干扰元素清理：完全移除水印、标识等非内容元素，清理背景噪点和无关物体。
7. 背景处理：显著淡化背景复杂度，创建干净的背景环境，确保主体物体突出，便于3D分割和提取。
8. 3D重建特定优化：确保多角度信息完整，优化遮挡区域的可见性，增强空间关系的表达，为点云生成和网格重建提供最佳输入质量。
输出要求：生成适合3D扫描、建模和重建的高质量优化图像，确保几何信息丰富、纹理清晰、空间关系明确、去除背景同时主体的颜色不要变动。
"""
    
    def validate_base64_image(self, base64_string: str) -> bool:
        """验证base64图像格式"""
        try:
            # 移除data URL前缀（如果存在）
            if base64_string.startswith('data:image'):
                base64_string = base64_string.split(',')[1]
            
            # 解码base64
            image_data = base64.b64decode(base64_string)
            
            # 尝试打开图像
            image = Image.open(io.BytesIO(image_data))
            image.verify()
            return True
        except Exception:
            return False
    
    def prepare_image_data_url(self, base64_string: str) -> str:
        """准备图像data URL"""
        if base64_string.startswith('data:image'):
            return base64_string
        else:
            return f"data:image/jpeg;base64,{base64_string}"
    
    async def download_image_to_base64(self, image_url: str) -> str:
        """异步下载图像并转换为base64"""
        try:
            response = requests.get(image_url, timeout=30)
            if response.status_code == 200:
                return base64.b64encode(response.content).decode('utf-8')
            else:
                raise Exception(f"下载失败: HTTP {response.status_code}")
        except Exception as e:
            raise Exception(f"图像下载失败: {str(e)}")
    
    async def optimize_image(self, image_base64: str, num_variants: int = 1) -> List[OptimizedImage]:
        """优化图像生成变体，自动应用所有策略"""
        
        # 验证输入图像
        if not self.validate_base64_image(image_base64):
            raise ValueError("无效的base64图像格式")
        
        # 准备图像URL
        image_data_url = self.prepare_image_data_url(image_base64)
        
        results = []
        
        # 生成多个变体
        for variant_num in range(1, num_variants + 1):
            try:
                # 为每个变体添加差异化指令
                variant_instruction = self.comprehensive_instruction + f"\n\n这是第{variant_num}个变体，请在保持所有优化目标的同时，适当调整处理强度和细节表现，创造出具有细微差异的优化版本。"
                
                # 构建API请求
                messages = [
                    {
                        "role": "user",
                        "content": [
                            {"image": image_data_url},
                            {"text": variant_instruction}
                        ]
                    }
                ]
                
                # 调用API
                response = MultiModalConversation.call(
                    api_key=self.api_key,
                    model="qwen-image-edit",
                    messages=messages,
                    stream=False,
                    watermark=False,
                    negative_prompt=""
                )
                
                if response.status_code == 200:
                    # 解析响应
                    if (hasattr(response, 'output') and response.output.choices and
                        response.output.choices[0].message.content and
                        len(response.output.choices[0].message.content) > 0):
                        
                        content_item = response.output.choices[0].message.content[0]
                        if 'image' in content_item:
                            # 下载图像并转换为base64
                            image_url = content_item['image']
                            optimized_base64 = await self.download_image_to_base64(image_url)
                            
                            results.append(OptimizedImage(
                                variant_number=variant_num,
                                image_base64=optimized_base64,
                                success=True
                            ))
                        else:
                            results.append(OptimizedImage(
                                variant_number=variant_num,
                                image_base64="",
                                success=False,
                                error="API响应中未找到图像"
                            ))
                    else:
                        results.append(OptimizedImage(
                            variant_number=variant_num,
                            image_base64="",
                            success=False,
                            error="API响应格式错误"
                        ))
                else:
                    error_msg = f"API调用失败: HTTP {response.status_code}"
                    if hasattr(response, 'code') and response.code:
                        error_msg += f", 错误码: {response.code}"
                    if hasattr(response, 'message') and response.message:
                        error_msg += f", 错误信息: {response.message}"
                    
                    results.append(OptimizedImage(
                        variant_number=variant_num,
                        image_base64="",
                        success=False,
                        error=error_msg
                    ))
                    
            except Exception as e:
                results.append(OptimizedImage(
                    variant_number=variant_num,
                    image_base64="",
                    success=False,
                    error=f"处理异常: {str(e)}"
                ))
        
        return results



