/*
 * Copyright (c) 2013 Juniper Networks, Inc. All rights reserved.
 */

#include <uve/vn_uve_entry.h>
#include <uve/agent_uve_stats.h>

VnUveEntry::VnUveEntry(Agent *agent, const VnEntry *vn)
    : VnUveEntryBase(agent, vn), port_bitmap_(), inter_vn_stats_(), mutex_(),
      in_bytes_(0), out_bytes_(0), prev_stats_update_time_(0),
      prev_in_bytes_(0), prev_out_bytes_(0) {
}

VnUveEntry::VnUveEntry(Agent *agent)
    : VnUveEntryBase(agent, NULL), port_bitmap_(), inter_vn_stats_(), mutex_(),
      in_bytes_(0), out_bytes_(0), prev_stats_update_time_(0),
      prev_in_bytes_(0), prev_out_bytes_(0) {
}

VnUveEntry::~VnUveEntry() {
}

void VnUveEntry::UpdatePortBitmap(uint8_t proto, uint16_t sport,
                                  uint16_t dport) {
    port_bitmap_.AddPort(proto, sport, dport);
}

void VnUveEntry::UpdateInterVnStats(const string &dst_vn, uint64_t bytes,
                                    uint64_t pkts, bool outgoing) {
    tbb::mutex::scoped_lock lock(mutex_);
    VnStatsPtr key(new VnStats(dst_vn, 0, 0, false));
    VnStatsSet::iterator stats_it = inter_vn_stats_.find(key);
    if (stats_it == inter_vn_stats_.end()) {
        VnStatsPtr stats(new VnStats(dst_vn, bytes, pkts, outgoing));
        inter_vn_stats_.insert(stats);
    } else {
        VnStatsPtr stats_ptr(*stats_it);
        VnStats *stats = stats_ptr.get();
        if (outgoing) {
            stats->out_bytes_ += bytes;
            stats->out_pkts_ += pkts;
        } else {
            stats->in_bytes_ += bytes;
            stats->in_pkts_ += pkts;
        }
    }
    if (outgoing) {
        out_bytes_ += bytes;
    } else {
        in_bytes_ += bytes;
    }
}

void VnUveEntry::ClearInterVnStats() {
    /* Remove all the elements of map entry value which is a set */
    VnStatsSet::iterator stats_it = inter_vn_stats_.begin();
    VnStatsSet::iterator del_it;
    while(stats_it != inter_vn_stats_.end()) {
        del_it = stats_it;
        stats_it++;
        inter_vn_stats_.erase(del_it);
    }
}

bool VnUveEntry::SetVnPortBitmap(UveVirtualNetworkAgent &uve) {
    bool changed = false;

    vector<uint32_t> tcp_sport;
    if (port_bitmap_.tcp_sport_.Sync(tcp_sport)) {
        uve.set_tcp_sport_bitmap(tcp_sport);
        changed = true;
    }

    vector<uint32_t> tcp_dport;
    if (port_bitmap_.tcp_dport_.Sync(tcp_dport)) {
        uve.set_tcp_dport_bitmap(tcp_dport);
        changed = true;
    }

    vector<uint32_t> udp_sport;
    if (port_bitmap_.udp_sport_.Sync(udp_sport)) {
        uve.set_udp_sport_bitmap(udp_sport);
        changed = true;
    }

    vector<uint32_t> udp_dport;
    if (port_bitmap_.udp_dport_.Sync(udp_dport)) {
        uve.set_udp_dport_bitmap(udp_dport);
        changed = true;
    }
    return changed;
}

bool VnUveEntry::UveVnFipCountChanged(int32_t size) const {
    if (!uve_info_.__isset.associated_fip_count) {
        return true;
    }
    if (size != uve_info_.get_associated_fip_count()) {
        return true;
    }
    return false;
}

bool VnUveEntry::UveVnInBandChanged(uint64_t in_band) const {
    if (!uve_info_.__isset.in_bandwidth_usage) {
        return true;
    }
    if (in_band != uve_info_.get_in_bandwidth_usage()) {
        return true;
    }
    return false;
}

