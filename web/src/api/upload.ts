import { post } from "@/utils/request";

const UPLOAD_ENDPOINT = "/api/get_model";

export type GenerationResultFormat =
  | "GLB"
  | "GLTF"
  | "FBX"
  | "USDZ"
  | "STL"
  | string;

type BasePayload = {
  Action: string;
  ResultFormat: GenerationResultFormat;
  UserId: string;
};

export type TextTo3DPayload = BasePayload & {
  Prompt: string;
  ImageBase64?: never;
};

export type ImageTo3DPayload = BasePayload & {
  ImageBase64: string;
  Prompt?: never;
};

export type UploadPayload = TextTo3DPayload | ImageTo3DPayload;

export type ApiSuccess<T = any> = {
  status: "success";
  code?: number;
  message?: string;
  data: T;
};

export type ApiError = {
  status?: "error";
  code?: number;
  message?: string;
};

export type UploadResponse<T = any> = ApiSuccess<T> | ApiError | T;

function validatePayload(payload: UploadPayload) {
  if (!payload) throw new Error("payload 不能为空");
  const hasPrompt =
    typeof (payload as any).Prompt === "string" &&
    (payload as any).Prompt.length > 0;
  const hasImage =
    typeof (payload as any).ImageBase64 === "string" &&
    (payload as any).ImageBase64.length > 0;

  if (hasPrompt === hasImage) {
    throw new Error("Prompt 与 ImageBase64 必须二选一且仅能填写一个");
  }
  if (!(payload as any).Action) throw new Error("Action 不能为空");
  if (!payload.ResultFormat) throw new Error("ResultFormat 不能为空");
  if (!payload.UserId) throw new Error("UserId 不能为空");
}

export async function uploadGeneration<T = any>(payload: UploadPayload) {
  validatePayload(payload);
  return post<UploadResponse<T>>(UPLOAD_ENDPOINT, payload);
}

export async function textTo3D<T = any>(params: {
  action: string;
  prompt: string;
  resultFormat: GenerationResultFormat;
  userId: string;
}) {
  const payload: TextTo3DPayload = {
    Action: params.action,
    Prompt: params.prompt,
    ResultFormat: params.resultFormat,
    UserId: params.userId,
  };
  return uploadGeneration<T>(payload);
}

export async function imageTo3D<T = any>(params: {
  action: string;
  imageBase64: string;
  resultFormat: GenerationResultFormat;
  userId: string;
}) {
  const payload: ImageTo3DPayload = {
    Action: params.action,
    ImageBase64: params.imageBase64,
    ResultFormat: params.resultFormat,
    UserId: params.userId,
  };
  return uploadGeneration<T>(payload);
}
