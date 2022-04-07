
#ifndef UNIFIED_DATA_MANAGEMENT_H
#define UNIFIED_DATA_MANAGEMENT_H

#include "ns3/basic-struct.h"
#include "ns3/basic-net-device.h"

namespace ns3
{
    class UnifiedDataManagement : public BasicNetDevice
    {
    public:
        static TypeId GetTypeId(void);

        UnifiedDataManagement();
        ~UnifiedDataManagement();

    private:
        virtual void ReceiveFromPrimaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType);
        virtual void ReceiveFromSecondaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType);

        void ProcessMobilityManagementNas(MobilityManagementNas *mmn);
        Tlv *DeriveKAusf(uint8_t sharedKey);
        Tlv *GetSupi(Tlv *suci);
        MobilityManagementNas *GenerateAuthenticationVectorMmn(Tlv *suci, Tlv *supi, Tlv *snid, Tlv *kAusf);

        std::vector<UdmDeviceInfo *> *deviceInfoVector;
    };

}; /* namespace ns3 */

#endif /* UNIFIED_DATA_MANAGEMENT_H */