bool VnUveEntry::UveVnOutBandChanged(uint64_t out_band) const {
    if (!uve_info_.__isset.out_bandwidth_usage) {
        return true;
    }
    if (out_band != uve_info_.get_out_bandwidth_usage()) {
        return true;
    }
    return false;
}

bool VnUveEntry::UveVnVrfStatsChanged(const vector<UveVrfStats> &vlist) const {
    if (!uve_info_.__isset.vrf_stats_list) {
        return true;
    }
    if (vlist != uve_info_.get_vrf_stats_list()) {
        return true;
    }
    return false;
}

bool VnUveEntry::UveInterVnInStatsChanged(const vector<UveInterVnStats>
                                          &new_list) const {
    if (!uve_info_.__isset.in_stats) {
        return true;
    }
    if (new_list != uve_info_.get_in_stats()) {
        return true;
    }
    return false;
}

bool VnUveEntry::UveVnInFlowCountChanged(uint32_t size) {
    if (!uve_info_.__isset.ingress_flow_count) {
        return true;
    }
    if (size != uve_info_.get_ingress_flow_count()) {
        return true;
    }
    return false;
}

bool VnUveEntry::UveVnOutFlowCountChanged(uint32_t size) {
    if (!uve_info_.__isset.egress_flow_count) {
        return true;
    }
    if (size != uve_info_.get_egress_flow_count()) {
        return true;
    }
    return false;
}

bool VnUveEntry::UveInterVnOutStatsChanged(const vector<UveInterVnStats>
                                           &new_list) const {
    if (!uve_info_.__isset.out_stats) {
        return true;
    }
    if (new_list != uve_info_.get_out_stats()) {
        return true;
    }
    return false;
}

bool VnUveEntry::UpdateVnFlowCount(const VnEntry *vn,
                                   UveVirtualNetworkAgent &s_vn) {
    bool changed = false;
    uint32_t in_count, out_count;
    agent_->pkt()->get_flow_proto()->VnFlowCounters(vn, &in_count, &out_count);
    if (UveVnInFlowCountChanged(in_count)) {
        s_vn.set_ingress_flow_count(in_count);
        uve_info_.set_ingress_flow_count(in_count);
        changed = true;
    }
    if (UveVnOutFlowCountChanged(out_count)) {
        s_vn.set_egress_flow_count(out_count);
        uve_info_.set_egress_flow_count(out_count);
        changed = true;
    }
    return changed;
}

bool VnUveEntry::UpdateVnFipCount(int count, UveVirtualNetworkAgent &s_vn) {
    if (UveVnFipCountChanged(count)) {
        s_vn.set_associated_fip_count(count);
        uve_info_.set_associated_fip_count(count);
        return true;
    }
    return false;
}

bool VnUveEntry::PopulateInterVnStats(UveVirtualNetworkAgent &s_vn) {
    bool changed = false;
    /* Aggregate/total current stats are sent in the following fields */
    vector<UveInterVnStats> in_list;
    vector<UveInterVnStats> out_list;
    /* Only diff since previous dispatch, is sent as part of the following
     * list */
    vector<InterVnStats> vn_stats_list;

    {
        tbb::mutex::scoped_lock lock(mutex_);
        VnStatsSet::iterator it = inter_vn_stats_.begin();
        VnStats *stats;
        VnStatsPtr stats_ptr;
        while (it != inter_vn_stats_.end()) {
            stats_ptr = *it;
            stats = stats_ptr.get();
            UveInterVnStats uve_stats;
            uve_stats.set_other_vn(stats->dst_vn_);

            uve_stats.set_tpkts(stats->in_pkts_);
            uve_stats.set_bytes(stats->in_bytes_);
            in_list.push_back(uve_stats);

            uve_stats.set_tpkts(stats->out_pkts_);
            uve_stats.set_bytes(stats->out_bytes_);
            out_list.push_back(uve_stats);

            InterVnStats diff_stats;
            diff_stats.set_other_vn(stats->dst_vn_);
            diff_stats.set_vrouter(agent_->agent_name());
            diff_stats.set_in_tpkts(stats->in_pkts_ - stats->prev_in_pkts_);
            diff_stats.set_in_bytes(stats->in_bytes_ - stats->prev_in_bytes_);
            diff_stats.set_out_tpkts(stats->out_pkts_ - stats->prev_out_pkts_);
            diff_stats.set_out_bytes(stats->out_bytes_ -
                                     stats->prev_out_bytes_);
            vn_stats_list.push_back(diff_stats);

            stats->prev_in_pkts_ = stats->in_pkts_;
            stats->prev_in_bytes_ = stats->in_bytes_;
            stats->prev_out_pkts_ = stats->out_pkts_;
            stats->prev_out_bytes_ = stats->out_bytes_;
            it++;
        }
    }
    if (!in_list.empty()) {
        if (UveInterVnInStatsChanged(in_list)) {
            s_vn.set_in_stats(in_list);
            uve_info_.set_in_stats(in_list);
            changed = true;
        }
    }
    if (!out_list.empty()) {
        if (UveInterVnOutStatsChanged(out_list)) {
            s_vn.set_out_stats(out_list);
            uve_info_.set_out_stats(out_list);
            changed = true;
        }
    }
    if (!vn_stats_list.empty()) {
        s_vn.set_vn_stats(vn_stats_list);
        changed = true;
    }
    return changed;
}

