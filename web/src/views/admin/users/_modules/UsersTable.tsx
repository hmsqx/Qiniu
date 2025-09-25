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
}

export const UsersTable: React.FC<UsersTableProps> = ({ list, loading }) => {
  return (
    <div className="overflow-auto rounded-lg border relative">
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
          {loading && list.length === 0 && (
            <TableRow>
              <TableCell
                colSpan={5}
                className="text-center py-14 text-muted-foreground"
              >
                加载中...
              </TableCell>
            </TableRow>
          )}
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
          {list.map((u) => (
            <TableRow key={u.id} className="even:bg-muted/30 hover:bg-muted/60">
              <TableCell className="font-mono text-[11px] text-zinc-400">
                {u.id}
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
