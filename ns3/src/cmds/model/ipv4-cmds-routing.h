/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef IPV4_CMDS_ROUTING_H
#define IPV4_CMDS_ROUTING_H

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-route.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/ipv4-header.h"
#include "ns3/data-rate.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"

#include "ns3/queue.h"
#include "ns3/ptr.h"

#include "ns3/ipv4-cmds-routing-table.h"

#include <map>
#include <vector>

namespace ns3 {

class Ipv4CmdSRouting : public Ipv4RoutingProtocol
{
public:
  Ipv4CmdSRouting ();
  ~Ipv4CmdSRouting ();

  static TypeId GetTypeId (void);

  void AddRoute (Ipv4Address network, Ipv4Mask networkMask, std::vector<uint32_t> interfaces);
  void AddQueue (Ptr<Queue> queue);

  void HandleMessage (Ptr<const Packet> p, const Ipv4Header &header);
  void SendMessage (uint32_t npkt);

  void Start ();

  //void SetSyncPeriod ();

  void SyncQueueSize ();

  /* Inherit From Ipv4RoutingProtocol */
  virtual Ptr<Ipv4Route> RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);
  virtual bool RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                           UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                           LocalDeliverCallback lcb, ErrorCallback ecb);
  virtual void NotifyInterfaceUp (uint32_t interface);
  virtual void NotifyInterfaceDown (uint32_t interface);
  virtual void NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void SetIpv4 (Ptr<Ipv4> ipv4);
  virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const;

  virtual void DoDispose (void);

private:

  Ipv4CmdSRoutingTable m_rtable;
  
  Time m_syncPeriod;
  EventId m_syncEvent;

  Ptr<Socket> m_socket;

  Ptr<Ipv4> m_ipv4;

  std::vector<Ptr<Queue> > m_queues;
};

}

#endif /* CmdS_ROUTING_H */