void VnUveEntry::BuildArpStats(const StatsManager::VrfStats *s,
                               UveVrfStats &vrf_stats) const {
    UveArpStats as;

    UveVmiStats vs;
    UveArpResponses ar1;
    ar1.set_proxies(s->arp_virtual_proxy);
    ar1.set_stitches(s->arp_virtual_stitch);
    ar1.set_floods(s->arp_virtual_flood);
    vs.set_stats(ar1);
    as.set_from_vm_interface(vs);

    UvePhyIntfStats ps;
    UveArpResponses ar2;
    ar2.set_proxies(s->arp_tor_proxy);
    ar2.set_stitches(s->arp_physical_stitch);
    ar2.set_floods(s->arp_physical_flood);
    ps.set_stats(ar2);
    as.set_from_physical_interface(ps);

    vrf_stats.set_arp_packet_counts(as);
}

void VnUveEntry::BuildNhStats(const StatsManager::VrfStats *s,
                              UveVrfStats &vrf_stats) const {
    UveNhStats nhs;
    nhs.set_discards(s->discards);
    nhs.set_resolves(s->resolves);
    nhs.set_l3_receives(s->receives);
    nhs.set_l2_receives(s->l2_receives);
    nhs.set_local_vm_l3_forwards(s->encaps);
    nhs.set_local_vm_l2_forwards(s->l2_encaps);
    nhs.set_vrf_translates(s->vrf_translates);
    nhs.set_ecmp_forwards(s->ecmp_composites);

    UveTunnelNhStats tns;
    tns.set_udp_encaps(s->udp_tunnels);
    tns.set_mpls_over_udp_encaps(s->udp_mpls_tunnels);
    tns.set_mpls_over_gre_encaps(s->gre_mpls_tunnels);
    tns.set_vxlan_encaps(s->vxlan_tunnels);
    nhs.set_tunnel_nh_stats(tns);

    UveMulticastNhStats mns;
    mns.set_source_replication_forwards(s->evpn_composites);
    mns.set_edge_replication_forwards(s->fabric_composites);
    mns.set_local_vm_l3_forwards(s->encap_composites);
    mns.set_total_multicast_forwards(s->l2_mcast_composites);
    nhs.set_comp_nh_stats(mns);
}

bool VnUveEntry::FillVrfStats(int vrf_id, UveVirtualNetworkAgent &s_vn) {
    bool changed = false;
    UveVrfStats vrf_stats;
    vector<UveVrfStats> vlist;

    AgentUveStats *uve = static_cast<AgentUveStats *>(agent_->uve());
    StatsManager::VrfStats *s = uve->stats_manager()->GetVrfStats(vrf_id);
    if (s != NULL) {
        vrf_stats.set_name(s->name);
        vrf_stats.set_diag_packet_count(s->diags);
        vrf_stats.set_unknown_unicast_floods(s->uuc_floods);

        BuildArpStats(s, vrf_stats);

        UveOffloadStats os;
        os.set_gro(s->gros);
        vrf_stats.set_offload_packet_counts(os);

        BuildNhStats(s, vrf_stats);

        vlist.push_back(vrf_stats);
        if (UveVnVrfStatsChanged(vlist)) {
            s_vn.set_vrf_stats_list(vlist);
            uve_info_.set_vrf_stats_list(vlist);
            changed = true;
        }
    }
    return changed;
}

