
#include "ns3/application-server.h"

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("ApplicationServer");

    NS_OBJECT_ENSURE_REGISTERED(ApplicationServer);

    TypeId ApplicationServer::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::ApplicationServer")
                                .SetParent<BasicNetDevice>()
                                .SetGroupName("LoRaWAN5GIntegration")
                                .AddConstructor<ApplicationServer>();

        return tid;
    }

    ApplicationServer::ApplicationServer()
    {
        NS_LOG_FUNCTION(this);
    }

    ApplicationServer::~ApplicationServer()
    {
        NS_LOG_FUNCTION(this);
    }

    void ApplicationServer::ReceiveFromPrimaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType)
    {
        NS_LOG_FUNCTION(this);

                Buffer::Iterator it = GetPacketDataIterator(packet);

        uint8_t supi = it.ReadU8();
        uint8_t appSKey = it.ReadU8();

        NS_LOG_INFO("[+] Application server receiving AppSKey = " << (int)appSKey << " for device SUPI = " << (int)supi);
    }

    void ApplicationServer::ReceiveFromSecondaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType)
    {
        NS_LOG_FUNCTION(this);
    }
};