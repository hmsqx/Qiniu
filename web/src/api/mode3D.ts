import { get, post } from "@/utils/request";
import { buildAssetUrl } from "@/utils/asset";

export type RawJobStatus =
  | "DONE"
  | "RUN"
  | "WAITING"
  | "QUEUE"
  | "QUERY_FAILED"
  | "FAILED";

export interface JobItem {
  jobId: string;
  status: RawJobStatus;
  imgUrl?: string | null;
  modelUrl?: string | null;
  isPrivate?: boolean;
  errorMsg?: string;
}

export interface QueryJobsParams {
  pageNum: number;
  pageSize: number;
  userId: string;
}

export interface QueryJobsResult {
  list: JobItem[];
  total: number;
}

function normalize(resp: any): QueryJobsResult {
  const data = resp?.data ?? resp;

  if (!Array.isArray(data?.taskList)) {
    return { list: [], total: 0 };
  }

  const list: JobItem[] = data.taskList.map((t: any) => {
    const raw = (t.status || "").toString().toUpperCase();

    // Normalize a few success variants that backend may return
    const successAliases = new Set(["DONE", "SUCCEED", "SUCCESS", "COMPLETED"]);
    const normalizedStatus: RawJobStatus = successAliases.has(raw)
      ? "DONE"
      : (raw as RawJobStatus);

    // Prefer previewImages string; if array provided, take first
    const preview = Array.isArray(t.previewImages)
      ? t.previewImages[0]
      : t.previewImages;

    // Extract model URL from various possible shapes
    let modelUrl: string | undefined;
    if (Array.isArray(t.modelList) && t.modelList.length > 0) {
      const m0 = t.modelList[0] || {};
      modelUrl = m0.fileUrl || m0.fileurl || m0.url || m0.link;
    }
    // Fallbacks on root level keys commonly seen in other endpoints
    modelUrl =
      modelUrl ||
      t.fileUrl ||
      t.fileurl ||
      t.modelUrl ||
      t.modelurl ||
      undefined;

    return {
      jobId: String(t.jobId || t.JobId || t.id || t.ID || ""),
      status: normalizedStatus,
      imgUrl: preview,
      modelUrl: buildAssetUrl(modelUrl),
      isPrivate: typeof t.Isprivate === "boolean" ? t.Isprivate : t.isPrivate,
      errorMsg: typeof t.errorMsg === "string" ? t.errorMsg : undefined,
    } as JobItem;
  });

  const total =
    Number(data?.pageInfo?.totalCount ?? list.length) || list.length;
  return { list, total };
}

export async function queryJobsByPage(
  params: QueryJobsParams
): Promise<QueryJobsResult> {
  const formattedParams: Record<string, any> = {};
  Object.keys(params).forEach((key) => {
    const newKey = key.charAt(0).toUpperCase() + key.slice(1);
    formattedParams[newKey] = (params as any)[key];
  });
  const resp = await get<any>("/api/query", { params: formattedParams });
  return normalize(resp);
}

export interface ToggleVisibilityResponse {
  jobId: string;
  isPrivate?: boolean;
  [k: string]: any;
}

export async function toggleJobVisibility(jobId: string) {
  if (!jobId) throw new Error("jobId 不能为空");
  const resp = await post<any>("/api/toggleJobIsPrivate", { jobId });
  return (resp?.data || resp) as ToggleVisibilityResponse;
}
