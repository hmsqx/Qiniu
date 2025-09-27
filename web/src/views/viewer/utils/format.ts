export function getFormatFromUrl(url: string | null): string | null {
  if (!url) return null;
  const m = url
    .split("?")[0]
    .split("#")[0]
    .match(/\.([a-z0-9]+)$/i);
  return m ? m[1].toLowerCase() : null;
}