bool VnUveEntry::UpdateVrfStats(const VnEntry *vn,
                                UveVirtualNetworkAgent &s_vn) {
    bool changed = false;
    VrfEntry *vrf = vn->GetVrf();
    if (vrf) {
        changed = FillVrfStats(vrf->vrf_id(), s_vn);
    } else {
        vector<UveVrfStats> vlist;
        if (UveVnVrfStatsChanged(vlist)) {
            s_vn.set_vrf_stats_list(vlist);
            uve_info_.set_vrf_stats_list(vlist);
            changed = true;
        }
    }

    return changed;
}

bool VnUveEntry::FrameVnStatsMsg(const VnEntry *vn,
                                 UveVirtualNetworkAgent &uve,
                                 bool only_vrf_stats) {
    bool changed = false;
    assert(!deleted_);
    uve.set_name(vn->GetName());

    if (UpdateVrfStats(vn, uve)) {
        changed = true;
    }

    if (only_vrf_stats) {
        return changed;
    }

    int fip_count = 0;
    InterfaceSet::iterator it = interface_tree_.begin();
    while (it != interface_tree_.end()) {
        const Interface *intf = *it;
        ++it;

        const VmInterface *vm_port = static_cast<const VmInterface *>(intf);
        fip_count += vm_port->GetFloatingIpCount();
    }

    uint64_t diff_in_bytes = 0;
    uint64_t diff_out_bytes = 0;

    uint64_t diff_seconds = 0;
    uint64_t cur_time = UTCTimestampUsec();
    bool send_bandwidth = false;
    uint64_t in_band, out_band;
    uint64_t b_intvl = agent_->uve()->bandwidth_intvl();
    if (prev_stats_update_time_ == 0) {
        in_band = out_band = 0;
        send_bandwidth = true;
        prev_stats_update_time_ = cur_time;
    } else {
        diff_seconds = (cur_time - prev_stats_update_time_) / b_intvl;
        if (diff_seconds > 0) {
            diff_in_bytes = in_bytes_ - prev_in_bytes_;
            diff_out_bytes = out_bytes_ - prev_out_bytes_;
            in_band = (diff_in_bytes * 8)/diff_seconds;
            out_band = (diff_out_bytes * 8)/diff_seconds;
            prev_stats_update_time_ = cur_time;
            prev_in_bytes_ = in_bytes_;
            prev_out_bytes_ = out_bytes_;
            send_bandwidth = true;
        }
    }
    if (send_bandwidth && UveVnInBandChanged(in_band)) {
        uve.set_in_bandwidth_usage(in_band);
        uve_info_.set_in_bandwidth_usage(in_band);
        changed = true;
    }

    if (send_bandwidth && UveVnOutBandChanged(out_band)) {
        uve.set_out_bandwidth_usage(out_band);
        uve_info_.set_out_bandwidth_usage(out_band);
        changed = true;
    }

    if (UpdateVnFlowCount(vn, uve)) {
        changed = true;
    }

    if (UpdateVnFipCount(fip_count, uve)) {
        changed = true;
    }

    /* VM interface list for VN is sent whenever any VM is added or
     * removed from VN. That message has only two fields set - vn name
     * and virtualmachine_list */

    if (PopulateInterVnStats(uve)) {
        changed = true;
    }

    if (SetVnPortBitmap(uve)) {
        changed = true;
    }

    return changed;
}

void VnUveEntry::Reset() {
    VnUveEntryBase::Reset();
    port_bitmap_.Reset();
    inter_vn_stats_.clear();
    prev_stats_update_time_ = 0;
    prev_in_bytes_ = 0;
    prev_out_bytes_ = 0;
}
