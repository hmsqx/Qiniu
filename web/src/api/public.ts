import { get } from "@/utils/request";

export interface QueryPublicModelsParams {
  pageNum: number;
  pageSize: number;
}

export interface QueryPublicModelsResult {
  list: any[];
  total: number;
}

function normalizeShowModel(resp: any): QueryPublicModelsResult {
  const data = resp?.data ?? resp;

  const rawList: any[] = Array.isArray(data?.list) ? data.list : [];
  const list: any[] = rawList;

  const total = Number(data?.pageInfo?.totalCount);
  console.log(list, total);
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
