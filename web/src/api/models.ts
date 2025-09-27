import { get } from "@/utils/request";

export interface AdminModelItem {
  id: string;
  name: string;
  owner: string;
  status?: string;
  createdAt: string; // ISO
  like?: number;
  downloadCount?: number;
  jobId?: string;
  fileurl?: string;
  resultFormat?: string;
  previewImages?: string;
  isPrivate?: boolean;
}

export interface ListAdminModelsResponse {
  list: AdminModelItem[];
  total: number;
  page: number;
  pageSize: number;
}

export interface ListAdminModelsParams {
  page?: number;
  pageSize?: number;
  minLike?: number;
  maxLike?: number;
  minDownload?: number;
  maxDownload?: number;
}

const REAL_ENDPOINT = "/api/admin/models";

function normalize(
  resp: any,
  page: number,
  pageSize: number
): ListAdminModelsResponse {
  const root = resp as any;
  const payload = root?.data ?? root;

  const rawList = Array.isArray(payload?.list)
    ? payload.list
    : Array.isArray(payload?.data?.list)
    ? payload.data.list
    : [];
  const pageInfo = payload?.pageInfo || payload?.data?.pageInfo || {};

  const list: AdminModelItem[] = rawList.map((it: any) => ({
    id: String(it.jobId ?? it.id ?? ""),
    name: String(it.name ?? it.prompt ?? it.title ?? it.jobId ?? ""),
    owner: String(it.owner ?? it.username ?? it.userId ?? ""),
    status: String(it.status ?? ""),
    createdAt: it.create_time
      ? new Date(String(it.create_time).replace(/-/g, "/")).toISOString()
      : String(it.createdAt ?? ""),
    like: typeof it.like === "number" ? it.like : undefined,
    downloadCount:
      typeof it.downloadCount === "number" ? it.downloadCount : undefined,
    jobId: typeof it.jobId === "string" ? it.jobId : undefined,
    fileurl: typeof it.fileurl === "string" ? it.fileurl : undefined,
    resultFormat:
      typeof it.resultFormat === "string" ? it.resultFormat : undefined,
    previewImages:
      typeof it.previewImages === "string" ? it.previewImages : undefined,
    isPrivate:
      typeof it.isPrivate === "boolean"
        ? it.isPrivate
        : typeof it.is_public === "boolean"
        ? !it.is_public
        : typeof it.visibility === "string"
        ? it.visibility.toLowerCase() === "private"
        : undefined,
  }));

  const total = Number(
    pageInfo?.totalCount ?? payload?.total ?? root?.total ?? list.length
  );
  const resolvedPage = Number(pageInfo?.pageNum ?? page);
  const resolvedSize = Number(pageInfo?.pageSize ?? pageSize);

  return { list, total, page: resolvedPage, pageSize: resolvedSize };
}

export async function listAdminModels(
  params: ListAdminModelsParams
): Promise<ListAdminModelsResponse> {
  const {
    page = 1,
    pageSize = 10,
    minLike,
    maxLike,
    minDownload,
    maxDownload,
  } = params || {};
  const query: Record<string, any> = {
    PageNum: page,
    PageSize: pageSize,
  };
  if (typeof minLike === "number") query.minLike = minLike;
  if (typeof maxLike === "number") query.maxLike = maxLike;
  if (typeof minDownload === "number") query.minDownload = minDownload;
  if (typeof maxDownload === "number") query.maxDownload = maxDownload;
  // model format filter removed as per requirements

  const resp = await get<any>(REAL_ENDPOINT, { params: query });
  return normalize(resp, page, pageSize);
}

export default {
  listAdminModels,
};
