import { Button } from "@/components/ui/button";
import { Menu } from "lucide-react";

const MobileMenuButton = () => {
  return (
    <div className="md:hidden">
      <Button variant="ghost" size="icon">
        <Menu className="h-6 w-6" />
      </Button>
    </div>
  );
};

export default MobileMenuButton;
