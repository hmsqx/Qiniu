import React, { useState } from "react";
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

export interface FiltersState {
  username: string;
  email: string;
  role: string;
}

interface FiltersProps {
  initial: FiltersState;
  loading: boolean;
  onApply: (state: FiltersState) => void;
  onRefresh: () => void;
}

export const Filters: React.FC<FiltersProps> = ({
  initial,
  loading,
  onApply,
  onRefresh,
}) => {
  const [tempUsername, setTempUsername] = useState(initial.username);
  const [tempEmail, setTempEmail] = useState(initial.email);
  const [tempRole, setTempRole] = useState(initial.role || "all");

  const apply = () => {
    onApply({
      username: tempUsername.trim(),
      email: tempEmail.trim(),
      role: tempRole === "all" ? "" : tempRole.trim(),
    });
  };

  const reset = () => {
    setTempUsername("");
    setTempEmail("");
    setTempRole("all");
    onApply({ username: "", email: "", role: "" });
  };

  return (
    <Card className="backdrop-blur supports-[backdrop-filter]:bg-background/70">
      <CardContent className="pt-4">
        <div className="flex flex-col gap-4 md:flex-row md:items-center md:flex-wrap">
          <div className="flex gap-3 flex-col sm:flex-row">
            <Input
              placeholder="用户名"
              value={tempUsername}
              onChange={(e) => setTempUsername(e.target.value)}
              onKeyDown={(e) => e.key === "Enter" && apply()}
              className="w-[180px]"
            />
            <Input
              placeholder="邮箱"
              value={tempEmail}
              onChange={(e) => setTempEmail(e.target.value)}
              onKeyDown={(e) => e.key === "Enter" && apply()}
              className="w-[220px]"
            />
            <Select value={tempRole} onValueChange={(v) => setTempRole(v)}>
              <SelectTrigger className="w-[140px]" size="sm">
                <SelectValue placeholder="角色" />
              </SelectTrigger>
              <SelectContent>
                <SelectItem value="all">角色</SelectItem>
                <SelectItem value="admin">admin</SelectItem>
                <SelectItem value="member">member</SelectItem>
                <SelectItem value="guest">guest</SelectItem>
              </SelectContent>
            </Select>
          </div>
          <div className="flex gap-2">
            <Button onClick={apply} disabled={loading} className="gap-1">
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
              onClick={reset}
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
