// URL 与文件扩展名相关的工具方法
// 位置：src/utils/url.ts

/**
 * 从 URL 或路径中提取文件扩展名。
 * - 返回不带点的扩展名且为小写，例如："png"。
 * - 若无法解析或入参为空，返回 null。
 */
export function getExt(url?: string | null): string | null {
  if (!url) return null;
  const m = url
    .split("?")[0]
    .split("#")[0]
    .match(/\.([a-z0-9]+)$/i);
  return m ? m[1].toLowerCase() : null;
}

/**
 * 将绝对/相对链接转为应用的模型代理地址格式。
 * - 若路径已以 "/model/" 开头，则保持不变（保留 query）。
 * - 否则在路径前加上 "/model" 前缀。
 * - 解析 URL 失败时，返回原始入参或空字符串。
 */
export function toProxiedUrl(url?: string | null): string {
  if (!url) return "";
  try {
    const u = new URL(url, window.location.origin);

    return u.pathname + (u.search || "");
  } catch {
    return url || "";
  }
}
