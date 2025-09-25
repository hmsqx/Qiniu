import React, { useState } from "react";
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
    // legacy keyword (unused when separate fields active)
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
  // draft filter state (controlled for Filters component)
  const [draftFilters, setDraftFilters] = useState({
    username: username,
    email: email,
    role: role,
  });

  const applyFilters = () => {
    setUsername(draftFilters.username.trim());
    setEmail(draftFilters.email.trim());
    setRole(draftFilters.role.trim());
    setKeyword("");
    setPage(1);
  };

  const resetFilters = () => {
    const empty = { username: "", email: "", role: "" };
    setDraftFilters(empty);
    setUsername("");
    setEmail("");
    setRole("");
    setKeyword("");
    setPage(1);
  };

  return (
    <div className="mx-auto w-full max-w-[1600px] space-y-6 pb-10">
      <Filters
        value={draftFilters}
        loading={loading}
        onFiltersChange={(v) => setDraftFilters(v)}
        onSearch={applyFilters}
        onReset={resetFilters}
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
