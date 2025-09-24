import { recordDownload } from "@/api/statistics";

/** 判断是否需要通过 /model 代理 (兼容带 query 的临时签名 URL) */
export function toCosProxied(url: string): string {
  try {
    const u = new URL(url);
    if (u.hostname.endsWith("tencentcos.cn")) {
      return "/model" + u.pathname + (u.search || "");
    }
    return url;
  } catch {
    return url;
  }
}

export interface DownloadOptions {
  jobId?: string;
  fileName?: string; // 自定义文件名（不含扩展），默认为 jobId 或 'model'
  extHint?: string; // 可传入扩展名提示
  onStart?: () => void;
  onSuccess?: () => void;
  onError?: (err: any) => void;
}

/** 推断扩展名 */
function guessExt(url: string, hint?: string): string {
  if (hint) return hint.replace(/^\./, "");
  const clean = url.split("?")[0].split("#")[0];
  const m = clean.match(/\.([a-z0-9]+)$/i);
  return m ? m[1].toLowerCase() : "model";
}

export async function downloadModelFile(
  rawUrl: string,
  opts: DownloadOptions = {}
) {
  if (!rawUrl) return;
  const { jobId, fileName, extHint, onStart, onSuccess, onError } = opts;
  if (jobId) recordDownload(jobId).catch(() => {});

  const proxied = toCosProxied(rawUrl);
  const debugPrefix = "[downloadModelFile]";
  console.debug(debugPrefix, "rawUrl=", rawUrl, "proxied=", proxied);

  let lastError: any = null;

  onStart?.();

  if (proxied.startsWith("/model/")) {
    try {
      const ext = guessExt(rawUrl, extHint);
      const a = document.createElement("a");
      a.href = proxied; // 让浏览器自己走代理流式下载
      a.download = `${fileName || jobId || "model"}.${ext}`;
      document.body.appendChild(a);
      a.click();
      a.remove();
      onSuccess?.();
      return;
    } catch (err) {
      lastError = err;
      console.warn(debugPrefix, "直接 anchor 代理下载失败, 改为 fetch", err);
    }
  }

  try {
    const res = await fetch(proxied, { credentials: "include" });
    console.debug(debugPrefix, "fetch status=", res.status, res);
    if (!res.ok) throw new Error(`下载失败: ${res.status}`);
    const blob = await res.blob();
    const ext = guessExt(rawUrl, extHint);
    const a = document.createElement("a");
    const url = URL.createObjectURL(blob);
    a.href = url;
    a.download = `${fileName || jobId || "model"}.${ext}`;
    document.body.appendChild(a);
    a.click();
    a.remove();
    URL.revokeObjectURL(url);
    onSuccess?.();
    return;
  } catch (err) {
    lastError = err;
    console.warn(debugPrefix, "fetch blob 下载失败, fallback", err);
  }

  try {
    console.debug(debugPrefix, "尝试 iframe fallback", rawUrl);
    const iframe = document.createElement("iframe");
    iframe.style.display = "none";
    iframe.referrerPolicy = "no-referrer";
    iframe.src = rawUrl;
    document.body.appendChild(iframe);
    setTimeout(() => {
      try {
        iframe.remove();
      } catch {}
    }, 60_000);
    onSuccess?.();
    return;
  } catch (err) {
    lastError = err;
    console.warn(debugPrefix, "iframe fallback 失败", err);
  }

  // 4) 最后退路：直接打开原始链接（可能会离开当前页 / 新标签）
  try {
    console.debug(debugPrefix, "最终直接 window.open", rawUrl);
    window.open(rawUrl, "_blank");
    onSuccess?.();
    return;
  } catch (err) {
    lastError = err;
  }

  onError?.(lastError || new Error("下载失败，所有策略均失败"));
}
