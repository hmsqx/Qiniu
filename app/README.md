# quickstart
1.激活环境
source .venv/bin/activate 

取消激活：deactivate

2.启动服务器
bash start_pro_service.sh

3.终止服务器
bash stop_pro_service.sh


4.测试服务器健康

curl --location --request GET 'http://47.120.8.25:8090/pro_txt/model-info'
curl --location --request GET 'http://47.120.8.25:8090/'

ss -tulnp | grep 8090

依赖安装 //
pip install tencentcloud-sdk-python-ai3d -i https://pypi.tuna.tsinghua.edu.cn/simple
pip install scikit-image -i https://pypi.tuna.tsinghua.edu.cn/simple
pip install pydantic-settings -i https://mirrors.aliyun.com/pypi/simple/
pip install tencentcloud-sdk-python-ai3d -i https://pypi.tuna.tsinghua.edu.cn/simple
pip install fastapi -i https://pypi.tuna.tsinghua.edu.cn/simple
pip install openai -i https://pypi.tuna.tsinghua.edu.cn/simple
pip install opencv-python -i https://pypi.tuna.tsinghua.edu.cn/simple
sudo apt-get install -y libgl1 libglib2.0-0
pip install uvicorn -i https://pypi.tuna.tsinghua.edu.cn/simple
pip install dashscope -i https://pypi.tuna.tsinghua.edu.cn/simple