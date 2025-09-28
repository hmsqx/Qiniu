请将您希望预置（inject）到运行容器中的初始模型文件放在这里。

 - 默认情况下，此文件夹会以只读模式挂载到容器内的 /opt/seed-models 路径。
 - 在首次启动时（或者当环境变量 ALWAYS_SEED_MODELS=1 时），此文件夹中的文件将被复制到 /var/www/models 目录。
 - 设置环境变量 SEED_OVERWRITE=1 可以在每次启动时覆盖目标目录中的现有文件。

推荐的目录结构示例：

/opt/seed-models/
  glb/
    example.glb
    example.jpg
  obj/
    sample.obj
    sample.jpg

您可以通过在 .env 文件或环境变量中设置 MODEL_FS_BASE_DIR 来自定义模型最终存放的目标基础目录。
您可以通过在 .env 文件或环境变量中设置 SEED_MODELS_DIR 来自定义用于预置模型的挂载路径。