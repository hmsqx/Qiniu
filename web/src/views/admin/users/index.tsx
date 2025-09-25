import React from "react";
import { useUsers } from "./hooks/useUsers";
import { Pagination } from "@/views/workspace/_modules/workspace-gallery/components/Pagination";
import { Filters } from "./_modules/Filters";
import { UsersTable } from "./_modules/UsersTable";

const AdminUsers: React.FC = () => {
  const {
    list,
    total,
    page,
    pageSize,
    setKeyword,
    loading,
    nextPage,
    prevPage,
    refresh,
    username,
    setUsername,
    email,
    setEmail,
    role,
    setRole,
    setPage,
  } = useUsers();
  const totalPages = Math.max(1, Math.ceil(total / pageSize));
  const handleApply = (s: {
    username: string;
    email: string;
    role: string;
  }) => {
    setUsername(s.username);
    setEmail(s.email);
    setRole(s.role);
    setKeyword("");
    setPage(1);
  };

  return (
    <div className="mx-auto w-full max-w-[1600px] space-y-6 pb-10">
      <Filters
        initial={{ username, email, role }}
        loading={loading}
        onApply={handleApply}
        onRefresh={refresh}
      />

      <UsersTable list={list} loading={loading} />
      <div className="mt-4">
        <Pagination
          total={total}
          pageNum={page}
          totalPages={totalPages}
          loading={loading}
          onPrev={prevPage}
          onNext={nextPage}
          onJump={(p) => setPage(p)}
        />
      </div>
    </div>
  );
};

export default AdminUsers;
