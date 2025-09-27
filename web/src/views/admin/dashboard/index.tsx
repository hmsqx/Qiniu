import React, { useState } from "react";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Tabs, TabsList, TabsTrigger } from "@/components/ui/tabs";
import StatsCards from "./_modules/StatsCards";
import VisitorsChart from "./_modules/VisitorsChart";

const Dashboard: React.FC = () => {
  const [range, setRange] = useState("14d");
  return (
    <div className="space-y-6 ">
      <StatsCards />
      <Card className="panel">
        <CardHeader className="flex flex-row items-start justify-between space-y-0 pb-2">
          <div className="ml-8">
            <CardTitle className="text-base">总使用量</CardTitle>
            <p className="text-xs text-muted-foreground">
              最近 {range === "14d" ? "14 天" : "7 天"}
            </p>
          </div>
          <Tabs value={range} onValueChange={setRange} className="w-auto">
            <TabsList>
              <TabsTrigger value="14d">14天</TabsTrigger>
              <TabsTrigger value="7d">7天</TabsTrigger>
            </TabsList>
          </Tabs>
        </CardHeader>
        <CardContent>
          <VisitorsChart range={range} />
        </CardContent>
      </Card>
    </div>
  );
};

export default Dashboard;
