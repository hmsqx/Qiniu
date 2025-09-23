import { Button } from "@/components/ui/button";

import { RotateCw } from "lucide-react";

interface ToolbarProps {
  loading: boolean;
  onRefresh: () => void;
}

export function Toolbar({ loading, onRefresh }: ToolbarProps) {
  return (
    <div className="flex items-center justify-between pb-4">
      <div className="flex items-center gap-3">
        <Button
          variant="outline"
          size="sm"
          onClick={onRefresh}
          disabled={loading}
          className="bg-transparent border-white/20 hover:bg-white/10 text-white"
        >
          <RotateCw
            className={`h-4 w-4 mr-2 ${loading ? "animate-spin" : ""}`}
          />
          刷新
        </Button>
      </div>
    </div>
  );
}
