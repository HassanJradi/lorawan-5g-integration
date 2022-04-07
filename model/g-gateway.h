
#ifndef G_GATEWAY_H
#define G_GATEWAY_H

#include "ns3/basic-struct.h"
#include "ns3/basic-net-device.h"

namespace ns3
{
    class gGateway : public BasicNetDevice
    {
    public:
        static TypeId GetTypeId(void);
        gGateway();
        ~gGateway();

    private:
        virtual void ReceiveFromPrimaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType);
        virtual void ReceiveFromSecondaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType);

        JoinRequestParams *GetJoinRequestParams(Buffer::Iterator it);
        gGwDeviceInfo *CreategGwDeviceInfo(JoinRequestParams *jrqstp);
        uint8_t DeriveSuci(uint8_t devEui);
        MobilityManagementNas *TranslateAuthenticationRequest(gGwDeviceInfo *di);
        void ProcessMobilityManagementNas(MobilityManagementNas *mmn);
        MobilityManagementNas *GenerateEAPResponse(gGwDeviceInfo *di);
        JoinResponseParams *GetJoinResponseParams(MobilityManagementNas *mmn);
        bool IsMicValid(gGwDeviceInfo *di);
        void SetDeviceSessionParams(gGwDeviceInfo *di);
        SessionManagementNas *GeneratePduSessionEstablishmentRequest(gGwDeviceInfo *di);
        void ProcessSessionManagementNas(SessionManagementNas *smn);
        SessionManagementNas *CreateEAPResponse(gGwDeviceInfo *di);
        Ptr<Packet> CreateJoinAccpetPacket(gGwDeviceInfo *di);

        std::vector<gGwDeviceInfo *> *deviceInfoVector;
    };

}; /* namespace ns3 */

#endif /* G_GATEWAY_H */