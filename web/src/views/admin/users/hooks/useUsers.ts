import { useCallback, useEffect, useState } from "react";
import { listUsers } from "@/api/users";
import type { ListUsersResponse } from "@/api/users";

export function useUsers() {
  const [data, setData] = useState<ListUsersResponse | null>(null);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<any>(null);

  const [page, setPage] = useState(1);
  const [pageSize] = useState(10);
  const [keyword, setKeyword] = useState(""); // 兼容旧逻辑（未使用单独字段时）
  const [username, setUsername] = useState("");
  const [email, setEmail] = useState("");
  const [role, setRole] = useState("");

  const fetchData = useCallback(async () => {
    setLoading(true);
    setError(null);
    try {
      const resp = await listUsers({
        page,
        pageSize,
        keyword: username || email || role ? undefined : keyword || undefined,
        username: username || undefined,
        email: email || undefined,
        role: role || undefined,
      });
      setData(resp);
    } catch (e) {
      setError(e);
    } finally {
      setLoading(false);
    }
  }, [page, pageSize, keyword, username, email, role]);

  useEffect(() => {
    fetchData();
  }, [fetchData]);

  return {
    data,
    list: data?.list || [],
    total: data?.total || 0,
    page,
    pageSize,
    keyword,
    setKeyword,
    username,
    setUsername,
    email,
    setEmail,
    role,
    setRole,
    loading,
    error,
    refresh: fetchData,
    nextPage: () => setPage((p) => p + 1),
    prevPage: () => setPage((p) => Math.max(1, p - 1)),
    setPage,
  };
}
