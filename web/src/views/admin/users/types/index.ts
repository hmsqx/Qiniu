// Shared types for admin users view
// Re-export API layer UserItem (so view-level components depend on this facade)
export type { UserItem } from "@/api/users";

export interface UsersFilters {
  username: string;
  email: string;
  role: string; // empty means all
}
