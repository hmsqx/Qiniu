import { getJobStatusMeta } from "@/constants/jobStatusMap";
import { Badge } from "@/components/ui/badge";
import clsx from "clsx";

interface Props {
  status?: string | null;
}

export function StatusBadge({ status }: Props) {
  const meta = getJobStatusMeta(status || "");
  const color = meta.color || "waiting";
  return (
    <Badge
      variant={"outline"}
      data-status={color}
      className={clsx(
        "status-badge-base border", // ensure base shape & border
        color === "success" && "status-success",
        color === "processing" && "status-processing",
        color === "waiting" && "status-waiting",
        color === "error" && "status-error"
      )}
    >
      {meta.label}
    </Badge>
  );
}
