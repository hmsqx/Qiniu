import React from "react";
import { Link } from "react-router-dom";
import {
  Table,
  TableHeader,
  TableRow,
  TableHead,
  TableBody,
  TableCell,
} from "@/components/ui/table";

export interface ModelItem {
  id: string | number;
  name: string;
  owner: string;
  status?: string;
  createdAt: string | number | Date;
  like?: number;
  downloadCount?: number;
  jobId?: string;
  fileurl?: string;
  resultFormat?: string;
  previewImages?: string;
  isPrivate?: boolean;
}

interface ModelsTableProps {
  list: ModelItem[];
  loading: boolean;
  forceSkeleton?: boolean;
  baseIndex?: number;
}

export const ModelsTable: React.FC<ModelsTableProps> = ({
  list,
  loading,
  forceSkeleton,
  baseIndex = 0,
}) => {
  const skeletonRows = 10;
  const renderPrivBadge = (val: boolean | undefined) => {
    if (val === undefined)
      return <span className="text-muted-foreground">-</span>;
    const cls = val
      ? "bg-amber-500/15 border-amber-500/30 text-amber-400"
      : "bg-green-500/15 border-green-500/30 text-green-400";
    return (
      <span
        className={
          "inline-flex items-center rounded-full border px-2 py-0.5 text-xs " +
          cls
        }
      >
        {val ? "私有" : "公有"}
      </span>
    );
  };

  const renderStatusBadge = (status?: string) => {
    if (!status) return <span className="text-muted-foreground">-</span>;
    const t = String(status).toUpperCase();
    let cls = "bg-muted/20 border-muted text-foreground";
    if (["SUCCEED", "SUCCESS", "DONE", "COMPLETED"].includes(t))
      cls = "bg-emerald-500/15 border-emerald-500/30 text-emerald-400";
    else if (["FAILED", "ERROR", "CANCELED"].includes(t))
      cls = "bg-rose-500/15 border-rose-500/30 text-rose-400";
    else if (["RUNNING", "PROCESSING", "PENDING", "QUEUED"].includes(t))
      cls = "bg-sky-500/15 border-sky-500/30 text-sky-400";
    return (
      <span
        className={
          "inline-flex items-center rounded-full border px-2 py-0.5 text-xs " +
          cls
        }
      >
        {status}
      </span>
    );
  };
  return (
    <div
      className="overflow-auto rounded-lg border relative min-h-[460px]"
      aria-busy={loading}
      aria-live="polite"
    >
      <Table>
        <TableHeader className="sticky top-0 z-10 bg-background/90 backdrop-blur">
          <TableRow>
            <TableHead className="w-[90px]">#</TableHead>
            <TableHead>名称</TableHead>
            {/* 去掉所有者列 */}
            <TableHead className="w-[120px]">预览</TableHead>
            <TableHead className="w-[100px]">点赞</TableHead>
            <TableHead className="w-[100px]">下载</TableHead>
            <TableHead className="w-[90px]">是否私有</TableHead>
            <TableHead className="w-[120px]">状态</TableHead>
            <TableHead className="w-[180px]">创建时间</TableHead>
            <TableHead className="w-[100px]">操作</TableHead>
          </TableRow>
        </TableHeader>
        <TableBody>
          {(forceSkeleton || (loading && list.length === 0)) &&
            Array.from({ length: skeletonRows }).map((_, i) => (
              <TableRow key={`skeleton-${i}`} className="even:bg-muted/20">
                <TableCell>
                  <div className="h-4 w-16 rounded bg-muted/40 animate-pulse" />
                </TableCell>
                <TableCell>
                  <div className="h-4 w-40 rounded bg-muted/40 animate-pulse" />
                </TableCell>
                <TableCell>
                  <div className="h-[80px] w-[80px] rounded bg-muted/40 animate-pulse" />
                </TableCell>
                <TableCell>
                  <div className="h-4 w-12 rounded bg-muted/40 animate-pulse" />
                </TableCell>
                <TableCell>
                  <div className="h-4 w-12 rounded bg-muted/40 animate-pulse" />
                </TableCell>
                <TableCell>
                  <div className="h-6 w-16 rounded-full bg-muted/40 animate-pulse" />
                </TableCell>
                <TableCell>
                  <div className="h-6 w-16 rounded-full bg-muted/40 animate-pulse" />
                </TableCell>
                <TableCell>
                  <div className="h-4 w-32 rounded bg-muted/40 animate-pulse" />
                </TableCell>
                <TableCell>
                  <div className="h-4 w-14 rounded bg-muted/40 animate-pulse" />
                </TableCell>
              </TableRow>
            ))}
          {!loading && list.length === 0 && (
            <TableRow>
              <TableCell
                colSpan={9}
                className="text-center py-14 text-muted-foreground"
              >
                暂无数据
              </TableCell>
            </TableRow>
          )}
          {list.map((m, i) => (
            <TableRow key={m.id} className="even:bg-muted/30 hover:bg-muted/60">
              <TableCell className="font-mono text-[11px] text-zinc-400">
                {baseIndex + i + 1}
              </TableCell>
              <TableCell className="font-medium">{m.name}</TableCell>
              <TableCell>
                {m.previewImages ? (
                  <img
                    src={m.previewImages}
                    alt={String(m.name)}
                    className="h-[80px] w-[80px] object-cover rounded border"
                    loading="lazy"
                  />
                ) : (
                  <div className="h-[80px] w-[80px] rounded bg-muted" />
                )}
              </TableCell>
              <TableCell className="text-xs">{m.like ?? 0}</TableCell>
              <TableCell className="text-xs">{m.downloadCount ?? 0}</TableCell>
              <TableCell>{renderPrivBadge(m.isPrivate)}</TableCell>
              <TableCell>{renderStatusBadge(m.status)}</TableCell>
              <TableCell className="text-xs text-muted-foreground">
                {new Date(m.createdAt).toLocaleString()}
              </TableCell>
              <TableCell className="text-xs">
                {m.fileurl ? (
                  <Link
                    className="text-purple-500 hover:underline"
                    to={`/viewer?url=${encodeURIComponent(m.fileurl)}${
                      m.resultFormat
                        ? `&format=${encodeURIComponent(m.resultFormat)}`
                        : ""
                    }${m.jobId ? `&jobId=${encodeURIComponent(m.jobId)}` : ""}`}
                  >
                    详情
                  </Link>
                ) : (
                  <span className="text-muted-foreground">-</span>
                )}
              </TableCell>
            </TableRow>
          ))}
        </TableBody>
      </Table>
    </div>
  );
};
