import React from "react";
import { Badge } from "@/components/ui/badge";
import { roleColor } from "../data/constants";

interface RoleBadgeProps {
  role: string;
}

export const RoleBadge: React.FC<RoleBadgeProps> = ({ role }) => {
  return (
    <Badge
      variant="outline"
      className={`border ${roleColor[role]?.className || ""}`}
    >
      {role}
    </Badge>
  );
};
