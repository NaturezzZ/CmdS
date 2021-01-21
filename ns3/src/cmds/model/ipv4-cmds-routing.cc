/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ipv4-cmds-routing.h"

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/net-device.h"
#include "ns3/channel.h"
#include "ns3/node.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/flow-id-tag.h"

#include "ns3/ipv4-cmds-tag.h"

#include <algorithm>

#define LOOPBACK_PORT 0

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Ipv4CmdSRouting");

NS_OBJECT_ENSURE_REGISTERED (Ipv4CmdSRouting);

Ipv4CmdSRouting::Ipv4CmdSRouting ()
    // Parameters
    : m_syncPeriod (MilliSeconds (10)),
    m_syncEvent (),
    m_ipv4 (0)
{
  NS_LOG_FUNCTION (this);
}

Ipv4CmdSRouting::~Ipv4CmdSRouting ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
Ipv4CmdSRouting::GetTypeId (void)
{
  static TypeId tid = TypeId("ns3::Ipv4CmdSRouting")
      .SetParent<Object>()
      .SetGroupName ("Internet")
      .AddConstructor<Ipv4CmdSRouting> ();

  return tid;
}

Ptr<Ipv4Route>
Ipv4CmdSRouting::RouteOutput (Ptr<Packet> packet, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
  NS_LOG_ERROR (this << " CmdS routing is not support for local routing output");
  return 0;
}

void
Ipv4CmdSRouting::HandleMessage (Ptr<const Packet> packet, const Ipv4Header &header)
{
  NS_LOG_DEBUG (GetObject<Node> () << " HandleMessage");
  
  Ipv4CmdSTag tag;
  packet->PeekPacketTag (tag);

  uint32_t queueSize = tag.GetQueueSize ();

  NS_ASSERT (queueSize != Ipv4CmdSTag::DO_SWITCH && queueSize != Ipv4CmdSTag::NO_SWITCH);

  Ipv4Address dest = header.GetDestination ();
  m_rtable.UpdateQueueSize (dest, queueSize);
}

void
Ipv4CmdSRouting::AddRoute (Ipv4Address network, Ipv4Mask networkMask, std::vector<uint32_t> interfaces)
{
  NS_LOG_LOGIC (GetObject<Node> () << " AddRoute: " << network << " " << networkMask);
  m_rtable.AddEntry (m_ipv4, network, networkMask, interfaces);
}

void
Ipv4CmdSRouting::SendMessage (uint32_t npkt) 
{
  Ptr<Packet> packet = Create<Packet> (1);
  Ipv4CmdSTag tag;
  tag.SetQueueSize (npkt);
  packet->AddPacketTag (tag);
  m_socket->Send (packet);
}

void
Ipv4CmdSRouting::SyncQueueSize () 
{ 
  uint32_t tot = 0;

  

  for (std::vector<Ptr<Queue> >::iterator it = m_queues.begin (); it != m_queues.end (); ++it)
    {
      tot += (*it)->GetNPackets ();
    }
  
NS_LOG_DEBUG (GetObject<Node> () << " Sync " << tot << " " << m_queues.size ());

  SendMessage (tot);

  m_syncEvent = Simulator::Schedule (m_syncPeriod, &Ipv4CmdSRouting::SyncQueueSize, this);
}

void
Ipv4CmdSRouting::Start ()
{
  NS_LOG_DEBUG ("Object " << GetObject<Node> ());
  m_socket = Socket::CreateSocket (GetObject<Node> (), UdpSocketFactory::GetTypeId ());
  m_socket->SetAllowBroadcast (true);
  m_socket->Bind ();
  m_socket->Connect (Address (InetSocketAddress ("255.255.255.255", 9)));
  m_syncEvent = Simulator::ScheduleNow (&Ipv4CmdSRouting::SyncQueueSize, this);
}

void
Ipv4CmdSRouting::AddQueue (Ptr<Queue> queue)
{
  m_queues.push_back (queue);
}

