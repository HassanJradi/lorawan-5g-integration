
#include "ns3/channel.h"
#include "ns3/uinteger.h"

#include "ns3/basic-net-device.h"

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("BasicNetDevice");

    NS_OBJECT_ENSURE_REGISTERED(BasicNetDevice);

    /* Public Functions */

    TypeId BasicNetDevice::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::BasicNetDevice")
                                .SetParent<NetDevice>()
                                .SetGroupName("LoRaWAN5GIntegration")
                                .AddAttribute("ProtocolNumber",
                                              "The specific Protocol Number to be used in L3 packets.",
                                              UintegerValue(Ipv4L3Protocol::PROT_NUMBER),
                                              MakeUintegerAccessor(&BasicNetDevice::m_protocolNumber),
                                              MakeUintegerChecker<uint16_t>())
                                .AddTraceSource("TimeTrace",
                                                "Time trace",
                                                MakeTraceSourceAccessor(&BasicNetDevice::m_timeTrace),
                                                "ns3::Time::TracedCallback");

        return tid;
    }

    Ptr<NetDevice> BasicNetDevice::GetPrimaryNetDevice() const
    {
        NS_LOG_FUNCTION(this);
        NS_ASSERT_MSG(m_primaryNetDevice != 0, "BasicNetDevice: can't find primary net device " << m_primaryNetDevice);
        return m_primaryNetDevice;
    }

    void BasicNetDevice::SetPrimaryNetDevice(Ptr<NetDevice> device)
    {
        NS_LOG_FUNCTION(this);
        m_primaryNetDevice = device;
        m_node->RegisterProtocolHandler(MakeCallback(&BasicNetDevice::ReceiveFromPrimaryDevice, this), m_protocolNumber, device, false);
    }

    Ptr<NetDevice> BasicNetDevice::GetSecondaryNetDevice() const
    {
        NS_LOG_FUNCTION(this);
        NS_ASSERT_MSG(m_secondaryNetDevice != 0, "BasicNetDevice: can't find secondary net device " << m_secondaryNetDevice);

        return m_secondaryNetDevice;
    }

    void BasicNetDevice::SetSecondaryNetDevice(Ptr<NetDevice> device)
    {
        NS_LOG_FUNCTION(this);

        m_secondaryNetDevice = device;
        m_node->RegisterProtocolHandler(MakeCallback(&BasicNetDevice::ReceiveFromSecondaryDevice, this), m_protocolNumber, device, false);
    }

    uint16_t BasicNetDevice::GetProcessingDelay()
    {
        return this->processingDelay;
    }

    void BasicNetDevice::AccumulateProcessingDelay(uint16_t additionalDelay)
    {
        this->processingDelay += additionalDelay;
    }

    void BasicNetDevice::ResetProcessingDelay()
    {
        this->processingDelay = 0;
    }
    /* Inherited Functions */

    void BasicNetDevice::AddLinkChangeCallback(Callback<void> callback)
    {
        NS_LOG_FUNCTION(this);
    }

    Address BasicNetDevice::GetAddress(void) const
    {
        NS_LOG_FUNCTION(this);
        return Address();
    }

    Address BasicNetDevice::GetBroadcast(void) const
    {
        NS_LOG_FUNCTION(this);
        return m_primaryNetDevice->GetBroadcast();
    }

    Ptr<Channel> BasicNetDevice::GetChannel(void) const
    {
        NS_LOG_FUNCTION(this);
        return m_primaryNetDevice->GetChannel();
    }

    uint32_t BasicNetDevice::GetIfIndex(void) const
    {
        NS_LOG_FUNCTION(this);
        return 0;
    }

    uint16_t BasicNetDevice::GetMtu(void) const
    {
        NS_LOG_FUNCTION(this);
        return 0;
    }

    Address BasicNetDevice::GetMulticast(Ipv4Address multicastGroup) const
    {
        NS_ABORT_MSG("Unsupported");
        return Address();
    }

    Address BasicNetDevice::GetMulticast(Ipv6Address addr) const
    {
        NS_LOG_FUNCTION(this);
        return Address();
    }

    Ptr<Node> BasicNetDevice::GetNode(void) const
    {
        NS_LOG_FUNCTION(this);
        return m_node;
    }

    bool BasicNetDevice::IsBridge(void) const
    {
        NS_LOG_FUNCTION(this);
        return false;
    }

    bool BasicNetDevice::IsBroadcast(void) const
    {
        NS_LOG_FUNCTION(this);
        return true;
    }

    bool BasicNetDevice::IsLinkUp(void) const
    {
        NS_LOG_FUNCTION(this);
        return m_primaryNetDevice != 0;
    }

    bool BasicNetDevice::IsMulticast(void) const
    {
        NS_LOG_FUNCTION(this);
        return true;
    }

    bool BasicNetDevice::IsPointToPoint(void) const
    {
        NS_LOG_FUNCTION(this);
        return false;
    }

    bool BasicNetDevice::NeedsArp(void) const
    {
        NS_LOG_FUNCTION(this);
        return true;
    }

    bool BasicNetDevice::Send(Ptr<Packet> packet, const Address &dest, uint16_t protocolNumber)
    {
        NS_LOG_FUNCTION(this);
        return true;
    }

    bool BasicNetDevice::SendFrom(Ptr<Packet> packet, const Address &src, const Address &dest, uint16_t protocolNumber)
    {
        NS_ABORT_MSG("Unsupported");
        return false;
    }

    void BasicNetDevice::SetAddress(Address address)
    {
        NS_LOG_FUNCTION(this);
    }

    void BasicNetDevice::SetIfIndex(const uint32_t index)
    {
        NS_LOG_FUNCTION(this << index);
    }

    bool BasicNetDevice::SetMtu(const uint16_t mtu)
    {
        NS_ABORT_MSG("Unsupported");
        return false;
    }

    void BasicNetDevice::SetNode(Ptr<Node> node)
    {
        NS_LOG_FUNCTION(this << node);
        m_node = node;
    }

    void BasicNetDevice::SetPromiscReceiveCallback(NetDevice::PromiscReceiveCallback cb)
    {
        NS_LOG_FUNCTION_NOARGS();
    }

    void BasicNetDevice::SetReceiveCallback(NetDevice::ReceiveCallback cb)
    {
        NS_LOG_FUNCTION_NOARGS();
        m_rxCallback = cb;
    }

    bool BasicNetDevice::SupportsSendFrom() const
    {
        NS_LOG_FUNCTION_NOARGS();
        return false;
    }

    /* Private Functions */

    Buffer::Iterator BasicNetDevice::GetPacketDataIterator(Ptr<const Packet> packet)
    {
        uint8_t length = packet->GetSize();
        uint8_t *data = new uint8_t[length];
        packet->CopyData(data, length);

        Buffer buf = Buffer(length);
        buf.AddAtStart(length);

        Buffer::Iterator it = buf.Begin();
        it.Write(data, length);
        it = buf.Begin();

        return it;
    }

    SecurityProtectedNas *BasicNetDevice::EncapsulateNas(PlainNasMessage *pnm, uint8_t kAmf)
    {
        NS_LOG_FUNCTION(this);
        NS_LOG_INFO("[*] Encpsulating NAS into SP NAS");

        AccumulateProcessingDelay(PACKET_ENCAPSULATION_DELAY);

        // This message should be encrypted using kAMF
        SecurityProtectedNas *spn = new SecurityProtectedNas();

        spn->securityHeaderType = INTEGRITY_PROTECTED_AND_CIPHERED;
        spn->messageAuthenticationCode = 100;
        spn->sequenceNumber = 5;
        spn->message = pnm;

        return spn;
    }

    PlainNasMessage *BasicNetDevice::DecapsulateNas(SecurityProtectedNas *spn, uint8_t kAmf)
    {
        NS_LOG_FUNCTION(this);

        AccumulateProcessingDelay(PACKET_DECAPSULATION_DELAY);

        // We should decrypt NAS here
        // For simplicity, we just get the PlainNasMessage

        if (spn->message->GetExtendedProtocolDiscriminator() == NAS_5GMM)
        {
            NS_LOG_INFO("[*] Decapsulating SP NAS into 5GMM NAS");
            MobilityManagementNas *mmnAsPnm = dynamic_cast<MobilityManagementNas *>(spn->message);
            return mmnAsPnm;
        }

        if (spn->message->GetExtendedProtocolDiscriminator() == NAS_5GSM)
        {
            NS_LOG_INFO("[*] Decapsulating SP NAS into 5GSM NAS");
            SessionManagementNas *smnAsPmn = dynamic_cast<SessionManagementNas *>(spn->message);
            return smnAsPmn;
        }

        return NULL;
    }
}