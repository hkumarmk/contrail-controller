/*
 * Copyright (c) 2015 Juniper Networks, Inc. All rights reserved.
 */
#ifndef __AGENT_FLOW_MGMT_REQUEST_H__
#define __AGENT_FLOW_MGMT_REQUEST_H__

#include "pkt/flow_table.h"
#include "pkt/flow_event.h"

////////////////////////////////////////////////////////////////////////////
// Request to the Flow Management module
////////////////////////////////////////////////////////////////////////////
class FlowMgmtRequest {
public:
    enum Event {
        INVALID,
        ADD_FLOW,
        DELETE_FLOW,
        ADD_DBENTRY,
        CHANGE_DBENTRY,
        DELETE_DBENTRY,
        RETRY_DELETE_VRF,
        UPDATE_FLOW_INDEX

    };

    FlowMgmtRequest(Event event, FlowEntryPtr &flow) :
        event_(event), flow_(flow), db_entry_(NULL), vrf_id_(0) {
            if (event == RETRY_DELETE_VRF)
                assert(vrf_id_);
        }

    FlowMgmtRequest(Event event, const DBEntry *db_entry, uint32_t gen_id) :
        event_(event), flow_(NULL), db_entry_(db_entry), vrf_id_(0),
        gen_id_(gen_id) {
            if (event == RETRY_DELETE_VRF) {
                const VrfEntry *vrf = dynamic_cast<const VrfEntry *>(db_entry);
                assert(vrf);
                vrf_id_ = vrf->vrf_id();
            }
        }

    virtual ~FlowMgmtRequest() { }

    // At the end of Flow Management Request, we may enqueue a response message
    // back to FlowTable module. Compute the message type to be enqueued in
    // response. Returns INVALID if no message to be enqueued
    FlowEvent::Event GetResponseEvent() const {
        FlowEvent::Event resp_event = FlowEvent::INVALID;
        if (db_entry_ == NULL)
            return resp_event;

        if (dynamic_cast<const VrfEntry *>(db_entry_)) {
            return resp_event;
        }

        if (event_ == ADD_DBENTRY || event_ == CHANGE_DBENTRY) {
            resp_event = FlowEvent::REVALUATE_DBENTRY;
        } else if (event_ == DELETE_DBENTRY) {
            resp_event = FlowEvent::DELETE_DBENTRY;
        }

        
        const InetUnicastRouteEntry *rt =
            dynamic_cast<const InetUnicastRouteEntry *>(db_entry_);
        if (rt) {
            if (event_ == ADD_DBENTRY || event_ == DELETE_DBENTRY) {
                resp_event = FlowEvent::REVALUATE_FLOW;
            }
        }

        return resp_event;
    }

    Event event() const { return event_; }
    FlowEntryPtr &flow() { return flow_; }
    const DBEntry *db_entry() const { return db_entry_; }
    void set_db_entry(const DBEntry *db_entry) { db_entry_ = db_entry; }
    uint32_t vrf_id() const { return vrf_id_; }
    uint32_t gen_id() const { return gen_id_; }

private:
    Event event_;
    // FlowEntry pointer to hold flow reference till message is processed
    FlowEntryPtr flow_;
    // DBEntry pointer. The DBState from FlowTable module ensures DBEntry is
    // not deleted while message holds pointer
    const DBEntry *db_entry_;
    uint32_t vrf_id_;
    uint32_t gen_id_;

    DISALLOW_COPY_AND_ASSIGN(FlowMgmtRequest);
};

#endif //  __AGENT_FLOW_MGMT_REQUEST_H__
