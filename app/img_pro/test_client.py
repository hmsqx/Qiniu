import base64
import requests
import json
from pathlib import Path

class ImageQualityTestClient:
    def __init__(self, base_url="http://localhost:8091"):
        self.base_url = base_url
    
    def image_to_base64(self, image_path: str) -> str:
        """将图像文件转换为base64字符串"""
        with open(image_path, "rb") as image_file:
            encoded_string = base64.b64encode(image_file.read()).decode('utf-8')
            return f"data:image/jpeg;base64,{encoded_string}"
    
    def save_base64_image(self, base64_string: str, output_path: str):
        """保存base64图像到文件"""
        if ',' in base64_string:
            base64_string = base64_string.split(',')[1]
        
        image_data = base64.b64decode(base64_string)
        with open(output_path, 'wb') as f:
            f.write(image_data)
    
    def test_assess_and_enhance(self, image_path: str, threshold: float = 0.7):
        """测试图像质量评估与增强"""
        print(f"\n=== 测试图像: {image_path} ===")
        
        # 读取并编码图像
        base64_image = self.image_to_base64(image_path)
        
        # 发送请求
        payload = {
            "image": base64_image,
            "threshold": threshold,
            "max_iterations": 3
        }
        
        response = requests.post(f"{self.base_url}/pro_pic/assess_and_enhance", 
                               json=payload)
        
        if response.status_code == 200:
            result = response.json()
            
            print(f"整体质量得分: {result['overall_score']:.3f}")
            print(f"是否需要增强: {result['needs_enhancement']}")
            print(f"增强迭代次数: {result['enhancement_iterations']}")
            print(f"处理状态: {result['message']}")
            
            print("\n详细评分:")
            for metric, score in result['detailed_scores'].items():
                if metric != 'overall':
                    print(f"  {metric}: {score:.3f}")
            
            # 保存增强后的图像
            if result['enhanced_image']:
                output_path = f"asserts/enhanced_{Path(image_path).name}"
                self.save_base64_image(result['enhanced_image'], output_path)
                print(f"增强后的图像已保存到: {output_path}")
            
            return result
        else:
            print(f"请求失败: {response.status_code}")
            print(f"错误信息: {response.text}")
            return None
    
    def test_assess_only(self, image_path: str):
        """测试仅质量评估"""
        print(f"\n=== 仅评估图像质量: {image_path} ===")
        
        # 读取并编码图像
        base64_image = self.image_to_base64(image_path)
        
        # 发送请求
        payload = {
            "image": base64_image
        }
        
        response = requests.post(f"{self.base_url}/pro_pic/assess_only", 
                               json=payload)
        
        if response.status_code == 200:
            result = response.json()
            
            print("质量评估结果:")
            for metric, score in result.items():
                print(f"  {metric}: {score:.3f}")
            
            return result
        else:
            print(f"请求失败: {response.status_code}")
            print(f"错误信息: {response.text}")
            return None

def main():
    """主测试函数"""
    client = ImageQualityTestClient()
    
    # 测试图像路径列表
    test_images = [
        "asserts/goose_h.png",  # 高质量图像
        "asserts/goose_l1.png",  # 低质量图像
        "asserts/goose_l2.png",  # 模糊图像
    ]

    test_images = [
        "asserts/goose_m1.png",  # 高质量图像
    ]
    
    print("开始测试图像质量评估与增强服务...")
    
    # 测试健康检查
    try:
        response = requests.get("http://localhost:8091/")
        print(f"服务状态: {response.json()}")
    except:
        print("服务未启动，请先启动服务")
        return
    
    # 测试每个图像
    for image_path in test_images:
        if Path(image_path).exists():
            # 测试完整流程
            client.test_assess_and_enhance(image_path, threshold=0.7)
            
            # 测试仅评估
            client.test_assess_only(image_path)
        else:
            print(f"图像文件不存在: {image_path}")
    
    print("\n测试完成!")

if __name__ == "__main__":
    main()