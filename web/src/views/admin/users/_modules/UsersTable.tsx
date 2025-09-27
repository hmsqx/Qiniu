import React from "react";
import {
  Table,
  TableHeader,
  TableRow,
  TableHead,
  TableBody,
  TableCell,
} from "@/components/ui/table";
import { RoleBadge } from "./RoleBadge";

export interface UserItem {
  id: string | number;
  username: string;
  email: string;
  role: string;
  createdAt: string | number | Date;
}

interface UsersTableProps {
  list: UserItem[];
  loading: boolean;
  // 当翻页或筛选触发时强制显示骨架屏，即使 list 可能有上一次的数据
  forceSkeleton?: boolean;
  // 用于计算跨页连续的序号，例如 (page - 1) * pageSize
  baseIndex?: number;
}

export const UsersTable: React.FC<UsersTableProps> = ({
  list,
  loading,
  forceSkeleton,
  baseIndex = 0,
}) => {
  const skeletonRows = 10; // 更大的占位，避免加载完成后表格高度突变
  return (
    <div
      className="overflow-auto rounded-lg border relative min-h-[380px]"
      aria-busy={loading}
      aria-live="polite"
    >
      <Table>
        <TableHeader className="sticky top-0 z-10 bg-background/90 backdrop-blur">
          <TableRow>
            <TableHead className="w-[90px]">#</TableHead>
            <TableHead>用户名</TableHead>
            <TableHead>邮箱</TableHead>
            <TableHead className="w-[120px]">角色</TableHead>
            <TableHead className="w-[180px]">创建时间</TableHead>
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
                  <div className="h-4 w-56 rounded bg-muted/40 animate-pulse" />
                </TableCell>
                <TableCell>
                  <div className="h-6 w-16 rounded-full bg-muted/40 animate-pulse" />
                </TableCell>
                <TableCell>
                  <div className="h-4 w-32 rounded bg-muted/40 animate-pulse" />
                </TableCell>
              </TableRow>
            ))}
          {!loading && list.length === 0 && (
            <TableRow>
              <TableCell
                colSpan={5}
                className="text-center py-14 text-muted-foreground"
              >
                暂无数据
              </TableCell>
            </TableRow>
          )}
          {list.map((u, i) => (
            <TableRow key={u.id} className="even:bg-muted/30 hover:bg-muted/60">
              <TableCell className="font-mono text-[11px] text-zinc-400">
                {baseIndex + i + 1}
              </TableCell>
              <TableCell className="font-medium">{u.username}</TableCell>
              <TableCell className="text-xs">{u.email}</TableCell>
              <TableCell>
                <RoleBadge role={u.role} />
              </TableCell>
              <TableCell className="text-xs text-muted-foreground">
                {new Date(u.createdAt).toLocaleString()}
              </TableCell>
            </TableRow>
          ))}
        </TableBody>
      </Table>
    </div>
  );
};
