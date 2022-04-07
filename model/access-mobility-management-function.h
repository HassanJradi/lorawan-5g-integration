
#ifndef ACCESS_MOBILITY_MANAGEMENT_FUNCTION_H
#define ACCESS_MOBILITY_MANAGEMENT_FUNCTION_H

#include "ns3/basic-struct.h"
#include "ns3/basic-net-device.h"

namespace ns3
{
    class AccessMobilityManagementFunction : public BasicNetDevice
    {
    public:
        static TypeId GetTypeId(void);
        AccessMobilityManagementFunction();
        ~AccessMobilityManagementFunction();

        Ptr<NetDevice> GetTertiaryNetDevice() const;
        void SetTertiaryNetDevice(Ptr<NetDevice> device);

    private:
        virtual void ReceiveFromPrimaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType);
        virtual void ReceiveFromSecondaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType);
        void ReceiveFromTertiaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType);

        void ProcessMobilityManagementNas(MobilityManagementNas *mmn);
        Tlv *GenerateSnidTlv();
        AmfDeviceInfo *CreateAmfDeviceInfo(MobilityManagementNas *mmn);

        void ProcessSessionManagementNas(SessionManagementNas *smn);
        void AddSessionParamsToDeviceInfo(AmfDeviceInfo *di, SessionManagementNas *smn);

        Ptr<NetDevice> m_tertiaryNetDevice;
        std::vector<AmfDeviceInfo *> *deviceInfoVector;
        uint8_t SNid = 251;
    };

}; /* namespace ns3 */

#endif /* ACCESS_MOBILITY_MANAGEMENT_FUNCTION_H */
