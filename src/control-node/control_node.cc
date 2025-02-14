/*
 * Copyright (c) 2013 Juniper Networks, Inc. All rights reserved.
 */

#include "control_node.h"
#include <boost/assign.hpp>
#include "base/task.h"

std::string ControlNode::hostname_;
std::string ControlNode::prog_name_;
std::string ControlNode::self_ip_;
DiscoveryServiceClient* ControlNode::ds_client_;
bool ControlNode::test_mode_;

//
// Default scheduler policy for control-node daemon and test processes.
//
void ControlNode::SetDefaultSchedulingPolicy() {
    static bool policy_set;

    if (policy_set)
        return;
    policy_set = true;

    TaskScheduler *scheduler = TaskScheduler::GetInstance();

    // Policy for bgp::Config Task.
    TaskPolicy config_policy = boost::assign::list_of
        (TaskExclusion(scheduler->GetTaskId("bgp::Config")))
        (TaskExclusion(scheduler->GetTaskId("bgp::RTFilter")))
        (TaskExclusion(scheduler->GetTaskId("bgp::SendTask")))
        (TaskExclusion(scheduler->GetTaskId("bgp::ServiceChain")))
        (TaskExclusion(scheduler->GetTaskId("bgp::StateMachine")))
        (TaskExclusion(scheduler->GetTaskId("bgp::PeerMembership")))
        (TaskExclusion(scheduler->GetTaskId("db::DBTable")))
        (TaskExclusion(scheduler->GetTaskId("io::ReaderTask")))
        (TaskExclusion(scheduler->GetTaskId("ifmap::StateMachine")))
        (TaskExclusion(scheduler->GetTaskId("xmpp::StateMachine")))
        (TaskExclusion(scheduler->GetTaskId("timer::TimerTask")))
        (TaskExclusion(scheduler->GetTaskId("bgp::ShowCommand")))
        (TaskExclusion(scheduler->GetTaskId("bgp::SendReadyTask")))
        (TaskExclusion(scheduler->GetTaskId("bgp::StaticRoute")))
        (TaskExclusion(scheduler->GetTaskId("bgp::ResolverPath")))
        (TaskExclusion(scheduler->GetTaskId("bgp::ResolverNexthop")));
    scheduler->SetPolicy(scheduler->GetTaskId("bgp::Config"), config_policy);

    // Policy for bgp::ServiceChain and bgp::StaticRoute Tasks.
    TaskPolicy static_service_chain_policy = boost::assign::list_of
        (TaskExclusion(scheduler->GetTaskId("bgp::Config")))
        (TaskExclusion(scheduler->GetTaskId("bgp::PeerMembership")))
        (TaskExclusion(scheduler->GetTaskId("bgp::ServiceChain")))
        (TaskExclusion(scheduler->GetTaskId("bgp::StaticRoute")))
        (TaskExclusion(scheduler->GetTaskId("db::DBTable")));
    scheduler->SetPolicy(scheduler->GetTaskId("bgp::ServiceChain"),
        static_service_chain_policy);
    scheduler->SetPolicy(scheduler->GetTaskId("bgp::StaticRoute"),
        static_service_chain_policy);


    // Policy for bgp::StateMachine and xmpp::StateMachine Tasks.
    // There should be exclusion between Reader and StateMachine
    // tasks with the same index only (as opposed to all indices).
    TaskPolicy sm_policy = boost::assign::list_of
        (TaskExclusion(scheduler->GetTaskId("io::ReaderTask")));
    scheduler->SetPolicy(scheduler->GetTaskId("bgp::StateMachine"),
        sm_policy);
    scheduler->SetPolicy(scheduler->GetTaskId("xmpp::StateMachine"),
        sm_policy);

    // Policy for bgp::PeerMembership Task.
    TaskPolicy peer_membership_policy = boost::assign::list_of
        (TaskExclusion(scheduler->GetTaskId("db::DBTable")))
        (TaskExclusion(scheduler->GetTaskId("bgp::SendTask")))
        (TaskExclusion(scheduler->GetTaskId("bgp::ServiceChain")))
        (TaskExclusion(scheduler->GetTaskId("bgp::ShowCommand")))
        (TaskExclusion(scheduler->GetTaskId("bgp::StateMachine")))
        (TaskExclusion(scheduler->GetTaskId("bgp::StaticRoute")))
        (TaskExclusion(scheduler->GetTaskId("xmpp::StateMachine")));
    scheduler->SetPolicy(scheduler->GetTaskId("bgp::PeerMembership"),
        peer_membership_policy);

    // Policy for bgp::SendReadyTask Task.
    TaskPolicy send_ready_policy = boost::assign::list_of
        (TaskExclusion(scheduler->GetTaskId("bgp::SendTask")))
        (TaskExclusion(scheduler->GetTaskId("bgp::PeerMembership")));
    scheduler->SetPolicy(scheduler->GetTaskId("bgp::SendReadyTask"),
        send_ready_policy);

    // Policy for bgp::RTFilter Task.
    TaskPolicy rtfilter_policy = boost::assign::list_of
        (TaskExclusion(scheduler->GetTaskId("db::DBTable")))
        (TaskExclusion(scheduler->GetTaskId("bgp::StateMachine")))
        (TaskExclusion(scheduler->GetTaskId("bgp::RTFilter")))
        (TaskExclusion(scheduler->GetTaskId("bgp::Config")));
    scheduler->SetPolicy(scheduler->GetTaskId("bgp::RTFilter"),
        rtfilter_policy);

    // Policy for bgp::ResolverPath Task.
    TaskPolicy resolver_path_policy = boost::assign::list_of
        (TaskExclusion(scheduler->GetTaskId("db::DBTable")))
        (TaskExclusion(scheduler->GetTaskId("bgp::Config")))
        (TaskExclusion(scheduler->GetTaskId("bgp::ResolverNexthop")));
    scheduler->SetPolicy(scheduler->GetTaskId("bgp::ResolverPath"),
        resolver_path_policy);

    // Policy for bgp::ResolverNexthop Task.
    TaskPolicy resolver_nexthop_policy = boost::assign::list_of
        (TaskExclusion(scheduler->GetTaskId("db::DBTable")))
        (TaskExclusion(scheduler->GetTaskId("bgp::Config")))
        (TaskExclusion(scheduler->GetTaskId("bgp::ResolverPath")));
    scheduler->SetPolicy(scheduler->GetTaskId("bgp::ResolverNexthop"),
        resolver_nexthop_policy);
}
