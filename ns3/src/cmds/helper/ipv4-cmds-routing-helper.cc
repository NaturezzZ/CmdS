/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ipv4-cmds-routing-helper.h"
#include "ns3/log.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/node.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Ipv4CmdSRoutingHelper");

Ipv4CmdSRoutingHelper::Ipv4CmdSRoutingHelper ()
{

}

Ipv4CmdSRoutingHelper::Ipv4CmdSRoutingHelper (const Ipv4CmdSRoutingHelper&)
{

}
  
Ipv4CmdSRoutingHelper* 
Ipv4CmdSRoutingHelper::Copy (void) const
{
  return new Ipv4CmdSRoutingHelper (*this); 
}
  
Ptr<Ipv4RoutingProtocol>
Ipv4CmdSRoutingHelper::Create (Ptr<Node> node) const
{
  Ptr<Ipv4CmdSRouting> CmdSRouting = CreateObject<Ipv4CmdSRouting> ();
  node->AggregateObject (CmdSRouting);
  return CmdSRouting;
}

Ptr<Ipv4CmdSRouting> 
Ipv4CmdSRoutingHelper::GetCmdSRouting (Ptr<Ipv4> ipv4) const
{
  Ptr<Ipv4RoutingProtocol> ipv4rp = ipv4->GetRoutingProtocol ();
  if (DynamicCast<Ipv4CmdSRouting> (ipv4rp))
  {
    NS_LOG_LOGIC ("Ipv4CmdSRouting found as the main IPv4 routing protocol");
    return DynamicCast<Ipv4CmdSRouting> (ipv4rp); 
  }
  return 0;
}

}

