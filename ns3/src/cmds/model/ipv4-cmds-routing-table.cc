#include "ipv4-cmds-routing-table.h"
#include "ns3/simulator.h"
#include "ns3/net-device.h"
#include "ns3/channel.h"
#include "ns3/node.h"
#include "ns3/log.h"
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Ipv4CmdSRoutingTable");

Ipv4CmdSRoutingTableEntry::Ipv4CmdSRoutingTableEntry (Ptr<Ipv4> ipv4, Ipv4Address dest, Ipv4Mask mask, 
  const std::vector<uint32_t> &interfaces) : m_dest(dest), m_mask(mask)
{
  for (std::vector<uint32_t>::const_iterator it = interfaces.begin (); it != interfaces.end (); ++it)
    {
      uint32_t interface = *it;
      Ptr<NetDevice> dev = ipv4->GetNetDevice (interface);
      Ptr<Channel> channel = dev->GetChannel ();
      uint32_t otherEnd = (channel->GetDevice (0) == dev) ? 1 : 0;
      Ptr<Node> nextHop = channel->GetDevice (otherEnd)->GetNode ();
      uint32_t nextIf = channel->GetDevice (otherEnd)->GetIfIndex ();
      Ipv4Address nextHopAddr = nextHop->GetObject<Ipv4> ()->GetAddress (nextIf,0).GetLocal ();

      m_routes.push_back (RouteEntry (interface, nextHopAddr));
    }
}

bool Ipv4CmdSRoutingTableEntry::IsMatch (Ipv4Address dest) const
{
  return m_mask.IsMatch (m_dest, dest);
}

Ptr<Ipv4Route>
Ipv4CmdSRoutingTableEntry::ConstructIpv4Route (Ptr<Ipv4> ipv4, uint32_t interface, Ipv4Address dest)
{
  Ptr<NetDevice> dev = ipv4->GetNetDevice (interface);
  Ptr<Channel> channel = dev->GetChannel ();

  uint32_t otherEnd = (channel->GetDevice (0) == dev) ? 1 : 0;
  Ptr<Node> nextHop = channel->GetDevice (otherEnd)->GetNode ();
  uint32_t nextIf = channel->GetDevice (otherEnd)->GetIfIndex ();
  Ipv4Address nextHopAddr = nextHop->GetObject<Ipv4> ()->GetAddress (nextIf,0).GetLocal ();
  Ptr<Ipv4Route> route = Create<Ipv4Route> ();
  route->SetOutputDevice (ipv4->GetNetDevice (interface));
  route->SetGateway (nextHopAddr);
  route->SetSource (ipv4->GetAddress (interface, 0).GetLocal ());
  route->SetDestination (dest);
  
  //NS_LOG_LOGIC ("Forward " << dest << " to " << nextHopAddr);
  
  
  return route;
}

bool Ipv4CmdSRoutingTableEntry::operator< (const Ipv4CmdSRoutingTableEntry &oth) const
{
  return m_mask.GetPrefixLength () > oth.m_mask.GetPrefixLength ();
}

Ptr<Ipv4Route> Ipv4CmdSRoutingTableEntry::GetRoute (uint32_t flowid, Ipv4Address dest, Ptr<Ipv4> ipv4,
  const std::map<Ipv4Address, uint32_t> &queueSizeMap, bool chbest)
{
  
  if (!chbest)
    {
      std::map<uint32_t, uint32_t>::iterator it = m_current.find (flowid);

      if (it != m_current.end())
        {
          return Ipv4CmdSRoutingTableEntry::ConstructIpv4Route (ipv4, it->second, dest);
        }
    }
    
    NS_LOG_DEBUG (this << " getroute " << flowid);

    std::vector<uint32_t> best_choices;
    std::vector<RouteEntry>::iterator it = m_routes.begin();
    best_choices.push_back (it->first);
    std::map<Ipv4Address, uint32_t>::const_iterator mit = queueSizeMap.find (it->second);
    uint32_t bestVal = mit == queueSizeMap.end () ? UINT32_MAX : mit->second; 


    if (bestVal != UINT32_MAX) 
      NS_LOG_DEBUG ("Best: " << it->second << " " << bestVal << " " << best_choices.size ());
    for (++it; it != m_routes.end (); ++it)
      {
        mit = queueSizeMap.find (it->second);
        uint32_t curVal = mit == queueSizeMap.end () ? UINT32_MAX : mit->second;
        

        if (curVal < bestVal)
          {
            best_choices.clear();
            best_choices.push_back (it->first);
            bestVal = curVal;
          }
        else if (curVal == bestVal)
          best_choices.push_back (it->first);
        
        if (curVal != UINT32_MAX) 
          NS_LOG_DEBUG ("Best: " << it->second << " " << curVal<< " " << best_choices.size ());
      }
    
    

    uint32_t choice = best_choices[rand() % best_choices.size ()];

    /*std::map<Ipv4Address, uint32_t>::iterator _it = m_current.find (flowid);

    if (_it != m_current.end ())
      {
        NS_LOG_DEBUG ("Switch from " << _it->second << " to " << choice);
      }*/

    m_current[flowid] = choice;
    return Ipv4CmdSRoutingTableEntry::ConstructIpv4Route (ipv4, choice, dest);
}

Ipv4CmdSRoutingTable::Ipv4CmdSRoutingTable() : sorted (true) {}

void Ipv4CmdSRoutingTable::UpdateQueueSize (Ipv4Address neighbor, uint32_t queueSize)
{
  m_queueSizeMap[neighbor] = queueSize;
}

void Ipv4CmdSRoutingTable::AddEntry (Ptr<Ipv4> ipv4, Ipv4Address dest, Ipv4Mask mask, 
  const std::vector<uint32_t> &interfaces)
{
  m_table.push_back (Ipv4CmdSRoutingTableEntry (ipv4, dest, mask, interfaces));
  sorted = false;
}

Ptr<Ipv4Route> Ipv4CmdSRoutingTable::Lookup (Ptr<Ipv4> ipv4, uint32_t flowid, Ipv4Address dest, bool best) 
{
  if (!sorted)
    {
      std::sort (m_table.begin (), m_table.end ());
      sorted = true;
    }

  for (table_iterator i = m_table.begin (); i != m_table.end (); i++) 
    if (i->IsMatch (dest))
        return i->GetRoute (flowid, dest, ipv4, m_queueSizeMap, best);        
  return 0;
}

}