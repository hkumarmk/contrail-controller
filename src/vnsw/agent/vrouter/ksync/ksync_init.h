/*
 * Copyright (c) 2013 Juniper Networks, Inc. All rights reserved.
 */

#ifndef vnsw_agent_ksync_init_h
#define vnsw_agent_ksync_init_h

#include <vrouter/ksync/flowtable_ksync.h>
#include <vrouter/ksync/mpls_ksync.h>
#include <vrouter/ksync/nexthop_ksync.h>
#include <vrouter/ksync/mirror_ksync.h>
#include <vrouter/ksync/route_ksync.h>
#include <vrouter/ksync/vxlan_ksync.h>
#include <vrouter/ksync/vrf_assign_ksync.h>
#include <vrouter/ksync/interface_scan.h>
#include "vnswif_listener.h"

class KSyncFlowMemory;
class FlowTableKSyncObject;

class KSync {
public:
    KSync(Agent *agent);
    virtual ~KSync();

    virtual void Init(bool create_vhost);
    virtual void InitDone();
    virtual void RegisterDBClients(DB *db);
    void VnswInterfaceListenerInit();
    void Shutdown();

    void UpdateVhostMac();
    Agent *agent() const  { return agent_; }
    MirrorKSyncObject *mirror_ksync_obj() const { 
        return mirror_ksync_obj_.get(); 
    }
    NHKSyncObject *nh_ksync_obj() const {
        return nh_ksync_obj_.get();
    }
    InterfaceKSyncObject *interface_ksync_obj() const {
        return interface_ksync_obj_.get();
    }
    VrfKSyncObject *vrf_ksync_obj() const {
        return vrf_ksync_obj_.get();
    }
    FlowTableKSyncObject *flow_table_ksync_obj(uint16_t index) const {
        return flow_table_ksync_obj_list_[index];
    }
    InterfaceKScan *interface_scanner() const {
        return interface_scanner_.get();
    }
    VnswInterfaceListener *vnsw_interface_listner() const  {
        return vnsw_interface_listner_.get();
    }
    KSyncFlowMemory *ksync_flow_memory() const  {
        return ksync_flow_memory_.get();
    }
protected:
    Agent *agent_;
    boost::scoped_ptr<InterfaceKSyncObject> interface_ksync_obj_; 
    std::vector<FlowTableKSyncObject *> flow_table_ksync_obj_list_;
    boost::scoped_ptr<MplsKSyncObject> mpls_ksync_obj_; 
    boost::scoped_ptr<NHKSyncObject> nh_ksync_obj_; 
    boost::scoped_ptr<MirrorKSyncObject> mirror_ksync_obj_; 
    boost::scoped_ptr<VrfKSyncObject> vrf_ksync_obj_;
    boost::scoped_ptr<VxLanKSyncObject> vxlan_ksync_obj_;
    boost::scoped_ptr<VrfAssignKSyncObject> vrf_assign_ksync_obj_;
    boost::scoped_ptr<InterfaceKScan> interface_scanner_;
    boost::scoped_ptr<VnswInterfaceListener> vnsw_interface_listner_;
    boost::scoped_ptr<KSyncFlowMemory> ksync_flow_memory_;
    virtual void InitFlowMem();
    void VRouterInterfaceSnapshot();
    void ResetVRouter(bool run_sync_mode);
    int Encode(Sandesh &encoder, uint8_t *buf, int buf_len);
private:
    void NetlinkInit();
    void CreateVhostIntf();
    DISALLOW_COPY_AND_ASSIGN(KSync);
};

class KSyncTcp : public KSync {
public:
    KSyncTcp(Agent *agent);
    virtual ~KSyncTcp();
    virtual void Init(bool create_vhost);
    void TcpInit();
protected:
    virtual void InitFlowMem();
};

int GenenericNetlinkFamily();
void GenericNetlinkInit();

#endif //vnsw_agent_ksync_init_h
