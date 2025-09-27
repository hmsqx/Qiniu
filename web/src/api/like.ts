import { post } from "@/utils/request";

export async function likeModel(jobId: string): Promise<any> {
  if (!jobId) throw new Error("jobId is required");
  return post<any>("/api/like", { jobId });
}

export default {
  likeModel,
};
