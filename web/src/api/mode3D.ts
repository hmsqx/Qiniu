import { get } from "@/utils/request";

export type JobStatus = "排队中" | "处理中" | "完成" | string;

export interface JobItem {
  jobId: string;
  status: JobStatus;
  imgUrl?: string | null;
  modelUrl?: string | null;
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

  if (Array.isArray(data?.taskList)) {
    const list = data.taskList.map((t: any) => ({
      jobId: String(t.jobId || t.jobId),
      // Normalize status codes so the UI substring checks match.
      status: ((): string => {
        const s = (t.status || "").toString();
        const up = s.toUpperCase();
        if (up === "DONE" || up === "SUCCESS" || up === "SUCCEEDED")
          return "完成";
        if (
          up === "PROCESSING" ||
          up === "IN_PROGRESS" ||
          up === "RUNNING" ||
          up === "RUN"
        )
          return "处理中";
        if (up === "PENDING" || up === "QUEUED" || up === "WAITING")
          return "排队中";
        return s;
      })(),
      imgUrl:
        Array.isArray(t.previewImages) && t.previewImages.length > 0
          ? t.previewImages[0]
          : undefined,
      modelUrl:
        Array.isArray(t.modelList) && t.modelList.length > 0
          ? t.modelList[0].fileUrl || t.modelList[0].fileUrl
          : undefined,
    }));
    const total =
      Number(data?.pageInfo?.totalCount ?? data.total ?? 0) || list.length;
    return { list, total };
  }

  if (Array.isArray(data?.list)) {
    return { list: data.list as JobItem[], total: Number(data.total) || 0 };
  }

  if (Array.isArray(data?.records)) {
    return { list: data.records as JobItem[], total: Number(data.total) || 0 };
  }

  if (Array.isArray(data)) {
    return {
      list: data as JobItem[],
      total: Number(resp?.total) || data.length,
    };
  }

  return { list: [], total: 0 };
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
