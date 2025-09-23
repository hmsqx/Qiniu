#pragma once

#include <string>
#include <jsoncpp/json/json.h>

// 提交混元转3D任务，成功返回包含 requestId/jobId 的 Json，失败抛异常
Json::Value submitHunyuanTo3DJob(const std::string &prompt,
                                 const std::string &imageBase64,
                                 const std::string &resultFormat);
Json::Value submitHunyuanTo3DJobPro(const std::string &prompt,
                                 const std::string &imageBase64,
                                 const std::string &resultFormat);
Json::Value submitHunyuanTo3DJobRapid(const std::string &prompt,
                                 const std::string &imageBase64,
                                 const std::string &resultFormat);


                                 

// 根据 JobId 查询任务状态及结果，失败时返回包含错误信息的结构（不会抛异常）
Json::Value queryTaskStatusFromTx(const std::string &jobId); 
Json::Value queryTaskStatusFromTxPro(const std::string &jobId); 
Json::Value queryTaskStatusFromTxRapid(const std::string &jobId); 