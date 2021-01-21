/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef IPV4_CMDS_ROUTING_HELPER_H
#define IPV4_CMDS_ROUTING_HELPER_H

#include "ns3/ipv4-cmds-routing.h"
#include "ns3/ipv4-routing-helper.h"

namespace ns3 {

class Ipv4CmdSRoutingHelper : public Ipv4RoutingHelper
{
public:
  Ipv4CmdSRoutingHelper ();
  Ipv4CmdSRoutingHelper (const Ipv4CmdSRoutingHelper&);
  
  Ipv4CmdSRoutingHelper* Copy (void) const;
  
  virtual Ptr<Ipv4RoutingProtocol> Create (Ptr<Node> node) const;

  Ptr<Ipv4CmdSRouting> GetCmdSRouting (Ptr<Ipv4> ipv4) const;
};

}

#endif /* IPV4_CmdS_ROUTING_HELPER_H */

