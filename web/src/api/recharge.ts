import { post } from "@/utils/request";

// API: POST /api/IncrTokenCount
// Header: Session-Token (required)
// Body: { delta: number } 需要充值的数量
// Returns: flexible envelope {code|status,data,message}

export type IncrTokenResp = {
  token_count?: number;
  tokenCount?: number;
};

export async function incrTokenCount(delta: number): Promise<number> {
  if (!Number.isFinite(delta) || delta <= 0) {
    throw new Error("充值数量必须是正整数");
  }
  const body = { delta: Math.floor(delta) };
  const resp = await post<any>("/api/IncrTokenCount", body, {
    headers: {
      // 有些后端需要在 Header 里也读 delta
      delta: String(body.delta),
    },
  });
  const root = resp as any;
  const data = root?.data ?? root;
  const count =
    typeof data?.token_count === "number"
      ? data.token_count
      : typeof data?.tokenCount === "number"
      ? data.tokenCount
      : undefined;
  if (typeof count === "number") return count;
  // 若后端不返回新的余额，则尝试从 data.delta 或 body 推断，但最终还是返回 body.delta 以便前端刷新自己再拉取
  return body.delta;
}

export default {
  incrTokenCount,
};
