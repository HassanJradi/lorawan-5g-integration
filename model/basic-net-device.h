/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef BASIC_NET_DEVICE_H
#define BASIC_NET_DEVICE_H

#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/ipv4-l3-protocol.h"

#include "ns3/basic-struct.h"

namespace ns3
{
    class BasicNetDevice : public NetDevice
    {
    public:
        static TypeId GetTypeId(void);

        Ptr<NetDevice> GetPrimaryNetDevice() const;
        void SetPrimaryNetDevice(Ptr<NetDevice> device);
        Ptr<NetDevice> GetSecondaryNetDevice() const;
        void SetSecondaryNetDevice(Ptr<NetDevice> device);

        uint16_t GetProcessingDelay();
        void AccumulateProcessingDelay(uint16_t additionalDelay);
        void ResetProcessingDelay();

        virtual void AddLinkChangeCallback(Callback<void> callback);
        virtual Address GetAddress(void) const;
        virtual Address GetBroadcast(void) const;
        virtual Ptr<Channel> GetChannel(void) const;
        virtual uint32_t GetIfIndex(void) const;
        virtual uint16_t GetMtu(void) const;
        virtual Address GetMulticast(Ipv4Address multicastGroup) const;
        virtual Address GetMulticast(Ipv6Address addr) const;
        virtual Ptr<Node> GetNode(void) const;
        virtual bool IsBridge(void) const;
        virtual bool IsBroadcast(void) const;
        virtual bool IsLinkUp(void) const;
        virtual bool IsMulticast(void) const;
        virtual bool IsPointToPoint(void) const;
        virtual bool NeedsArp(void) const;
        virtual bool Send(Ptr<Packet> packet, const Address &dest, uint16_t protocolNumber);
        virtual bool SendFrom(Ptr<Packet> packet, const Address &source, const Address &dest, uint16_t protocolNumber);
        virtual void SetAddress(Address address);
        virtual void SetIfIndex(const uint32_t index);
        virtual bool SetMtu(const uint16_t mtu);
        virtual void SetNode(Ptr<Node> node);
        virtual void SetPromiscReceiveCallback(PromiscReceiveCallback cb);
        virtual void SetReceiveCallback(ReceiveCallback cb);
        virtual bool SupportsSendFrom(void) const;

    protected:
        virtual void ReceiveFromPrimaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType) = 0;
        virtual void ReceiveFromSecondaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType) = 0;

        Buffer::Iterator GetPacketDataIterator(Ptr<const Packet> packet);

        SecurityProtectedNas *EncapsulateNas(PlainNasMessage *pnm, uint8_t kAmf = 0);
        PlainNasMessage *DecapsulateNas(SecurityProtectedNas *spn, uint8_t kAmf = 0);

        template <typename NasType>
        void SendNas(Ptr<NetDevice> device, NasType *nas)
        {
            uint8_t length = nas->GetSerializedSize();

            Buffer buf = Buffer(length);
            buf.AddAtStart(length);
            Buffer::Iterator it = buf.Begin();
            nas->Serialize(it);
            it = buf.Begin();

            uint8_t *buffer = new uint8_t[length];
            it.Read(buffer, length);

            Ptr<Packet> packet = Create<Packet>(buffer, length);
            AccumulateProcessingDelay(PACKET_GENERATION_DELAY);

            Simulator::Schedule(MicroSeconds(processingDelay), MakeEvent(&NetDevice::Send, device, packet, GetBroadcast(), Ipv4L3Protocol::PROT_NUMBER));
            ResetProcessingDelay();
        }

        template <typename NasType>
        NasType *ReceiveNas(Buffer::Iterator it, uint8_t length)
        {
            NasType *nas = new NasType();
            nas->Deserialize(it, length);
            return nas;
        }

        template <typename DeviceInfoType>
        DeviceInfoType *GetDeviceInfoBySupi(std::vector<DeviceInfoType *> *deviceInfoVector, Tlv *supi)
        {
            AccumulateProcessingDelay(SEARCHING_DELAY);

            for (auto vecit = std::begin(*deviceInfoVector); vecit != std::end(*deviceInfoVector); ++vecit)
            {
                if ((*vecit)->supi == supi->value[0])
                    return *vecit;
            }

            return nullptr;
        }

        Ptr<Node> m_node;
        uint16_t m_protocolNumber;
        uint16_t processingDelay;
        NetDevice::ReceiveCallback m_rxCallback;
        Ptr<NetDevice> m_primaryNetDevice;
        Ptr<NetDevice> m_secondaryNetDevice;
        TracedCallback<> m_timeTrace;
    };
}

#endif /* BASIC_NET_DEVICE_H */
