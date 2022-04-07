
#ifndef AAA_SERVER_H
#define AAA_SERVER_H

#include "ns3/basic-struct.h"
#include "ns3/basic-net-device.h"

namespace ns3
{
    class AaaServer : public BasicNetDevice
    {
    public:
        static TypeId GetTypeId(void);
        AaaServer();
        ~AaaServer();

    private:
        virtual void ReceiveFromPrimaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType);
        virtual void ReceiveFromSecondaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType);

        void ProcessSessionManagementNas(SessionManagementNas *smn);

        AaaServerDeviceInfo *CreateAaaServerDeviceInfo(SessionManagementNas *smn);
        SessionManagementNas *GenerateEAPRequest(AaaServerDeviceInfo *di);
        void SaveNewDeviceInfo(AaaServerDeviceInfo *di, SessionManagementNas *smn);
        uint8_t DeriveAppSKey(AaaServerDeviceInfo *di);
        SessionManagementNas *GenerateEAPSuccess(AaaServerDeviceInfo *di);
        void DeliverAppSKey(uint8_t supi, uint8_t appSKey);

        std::vector<AaaServerDeviceInfo *> *deviceInfoVector;
    };

}; /* namespace ns3 */

#endif /* AAA_SERVER_H */