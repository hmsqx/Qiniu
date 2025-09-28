import { useState } from "react";
import { Gift } from "lucide-react";
import { Button } from "@/components/ui/button";
import RechargeModal from "@/components/RechargeModal";
import { useAuth } from "@/context/AuthContext";

const RechargeEntry: React.FC = () => {
  const { isAuthenticated, openLoginModal } = useAuth();
  const [open, setOpen] = useState(false);

  const onClick = () => {
    if (!isAuthenticated) {
      openLoginModal();
      return;
    }
    setOpen(true);
  };

  return (
    <>
      <Button
        size="sm"
        onClick={onClick}
        className="bg-gradient-to-r from-purple-500/80 to-fuchsia-500/80 text-white hover:from-purple-500 hover:to-fuchsia-500 shadow-sm"
      >
        <Gift className="h-4 w-4 mr-1" /> 充值
      </Button>
      <RechargeModal open={open} onOpenChange={setOpen} />
    </>
  );
};

export default RechargeEntry;
