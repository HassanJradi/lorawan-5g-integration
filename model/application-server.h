
#ifndef APPLICATION_SERVER_H
#define APPLICATION_SERVER_H

#include "ns3/basic-struct.h"
#include "ns3/basic-net-device.h"

namespace ns3
{
    class ApplicationServer : public BasicNetDevice
    {
    public:
        static TypeId GetTypeId(void);
        ApplicationServer();
        ~ApplicationServer();

    private:
        virtual void ReceiveFromPrimaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType);
        virtual void ReceiveFromSecondaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType);

        std::vector<AaaServerDeviceInfo *> *deviceInfoVector;
    };

}; /* namespace ns3 */

#endif /* APPLICATION_SERVER_H */