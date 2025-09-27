import { post } from "@/utils/request";

export async function likeModel(jobId: string): Promise<any> {
  return post<any>("/api/like/toggle", { jobId });
}

export default {
  likeModel,
};
