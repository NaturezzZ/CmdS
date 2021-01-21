#ifndef NS3_IPV4_CMDS_TAG
#define NS3_IPV4_CMDS_TAG

#include "ns3/tag.h"

namespace ns3 {

class Ipv4CmdSTag : public Tag
{
public:
    Ipv4CmdSTag ();

	static const unsigned DO_SWITCH;
	static const unsigned NO_SWITCH;

    static TypeId GetTypeId (void);

    void SetQueueSize (uint32_t queueSize);
    uint32_t GetQueueSize (void) const;

    virtual TypeId GetInstanceTypeId (void) const;

    virtual uint32_t GetSerializedSize (void) const;

    virtual void Serialize (TagBuffer i) const;

    virtual void Deserialize (TagBuffer i);

    virtual void Print (std::ostream &os) const;

private:
	uint32_t m_queueSize;
};

}

#endif