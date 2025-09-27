import { get } from "@/utils/request";

export interface UserItem {
  id: string; // 唯一 ID
  username: string;
  email: string;
  role: "admin" | "member" | "guest" | string;
  createdAt: string; // ISO 时间
}

export interface ListUsersResponse {
  list: UserItem[];
  total: number;
  page: number;
  pageSize: number;
}

export interface ListUsersParams {
  page?: number;
  pageSize?: number;
  keyword?: string;
  username?: string;
  email?: string;
  role?: string;
}

const REAL_ENDPOINT = "/api/admin/users";

export async function listUsers(
  params: ListUsersParams
): Promise<ListUsersResponse> {
  const { page = 1, pageSize = 10, username, email, role } = params || {};
  const query: Record<string, any> = {
    PageNum: page,
    PageSize: pageSize,
  };
  if (username && username.trim()) query.username = username.trim();
  if (email && email.trim()) query.email = email.trim();
  if (role && role.trim()) query.role = role.trim();

  const resp = await get<any>(REAL_ENDPOINT, { params: query });

  const root = resp as any;
  const payload = root?.data ?? root;

  const rawList = Array.isArray(payload?.list)
    ? payload.list
    : Array.isArray(payload?.data?.list)
    ? payload.data.list
    : [];
  const pageInfo = payload?.pageInfo || payload?.data?.pageInfo || {};

  const list: UserItem[] = rawList.map((it: any) => ({
    id: String(it.userId ?? it.id ?? ""),
    username: String(it.username ?? ""),
    email: String(it.email ?? ""),
    role: String(it.role ?? ""),
    createdAt: it.create_time
      ? new Date(it.create_time.replace(/-/g, "/")).toISOString()
      : String(it.createdAt ?? ""),
  }));

  const total = Number(
    pageInfo?.totalCount ?? payload?.total ?? root?.total ?? list.length
  );
  const resolvedPage = Number(pageInfo?.pageNum ?? page);
  const resolvedSize = Number(pageInfo?.pageSize ?? pageSize);

  return {
    list,
    total,
    page: resolvedPage,
    pageSize: resolvedSize,
  };
}

export default {
  listUsers,
};
