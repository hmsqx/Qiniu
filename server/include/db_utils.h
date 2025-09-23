#pragma once

#include <string>
#include <vector>
#include <utility>

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