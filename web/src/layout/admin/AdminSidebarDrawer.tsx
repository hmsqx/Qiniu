import { Dialog, DialogContent } from "@/components/ui/dialog";
import AdminSidebar from "./AdminSidebar";

type Props = {
  open: boolean;
  onOpenChange: (open: boolean) => void;
};

export default function AdminSidebarDrawer({ open, onOpenChange }: Props) {
  return (
    <Dialog open={open} onOpenChange={onOpenChange}>
      <DialogContent className="p-0 gap-0 max-w-none w-[80vw] sm:w-[380px] h-[100vh] m-0 translate-x-0 left-auto right-0">
        <AdminSidebar variant="drawer" onNavigate={() => onOpenChange(false)} />
      </DialogContent>
    </Dialog>
  );
}
