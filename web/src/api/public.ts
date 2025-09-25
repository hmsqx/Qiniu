import { get } from "@/utils/request";
import { buildAssetUrl } from "@/utils/asset";

export interface PublicModelItem {
  modelId: string;
  imgUrl?: string | null;
  modelUrl?: string | null;
  isPrivate: false;
  raw?: any;
}

export interface QueryPublicModelsParams {
  pageNum: number;
  pageSize: number;
}

export interface QueryPublicModelsResult {
  list: PublicModelItem[];
  total: number;
}

function normalizeShowModel(resp: any): QueryPublicModelsResult {
  const data = resp?.data ?? resp;

  const rawList: any[] = Array.isArray(data?.taskList)
    ? data.taskList
    : Array.isArray(data?.list)
    ? data.list
    : Array.isArray(data?.models)
    ? data.models
    : [];

  const list: PublicModelItem[] = rawList.map((t: any) => {
    const id = t.modelId || t.jobId || t.id;
    const modelFiles = Array.isArray(t.modelList)
      ? t.modelList
      : Array.isArray(t.files)
      ? t.files
      : [];
    const firstFile = modelFiles[0];
    return {
      modelId: String(id ?? ""),
      imgUrl: t.previewImages || t.imgUrl || t.coverUrl || null,
      modelUrl: buildAssetUrl(
        firstFile?.fileUrl || firstFile?.url || t.modelUrl || undefined
      ),
      isPrivate: false,
      raw: t,
    };
  });

  const total =
    Number(
      data?.pageInfo?.totalCount ??
        data?.total ??
        data?.totalCount ??
        list.length
    ) || list.length;

  return { list, total };
}

export async function queryPublicModels(
  params: QueryPublicModelsParams
): Promise<QueryPublicModelsResult> {
  const { pageNum, pageSize } = params;
  const queryParams = {
    PageNum: pageNum,
    PageSize: pageSize,
    Isprivate: false,
  };

  const resp = await get<any>("/api/showModel", { params: queryParams });
  return normalizeShowModel(resp);
}

export default {
  queryPublicModels,
};
