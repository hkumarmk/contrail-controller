/*
 * Copyright (c) 2015 Juniper Networks, Inc. All rights reserved.
 */

#ifndef __src_vnsw_agent_vrouter_ksync_ksync_flow_memory_h
#define __src_vnsw_agent_vrouter_ksync_ksync_flow_memory_h

/*
 * Module responsible to manage the VRouter memory mapped to agent
 */
#include <list>

class Timer;
class KSync;
class FlowKey;
struct vr_flow_entry;
struct vr_flow_entry;

class KSyncFlowMemory {
public:
    static const uint32_t kAuditYieldTimer = 500;         // in msec
    static const uint32_t kAuditTimeout = 2000;           // in msec
    static const int kAuditYield = 1024;

    KSyncFlowMemory(KSync *ksync);
    ~KSyncFlowMemory();

    void Init();
    void InitFlowMem();
    void InitTest();
    void Shutdown();

    const vr_flow_entry *GetKernelFlowEntry(uint32_t idx,
                                            bool ignore_active_status);
    bool GetFlowKey(uint32_t index, FlowKey *key);

    uint32_t flow_table_entries_count() { return flow_table_entries_count_; }

    bool AuditProcess();
    void MapFlowMem();
    void UnmapFlowMemTest();
    void MapSharedMemory();
    void GetFlowTableSize();

    KSync *ksync() const { return ksync_; }
    void set_major_devid(int id) { major_devid_ = id; }
    void set_flow_table_size(int count) { flow_table_size_ = count; }
    void set_flow_table_path(const std::string &path) {
        flow_table_path_ = path;
    }
private:
    KSync                   *ksync_;
    vr_flow_entry           *flow_table_;
    // Name of file used to map flow table
    std::string             flow_table_path_;
    // major dev-id on linux based implementation
    int                     major_devid_;
    // Size of flow table memory
    int                     flow_table_size_;
    // Count of entries in flow-table
    uint32_t                flow_table_entries_count_;
    // Audit related entries
    Timer                   *audit_timer_;
    uint32_t                audit_timeout_;
    int                     audit_yield_;
    uint32_t                audit_flow_idx_;
    uint64_t                audit_timestamp_;
    std::list<std::pair<uint32_t, uint64_t> > audit_flow_list_;
};

#endif //  __src_vnsw_agent_vrouter_ksync_ksync_flow_memory_h
