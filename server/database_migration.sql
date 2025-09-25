-- 为ai3d_tasks表添加新字段
-- 用于存储本地下载的文件URL和预览图片

-- 添加fileurl字段，存储模型文件的本地URL（多个文件用逗号分隔）
ALTER TABLE ai3d_tasks ADD COLUMN fileurl TEXT COMMENT '本地模型文件URL，多个文件用逗号分隔';

-- 添加previewImages字段，存储预览图片的本地URL（多个图片用逗号分隔）
ALTER TABLE ai3d_tasks ADD COLUMN previewImages TEXT COMMENT '本地预览图片URL，多个图片用逗号分隔';

-- 添加索引以提高查询性能
CREATE INDEX idx_ai3d_tasks_fileurl ON ai3d_tasks(fileurl(255));
CREATE INDEX idx_ai3d_tasks_preview ON ai3d_tasks(previewImages(255));

-- 注意：预览图片现在与模型文件放在同一目录下，文件名相同但后缀不同
-- 例如：/model/glb/123456_7890.glb 和 /model/glb/123456_7890.jpg

-- 显示表结构
DESCRIBE ai3d_tasks;
