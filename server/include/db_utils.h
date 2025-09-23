#pragma once

#include <string>
#include <vector>
#include <utility>
#include <jsoncpp/json/json.h>

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

// 更新任务状态
bool updateAi3dTaskStatus(const std::string &jobId, const std::string &status);

// 更新任务失败信息
bool updateAi3dTaskError(const std::string &jobId, const std::string &errorMessage); 

// 尝试消费一次用户的 token_count（>0 才能扣减，原子扣减，成功返回 true）
bool tryConsumeUserTokenOnce(const std::string &userId);

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