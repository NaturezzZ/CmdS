#ifndef NS3_IPV4_CMDS_ROUTING_TABLE
#define NS3_IPV4_CMDS_ROUTING_TABLE

#include "ns3/ptr.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-route.h"

#include <map>
#include <vector>



namespace ns3 {

class Ipv4CmdSRoutingTableEntry {
public:
  typedef std::pair<uint32_t, Ipv4Address> RouteEntry;

  Ipv4CmdSRoutingTableEntry (Ptr<Ipv4> ipv4, Ipv4Address dest, Ipv4Mask mask, const std::vector<uint32_t> &interfaces);

  static Ptr<Ipv4Route> ConstructIpv4Route (Ptr<Ipv4> ipv4, uint32_t interface, Ipv4Address dest);

  Ptr<Ipv4Route> GetRoute (uint32_t flowid, Ipv4Address dest, Ptr<Ipv4> ipv4,
    const std::map<Ipv4Address, uint32_t> &queueSizeMap, bool best);

  bool IsMatch (Ipv4Address dest) const;

  bool operator< (const Ipv4CmdSRoutingTableEntry &oth) const;

private:
  Ipv4Address m_dest;
  Ipv4Mask m_mask;

  std::vector<RouteEntry> m_routes;
  std::map<uint32_t, uint32_t> m_current; // from flowid to interface
};

class Ipv4CmdSRoutingTable {
public:
  Ipv4CmdSRoutingTable ();

  Ptr<Ipv4Route> Lookup (Ptr<Ipv4> ipv4, uint32_t flowid, Ipv4Address dest, bool best = false);

  void UpdateQueueSize (Ipv4Address neighbor, uint32_t queueSize);

  void AddEntry (Ptr<Ipv4> ipv4, Ipv4Address dest, Ipv4Mask mask, const std::vector<uint32_t> &interfaces);

private:
  typedef std::vector<Ipv4CmdSRoutingTableEntry>::iterator table_iterator;

  std::map<Ipv4Address, uint32_t> m_queueSizeMap;

  std::vector<Ipv4CmdSRoutingTableEntry> m_table;

  bool sorted;
};

}

#endif