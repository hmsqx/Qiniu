import { Button } from "@/components/ui/button";
import { Menu } from "lucide-react";

type Props = {
  onOpenSidebar: () => void;
  className?: string;
};

export default function AdminTopBar({ onOpenSidebar, className }: Props) {
  return (
    <div
      className={`sticky top-0 z-20 flex items-center justify-end p-2 md:hidden ${
        className ?? ""
      }`}
    >
      <Button size="icon" variant="ghost" onClick={onOpenSidebar}>
        <Menu className="h-5 w-5" />
      </Button>
    </div>
  );
}
