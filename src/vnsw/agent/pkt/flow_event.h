/*
 * Copyright (c) 2015 Juniper Networks, Inc. All rights reserved.
 */
#ifndef __AGENT_FLOW_EVENT_H__
#define __AGENT_FLOW_EVENT_H__

#include "flow_table.h"

////////////////////////////////////////////////////////////////////////////
// Control events for flow management
////////////////////////////////////////////////////////////////////////////
class FlowEvent {
public:
    enum Event {
        INVALID,
        // Flow add message from VRouter
        VROUTER_FLOW_MSG,
        // Event to delete a flow entry
        DELETE_FLOW,
        // Event by audit module to delete a flow
        AUDIT_FLOW,
        // Revaluate flow due to deletion of a DBEntry. Other than for INET
        // routes, delete of a DBEntry will result in deletion of flows using
        // the DBEntry
        DELETE_DBENTRY,
        // Revaluate route due to change in a DBEntry. This event is used to
        // revaluate a flow on add/change of interface, vm, vn etc...
        // The action typically invovles only re-evaluating ACL lookup actions
        REVALUATE_DBENTRY,
        // Add/Delete of route can result in flow using a next higher/lower
        // prefix. The event will re-valuate the complete flow due to change
        // in route used for flow
        REVALUATE_FLOW,
        // Flow entry should be freed from kTaskFlowEvent task context.
        // Event to ensure flow entry is freed from right context
        FREE_FLOW_REF,
        // A DBEntry should be freed from kTaskFlowEvent task context.
        // Event to ensure DBEntry entry reference is freed from right context
        FREE_DBENTRY
    };

    FlowEvent() :
        event_(INVALID), flow_(NULL), pkt_info_(), db_entry_(NULL),
        gen_id_(0), del_rev_flow_(false) {
    }

    FlowEvent(Event event, FlowEntry *flow) :
        event_(event), flow_(flow), pkt_info_(), db_entry_(NULL),
        gen_id_(0), del_rev_flow_(false) {
    }

    FlowEvent(Event event, FlowEntry *flow, const DBEntry *db_entry) :
        event_(event), flow_(flow), pkt_info_(), db_entry_(db_entry),
        gen_id_(0), del_rev_flow_(false) {
    }

    FlowEvent(Event event, const DBEntry *db_entry, uint32_t gen_id) :
        event_(event), flow_(NULL), pkt_info_(), db_entry_(db_entry),
        gen_id_(gen_id), del_rev_flow_(false) {
    }

    FlowEvent(Event event, const FlowKey &key, bool del_rev_flow) :
        event_(event), flow_(NULL), pkt_info_(), db_entry_(NULL),
        gen_id_(0), flow_key_(key), del_rev_flow_(del_rev_flow) {
    }

    FlowEvent(Event event, PktInfoPtr pkt_info) :
        event_(event), flow_(NULL), pkt_info_(pkt_info), db_entry_(NULL),
        gen_id_(0), flow_key_(), del_rev_flow_() {
    }

    FlowEvent(const FlowEvent &rhs) :
        event_(rhs.event_), flow_(rhs.flow()), pkt_info_(rhs.pkt_info_),
        db_entry_(rhs.db_entry_), gen_id_(rhs.gen_id_),
        flow_key_(rhs.flow_key_), del_rev_flow_(rhs.del_rev_flow_) {
    }

    virtual ~FlowEvent() { }

    Event event() const { return event_; }
    FlowEntry *flow() const { return flow_.get(); }
    void set_flow(FlowEntry *flow) { flow_ = flow; }
    const DBEntry *db_entry() const { return db_entry_; }
    void set_db_entry(const DBEntry *db_entry) { db_entry_ = db_entry; }
    uint32_t gen_id() const { return gen_id_; }
    const FlowKey &get_flow_key() const { return flow_key_; }
    bool get_del_rev_flow() const { return del_rev_flow_; }
    PktInfoPtr pkt_info() const { return pkt_info_; }

private:
    Event event_;
    FlowEntryPtr flow_;
    PktInfoPtr pkt_info_;
    const DBEntry *db_entry_;
    uint32_t gen_id_;
    FlowKey flow_key_;
    bool del_rev_flow_;
};

#endif //  __AGENT_FLOW_EVENT_H__
