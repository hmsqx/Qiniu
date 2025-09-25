import { post } from "@/utils/request";

// 通用返回包裹（后端可能返回这种结构，也可能直接返回 data）
export interface ApiEnvelope<T = any> {
  code?: number;
  status?: string;
  message?: string;
  data?: T;
}

// 点赞 / 下载 操作的通用响应（多数情况下只需要知道成功与否）
export interface OperationResult {
  success?: boolean;
  likeCount?: number; // 最新点赞数量
  downloadCount?: number; // 最新下载数量
  [k: string]: any;
}

// 下载统计接口当前返回格式示例：
// { "code":200, "message":"下载计数+1", "status":"success" }
export interface DownloadStatResponse {
  code: number;
  message: string;
  status: string; // e.g. 'success'
}

// 点赞模型接口。传入目标模型的 jobId。
export async function likeModel(
  jobId: string
): Promise<ApiEnvelope<OperationResult> | OperationResult> {
  if (!jobId) throw new Error("jobId 不能为空");
  return post<ApiEnvelope<OperationResult> | OperationResult>("/api/like", {
    jobId,
  });
}

// 统计“下载”操作接口。
// 说明：后端提供的 /api/downloadModel 仅用于增加下载统计次数，并不返回真实文件。
// 若后续需要真正下载文件，应调用返回真实模型地址的其它接口再执行实际下载逻辑。
// 原始请求函数：真正向后端发送统计请求
async function postDownloadStat(jobId: string): Promise<DownloadStatResponse> {
  return post<DownloadStatResponse>("/api/downloadModel", { jobId });
}

// 简单节流/去重：同一个 jobId 在 cooldown 内重复点击，只发一次请求。
const downloadCooldownMs = 3000; // 3 秒内忽略重复
const lastDownloadHit: Record<string, number> = {};
const inflight: Record<string, Promise<DownloadStatResponse> | undefined> = {};

export async function downloadModel(
  jobId: string
): Promise<DownloadStatResponse | undefined> {
  if (!jobId) throw new Error("jobId 不能为空");
  const now = Date.now();
  const last = lastDownloadHit[jobId] || 0;
  if (now - last < downloadCooldownMs) {
    // 如果已有在途请求则返回该 Promise；否则直接忽略（返回 undefined 代表被节流）
    return inflight[jobId];
  }
  lastDownloadHit[jobId] = now;
  const p = postDownloadStat(jobId)
    .catch((e) => {
      // 失败时允许后续立即重试（清理时间戳）
      delete lastDownloadHit[jobId];
      throw e;
    })
    .finally(() => {
      // 请求完成后移除 in-flight 记录
      delete inflight[jobId];
    });
  inflight[jobId] = p;
  return p;
}

// 别名：更易读的统计方法
export const recordDownload = downloadModel;
