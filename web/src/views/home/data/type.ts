// Backend-aligned model type returned by /api/showModel
// Example keys observed:
// {
//   Isprivate: false,
//   create_time: "2025-09-25 17:44:41",
//   downloadCount: 0,
//   fileurl: "/models/glb/1758802151532_3481.glb",
//   islike: false,
//   jobId: "1362725518403043328",
//   like: 0,
//   previewImages: "/models/glb/1758802151532_3481.jpg",
//   prompt: "一只橘猫",
//   resultFormat: "GLB",
//   status: "SUCCEED",
//   userId: "aff32f7c-9905-11f0-8927-00163e103396",
//   username: "hou",
//   version: "rapid",
//   viewCount: 0
// }
export interface Inspiration {
  jobId: string;
  prompt: string;
  previewImages?: string; // image url
  fileurl?: string; // model url
  status?: string; // e.g. SUCCEED
  downloadCount?: number;
  like?: number;
  resultFormat?: string; // e.g. GLB
  create_time?: string; // original backend key
  userId?: string;
  username?: string;
  version?: string;
  viewCount?: number;
  Isprivate?: boolean; // original backend key (note capital I)
  islike?: boolean; // server-liked state
}
