
#ifndef AUTHENTICATION_SERVER_FUNCTION_H
#define AUTHENTICATION_SERVER_FUNCTION_H

#include "ns3/basic-struct.h"
#include "ns3/basic-net-device.h"

namespace ns3
{
    class AuthenticationServerFunction : public BasicNetDevice
    {
    public:
        static TypeId GetTypeId(void);
        AuthenticationServerFunction();
        ~AuthenticationServerFunction();

    private:
        virtual void ReceiveFromPrimaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType);
        virtual void ReceiveFromSecondaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType);

        void ProcessMobilityManagementNas(MobilityManagementNas *mmn);
        AusfDeviceInfo *CreateAusfDeviceInfo(MobilityManagementNas *mmn);
        MobilityManagementNas *GenerateEAPRequest(AusfDeviceInfo *di);
        JoinRequestParams *GetJoinRequestParams(MobilityManagementNas *mmn);
        bool IsMicValid(AusfDeviceInfo *di, JoinRequestParams *jrqstp);
        uint8_t *GenerateLoRaWANSessionKeys(AusfDeviceInfo *di, JoinRequestParams *jrqstp, uint8_t joinNonce);
        JoinResponseParams *GenerateJoinRespParams(AusfDeviceInfo *di, uint8_t joinNonce, uint8_t jSIntKey);
        MobilityManagementNas *GenerateEAPSuccess(AusfDeviceInfo *di, uint8_t *sessionkeys, JoinResponseParams *jrespp);

        std::vector<AusfDeviceInfo *> *deviceInfoVector;
    };

}; /* namespace ns3 */

#endif /* AUTHENTICATION_SERVER_FUNCTION_H */
