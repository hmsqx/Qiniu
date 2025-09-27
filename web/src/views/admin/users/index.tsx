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
  const [paging, setPaging] = React.useState(false);
  const totalPages = Math.max(1, Math.ceil(total / pageSize));
  const handleApply = (s: {
    username: string;
    email: string;
    role: string;
  }) => {
    setPaging(true);
    setUsername(s.username);
    setEmail(s.email);
    setRole(s.role);
    setKeyword("");
    setPage(1);
  };

  React.useEffect(() => {
    if (!loading && paging) setPaging(false);
  }, [loading, paging]);

  const handlePrev = () => {
    if (page > 1) {
      setPaging(true);
      prevPage();
    }
  };
  const handleNext = () => {
    if (page < totalPages) {
      setPaging(true);
      nextPage();
    }
  };
  const handleJump = (p: number) => {
    setPaging(true);
    setPage(p);
  };

  return (
    <div className="mx-auto w-full max-w-[1600px] space-y-6 pb-10">
      <Filters
        initial={{ username, email, role }}
        loading={loading}
        onApply={handleApply}
        onRefresh={refresh}
      />

      <UsersTable
        list={list}
        loading={loading}
        forceSkeleton={paging}
        baseIndex={(page - 1) * pageSize}
      />
      <div className="mt-4">
        <Pagination
          total={total}
          pageNum={page}
          totalPages={totalPages}
          loading={loading}
          onPrev={handlePrev}
          onNext={handleNext}
          onJump={handleJump}
        />
      </div>
    </div>
  );
};

export default AdminUsers;
