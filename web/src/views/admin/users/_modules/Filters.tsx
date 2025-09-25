import React from "react";
import { Card, CardContent } from "@/components/ui/card";
import { Input } from "@/components/ui/input";
import { Button } from "@/components/ui/button";
import {
  Select,
  SelectTrigger,
  SelectContent,
  SelectItem,
  SelectValue,
} from "@/components/ui/select";
import { Search, RotateCcw, Eraser, Loader2 } from "lucide-react";

export interface FiltersValue {
  username: string;
  email: string;
  role: string; // empty string means all
}

interface FiltersProps {
  value: FiltersValue;
  loading: boolean;
  onFiltersChange: (value: FiltersValue) => void;
  onSearch: () => void; // trigger apply with current value
  onReset: () => void; // parent resets value then passes down
  onRefresh: () => void;
}

export const Filters: React.FC<FiltersProps> = ({
  value,
  loading,
  onFiltersChange,
  onSearch,
  onReset,
  onRefresh,
}) => {
  const update = (patch: Partial<FiltersValue>) => {
    onFiltersChange({ ...value, ...patch });
  };

  return (
    <Card className="backdrop-blur supports-[backdrop-filter]:bg-background/70">
      <CardContent className="pt-4">
        <div className="flex flex-col gap-4 md:flex-row md:items-center md:flex-wrap">
          <div className="flex gap-3 flex-col sm:flex-row">
            <Input
              placeholder="用户名"
              value={value.username}
              onChange={(e) => update({ username: e.target.value })}
              onKeyDown={(e) => e.key === "Enter" && onSearch()}
              className="w-[180px]"
            />
            <Input
              placeholder="邮箱"
              value={value.email}
              onChange={(e) => update({ email: e.target.value })}
              onKeyDown={(e) => e.key === "Enter" && onSearch()}
              className="w-[220px]"
            />
            <Select
              value={value.role || "all"}
              onValueChange={(v) => update({ role: v === "all" ? "" : v })}
            >
              <SelectTrigger className="w-[140px]" size="sm">
                <SelectValue placeholder="角色" />
              </SelectTrigger>
              <SelectContent>
                <SelectItem value="all">全部角色</SelectItem>
                <SelectItem value="admin">admin</SelectItem>
                <SelectItem value="member">member</SelectItem>
                <SelectItem value="guest">guest</SelectItem>
              </SelectContent>
            </Select>
          </div>
          <div className="flex gap-2">
            <Button onClick={onSearch} disabled={loading} className="gap-1">
              {loading ? (
                <Loader2 className="w-4 h-4 animate-spin" />
              ) : (
                <Search className="w-4 h-4" />
              )}
              搜索
            </Button>
            <Button
              type="button"
              variant="outline"
              onClick={onReset}
              disabled={loading}
              className="gap-1"
            >
              {loading ? (
                <Loader2 className="w-4 h-4 animate-spin" />
              ) : (
                <Eraser className="w-4 h-4" />
              )}
              重置
            </Button>
            <Button
              type="button"
              variant="ghost"
              onClick={onRefresh}
              disabled={loading}
              className="gap-1"
            >
              {loading ? (
                <Loader2 className="w-4 h-4 animate-spin" />
              ) : (
                <RotateCcw className="w-4 h-4" />
              )}
              刷新
            </Button>
          </div>
        </div>
      </CardContent>
    </Card>
  );
};
