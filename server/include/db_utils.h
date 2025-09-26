#pragma once

#include <string>
#include <vector>
#include <utility>
#include <jsoncpp/json/json.h>
#include "connection_pool.h"
#include "transaction_manager.h"

// 返回 <总条数, 当前页任务ID列表>
std::pair<int, std::vector<std::pair<std::string, std::string>>> getTaskIdsByMySQLCAPI(const std::string &userId,
                                                              int pageNum,
                                                              int pageSize,
                                                              const std::string &version);

// 插入一条 AI3D 任务记录
bool insertAi3dTask(const std::string &userId,
                    const std::string &jobId,
                    const std::string &requestId,
                    const std::string &prompt,
                    const std::string &resultFormat,
                    const std::string &status,
                    const std::string &version);

// 更新任务的文件URL和预览图片
bool updateAi3dTaskFiles(const std::string &jobId, 
                        const std::string &fileUrl, 
                        const std::string &previewImages);

// 更新任务状态
bool updateAi3dTaskStatus(const std::string &jobId, const std::string &status);

// 更新任务失败信息
bool updateAi3dTaskError(const std::string &jobId, const std::string &errorMessage); 

// 尝试消费一次用户的 token_count（>0 才能扣减，原子扣减，成功返回 true）事务版
bool consumeUserTokenSafely(const std::string& userId);

// 尝试消费一次用户的 token_count（>0 才能扣减，原子扣减，成功返回 true）
//bool tryConsumeUserTokenOnce(const std::string &userId);

// 通过会话 token 获取用户信息（username/user_id/role/token_count），失败返回空对象
Json::Value getUserInfoBySessionToken(const std::string &sessionToken);

// 模型下载量 +1（按任务 tx_job_id 计数）
bool incrementModelDownloadCount(const std::string &jobId);

// 模型收藏量 +1（按任务 tx_job_id 计数）
bool incrementModelLikeCount(const std::string &jobId);

// 分页查询模型（按是否私有筛选），返回 <总条数, 当前页模型列表Json数组>
std::pair<int, Json::Value> queryModelsByPrivacy(bool isPrivate,
                                                 int pageNum,
                                                 int pageSize);

//修改用户余额
bool updateUserTokenCount(const std::string& userId, int delta);

//修改任务私有属性
bool toggleJobIsPrivate(const std::string& jobId);

// 获取任务的详细文件信息
Json::Value getTaskFileInfo(const std::string& jobId);

// 获取任务的完整信息（包含文件URL和私有状态）
Json::Value getTaskCompleteInfo(const std::string& jobId);

// 点赞相关：按用户-模型维度标记是否点赞
// 读取点赞状态（true: 已点赞, false: 未点赞）
bool getUserLikeForJob(const std::string& userId, const std::string& jobId);

// 取反点赞状态，返回操作是否成功，并通过 outNewStatus 返回最新状态
bool toggleUserLikeForJob(const std::string& userId, const std::string& jobId, bool &outNewStatus);

// 浏览量相关
bool incrementModelViewCount(const std::string &jobId);
// 读取某任务的 like、download、view 统计
bool getTaskStats(const std::string &jobId, int &outLike, int &outDownload, int &outView);
// 用户增长：给定时间窗口返回新用户数量
int getNewUserCountInRange(const std::string &startIso, const std::string &endIso);

// 管理员统计与查询
Json::Value getAdminOverviewStats();
// 用户筛选+分页（管理员）返回 <总数, 列表>
std::pair<int, Json::Value> adminQueryUsers(const std::string &username,
                                            const std::string &email,
                                            const std::string &role,
                                            int pageNum,
                                            int pageSize);
// 模型筛选+分页（管理员）返回 <总数, 列表>
std::pair<int, Json::Value> adminQueryModels(const std::string &minLike,
                                             const std::string &maxLike,
                                             const std::string &minDownload,
                                             const std::string &maxDownload,
                                             const std::string &isPrivate,
                                             int pageNum,
                                             int pageSize);