bool
Ipv4CmdSRouting::RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                           UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                           LocalDeliverCallback lcb, ErrorCallback ecb)
{
  NS_LOG_LOGIC (GetObject<Node> () << " RouteInput: " << "Ip header: " << header);

  NS_ASSERT (m_ipv4->GetInterfaceForDevice (idev) >= 0);

  Ptr<Packet> packet = ConstCast<Packet> (p);

  Ipv4Address srcAddress = header.GetSource ();
  Ipv4Address destAddress = header.GetDestination ();

  FlowIdTag ftag;
  packet->PeekPacketTag (ftag);
  uint32_t flowid = ftag.GetFlowId ();

  // CmdS routing only supports unicast
  if (destAddress.IsMulticast ()) 
    {
      NS_LOG_ERROR (this << " CmdS routing only supports unicast");
      ecb (packet, header, Socket::ERROR_NOROUTETOHOST);
      return false;
    }

  // Check if input device supports IP forwarding
  uint32_t iif = m_ipv4->GetInterfaceForDevice (idev);
  if (m_ipv4->IsForwarding (iif) == false) 
    {
      NS_LOG_ERROR (this << " Forwarding disabled for this interface");
      ecb (packet, header, Socket::ERROR_NOROUTETOHOST);
      return false;
    }

  /*if (destAddress.IsBroadcast ())
    {
      NS_LOG_DEBUG (GetObject<Node> () << " Broadcast " << srcAddress);
      HandleMessage (p, header);
      return true;
    }*/

  if (m_ipv4->IsDestinationAddress (header.GetDestination (), iif))
    {
      Ipv4CmdSTag tag;
      packet->PeekPacketTag (tag);

      uint32_t queueSize = tag.GetQueueSize ();
      
      NS_LOG_DEBUG ("!!! " << srcAddress << " " <<  destAddress << " | " << queueSize);

      if (queueSize != Ipv4CmdSTag::DO_SWITCH && queueSize != Ipv4CmdSTag::NO_SWITCH)
        {
          m_rtable.UpdateQueueSize (srcAddress, queueSize);
          return true;
        }


      
      
      if (!lcb.IsNull ())
        {
          NS_LOG_LOGIC ("Local delivery to " << header.GetDestination ());
          lcb (p, header, iif);
          return true;
        }
      else
        {
          // The local delivery callback is null.  This may be a multicast
          // or broadcast packet, so return false so that another
          // multicast routing protocol can handle it.  It should be possible
          // to extend this to explicitly check whether it is a unicast
          // packet, and invoke the error callback if so
          return false;
        }
    }

  Ipv4CmdSTag tag;
  packet->PeekPacketTag (tag);

  Ptr<Ipv4Route> rtentry = m_rtable.Lookup (m_ipv4, flowid, destAddress, tag.GetQueueSize () == Ipv4CmdSTag::DO_SWITCH);
  if (rtentry != 0)
    {
      NS_LOG_LOGIC ("Found unicast destination- calling unicast callback");
      ucb (rtentry, p, header);
      return true;
    }
  else
    {
      NS_LOG_ERROR ("Did not find unicast destination- returning false");
      return false; // Let other routing protocols try to handle this
                    // route request.
    }
}

void
Ipv4CmdSRouting::NotifyInterfaceUp (uint32_t interface)
{
}

void
Ipv4CmdSRouting::NotifyInterfaceDown (uint32_t interface)
{
}

void
Ipv4CmdSRouting::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
}

void
Ipv4CmdSRouting::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
}

void
Ipv4CmdSRouting::SetIpv4 (Ptr<Ipv4> ipv4)
{
  NS_LOG_INFO (this << "Setting up Ipv4: " << ipv4);
  NS_ASSERT (m_ipv4 == 0 && ipv4 != 0);
  m_ipv4 = ipv4;
  Simulator::ScheduleNow (&Ipv4CmdSRouting::Start, this);
}

void
Ipv4CmdSRouting::PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const
{
}


void
Ipv4CmdSRouting::DoDispose (void)
{
  m_syncEvent.Cancel ();
  m_socket->Close ();
  m_ipv4 = 0;
  Ipv4RoutingProtocol::DoDispose ();
}

}