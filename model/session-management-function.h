
#ifndef SESSION_MANAGEMENT_FUNCTION_H
#define SESSION_MANAGEMENT_FUNCTION_H

#include "ns3/basic-struct.h"
#include "ns3/basic-net-device.h"

namespace ns3
{
    class SessionManagementFunction : public BasicNetDevice
    {
    public:
        static TypeId GetTypeId(void);
        SessionManagementFunction();
        ~SessionManagementFunction();

    private:
        virtual void ReceiveFromPrimaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType);
        virtual void ReceiveFromSecondaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType);

        void ProcessSessionManagementNas(SessionManagementNas *smn);
        SmfDeviceInfo *CreateSmfDeviceInfo(SessionManagementNas *smn);
        SessionManagementNas *CreateAuthenticationNotification(SmfDeviceInfo *di);

        std::vector<SmfDeviceInfo *> *deviceInfoVector;
    };

}; /* namespace ns3 */

#endif /* SESSION_MANAGEMENT_FUNCTION_H */