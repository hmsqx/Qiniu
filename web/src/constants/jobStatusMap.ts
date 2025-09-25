// Mapping from backend raw job status codes to display labels & style meta.
// Extend as backend introduces new statuses.

export interface JobStatusMeta {
  label: string;
  color?: string; // optional semantic color token / tailwind class
}

// Raw status => metadata
import type { RawJobStatus } from "@/api/mode3D";

export const JOB_STATUS_MAP: Record<RawJobStatus, JobStatusMeta> = {
  DONE: { label: "完成", color: "success" },
  RUN: { label: "处理中", color: "processing" },
  WAITING: { label: "排队中", color: "waiting" },
  QUEUE: { label: "排队中", color: "waiting" },
  QUERY_FAILED: { label: "查询失败", color: "error" },
  FAILED: { label: "失败", color: "error" },
};

export function getJobStatusMeta(
  raw: string | undefined | null
): JobStatusMeta {
  if (!raw) return { label: "-" };
  const key = raw.toUpperCase() as RawJobStatus;
  return (
    (JOB_STATUS_MAP as Record<string, JobStatusMeta>)[key] || { label: raw }
  );
}

export function getJobStatusLabel(raw: string | undefined | null): string {
  return getJobStatusMeta(raw).label;
}
