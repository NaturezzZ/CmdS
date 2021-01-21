#include "ipv4-cmds-tag.h"

namespace ns3
{

const unsigned Ipv4CmdSTag::DO_SWITCH = 0xffffffffu;
const unsigned Ipv4CmdSTag::NO_SWITCH = 0xfffffffeu;

Ipv4CmdSTag::Ipv4CmdSTag () {}

TypeId
Ipv4CmdSTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Ipv4CmdSTag")
    .SetParent<Tag> ()
    .SetGroupName ("Internet")
    .AddConstructor<Ipv4CmdSTag> ();
  return tid;
}

void
Ipv4CmdSTag::SetQueueSize (uint32_t queueSize)
{
  m_queueSize = queueSize;
}

uint32_t
Ipv4CmdSTag::GetQueueSize (void) const
{
  return m_queueSize;
}

TypeId
Ipv4CmdSTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
Ipv4CmdSTag::GetSerializedSize (void) const
{
  return sizeof (uint32_t);
}

void
Ipv4CmdSTag::Serialize (TagBuffer i) const
{
  i.WriteU32(m_queueSize);
}

void
Ipv4CmdSTag::Deserialize (TagBuffer i)
{
  m_queueSize = i.ReadU32 ();
}

void
Ipv4CmdSTag::Print (std::ostream &os) const
{
  os << "Queue Size = " << m_queueSize;
}

}
