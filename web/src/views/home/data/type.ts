export interface Inspiration {
  id: string; // modelId | jobId
  title: string;
  author: string;
  tags: string[];
  image: string; // preview image (may be empty string if not provided)
  views?: number; // synthetic for now
  status?: string; // e.g. SUCCEED
  downloadCount?: number;
  like?: number;
  resultFormat?: string; // e.g. STL
  createTime?: string; // create_time
  userId?: string;
  version?: string;
  isPrivate?: boolean;
  isLiked?: boolean; // server-side liked state (islike)
}
