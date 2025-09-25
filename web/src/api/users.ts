import { get } from "@/utils/request";

// 用户实体类型
export interface UserItem {
  id: string; // 唯一 ID
  username: string;
  email: string;
  role: "admin" | "member" | "guest" | string;
  createdAt: string; // ISO 时间
}

// 列表返回
export interface ListUsersResponse {
  list: UserItem[];
  total: number;
  page: number;
  pageSize: number;
}

export interface ListUsersParams {
  page?: number;
  pageSize?: number;
  // 旧的混合 keyword（兼容）
  keyword?: string;
  // 新增独立筛选字段（任意组合）
  username?: string;
  email?: string;
  role?: string; // 精确或模糊（目前简单包含匹配）
}

// 简单的伪随机工具: 固定种子生成稳定 mock 数据，保证翻页/搜索一致性
function seededRandom(seed: number) {
  let x = Math.sin(seed) * 10000;
  return x - Math.floor(x);
}

const ROLES = ["admin", "member", "member", "guest", "guest"]; // 调整概率

// 生成一批稳定的数据（假设 123 条）
function generateAllMockUsers(): UserItem[] {
  const count = 123;
  const users: UserItem[] = [];
  for (let i = 1; i <= count; i++) {
    const r = seededRandom(i);
    const role = ROLES[Math.floor(r * ROLES.length)] || "guest";
    const username = `user_${i.toString().padStart(3, "0")}`;
    const email = `${username}@example.com`;
    // 随机一个过去 0~120 天内的创建时间
    const daysAgo = Math.floor(seededRandom(i + 999) * 120);
    const createdAt = new Date(
      Date.now() - daysAgo * 24 * 3600 * 1000
    ).toISOString();
    users.push({ id: String(i), username, email, role, createdAt });
  }
  return users;
}

// 缓存，避免重复生成
let _ALL_USERS: UserItem[] | null = null;
function ensureAllUsers() {
  if (!_ALL_USERS) _ALL_USERS = generateAllMockUsers();
  return _ALL_USERS;
}

// 过滤与分页
function strIncludes(a: string, b?: string) {
  if (!b) return true;
  return a.toLowerCase().includes(b.trim().toLowerCase());
}

function filterAndSlice(params: ListUsersParams): ListUsersResponse {
  const {
    page = 1,
    pageSize = 10,
    keyword,
    username,
    email,
    role,
  } = params || {};
  const all = ensureAllUsers();

  let arr = all;
  // 如果提供了独立字段，则按字段组合过滤；否则回退 keyword 模糊
  if (username || email || role) {
    arr = all.filter(
      (u) =>
        strIncludes(u.username, username) &&
        strIncludes(u.email, email) &&
        strIncludes(u.role, role)
    );
  } else if (keyword && keyword.trim()) {
    const k = keyword.trim().toLowerCase();
    arr = all.filter(
      (u) =>
        u.username.toLowerCase().includes(k) ||
        u.email.toLowerCase().includes(k) ||
        u.role.toLowerCase().includes(k)
    );
  }

  const total = arr.length;
  const start = (page - 1) * pageSize;
  const list = arr.slice(start, start + pageSize);
  return { list, total, page, pageSize };
}

// 是否使用本地 mock：默认 true（开发友好），生产可在 .env.production 设置 VITE_USE_MOCK=false
const USE_MOCK = (import.meta.env as any)?.VITE_USE_MOCK !== "false";

// 真实 API 端点（如果后端上线后直接替换）
const REAL_ENDPOINT = "/api/admin/users";

export async function listUsers(
  params: ListUsersParams
): Promise<ListUsersResponse> {
  if (USE_MOCK) {
    // 模拟网络延迟
    await new Promise((r) => setTimeout(r, 300));
    return filterAndSlice(params || {});
  }

  // 期望后端返回格式：{ list: UserItem[], total: number }
  const resp = await get<any>(REAL_ENDPOINT, { params });
  const data = (resp?.data ?? resp) as any;
  return {
    list: Array.isArray(data?.list) ? data.list : [],
    total: Number(data?.total) || 0,
    page: params.page || 1,
    pageSize: params.pageSize || 10,
  };
}

export default {
  listUsers,
};
