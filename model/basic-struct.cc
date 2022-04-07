
#include "ns3/basic-struct.h"

namespace ns3
{
    /* MobilityManagementNas */

    ExtendedProtocolDiscriminator MobilityManagementNas::GetExtendedProtocolDiscriminator()
    {
        return epd;
    }

    uint8_t MobilityManagementNas::GetSerializedSize(void) const
    {
        uint8_t serializedSize = sizeof(epd) + sizeof(securityHeaderType) + sizeof(messageType) + sizeof(ie->size());
        for (auto vecit = std::begin(*ie); vecit != std::end(*ie); ++vecit)
        {
            serializedSize += (*vecit)->GetSerializedSize();
        }

        return serializedSize;
    }

    void MobilityManagementNas::Serialize(Buffer::Iterator &it) const
    {
        it.WriteU8(epd);
        it.WriteU8(securityHeaderType);
        it.WriteU8(messageType);

        it.WriteU8(ie->size());

        for (auto vecit = std::begin(*ie); vecit != std::end(*ie); ++vecit)
        {
            (*vecit)->Serialize(it);
        }
    }

    void MobilityManagementNas::Deserialize(Buffer::Iterator &it, uint8_t length)
    {
        it.ReadU8();
        securityHeaderType = (SecurityHeaderType)it.ReadU8();
        messageType = (MobilityManagementMessageType)it.ReadU8();

        uint8_t ieSize = it.ReadU8();

        for (int i = 0; i < ieSize; i++)
        {
            Tlv *informationElement = new Tlv();
            informationElement->Deserialize(it);
            ie->push_back(informationElement);
        }
    }

    void MobilityManagementNas::Print() const
    {
        NS_LOG_UNCOND("      [MobilityManagementNas]\n"
                      << "      Serialized Size = " << (int)GetSerializedSize() << " "
                      << "ExtendedProtocolDiscriminator = " << epd << " "
                      << "SecurityHeaderType = " << securityHeaderType << " "
                      << "MessageType = " << messageType);

        for (auto vecit = std::begin(*ie); vecit != std::end(*ie); ++vecit)
        {
            (*vecit)->Print();
        }
    }

    /* SessionManagementNas */

    ExtendedProtocolDiscriminator SessionManagementNas::GetExtendedProtocolDiscriminator()
    {
        return epd;
    }

    uint8_t SessionManagementNas::GetSerializedSize(void) const
    {
        uint8_t serializedSize = sizeof(epd) + sizeof(pduSessionId) + sizeof(procedureTransactionId) + sizeof(messageType) + sizeof(ie->size());

        for (auto vecit = std::begin(*ie); vecit != std::end(*ie); ++vecit)
        {
            serializedSize += (*vecit)->GetSerializedSize();
        }

        return serializedSize;
    }

    void SessionManagementNas::Serialize(Buffer::Iterator &it) const
    {
        it.WriteU8(epd);
        it.WriteU8(pduSessionId);
        it.WriteU8(procedureTransactionId);
        it.WriteU8(messageType);

        it.WriteU8(ie->size());

        for (auto vecit = std::begin(*ie); vecit != std::end(*ie); ++vecit)
        {
            (*vecit)->Serialize(it);
        }
    }

    void SessionManagementNas::Deserialize(Buffer::Iterator &it, uint8_t length)
    {
        it.ReadU8();
        pduSessionId = it.ReadU8();
        procedureTransactionId = it.ReadU8();
        messageType = (SessionManagementMessageType)it.ReadU8();

        uint8_t ieSize = it.ReadU8();

        for (int i = 0; i < ieSize; i++)
        {
            Tlv *informationElement = new Tlv();
            informationElement->Deserialize(it);
            ie->push_back(informationElement);
        }
    }

    void SessionManagementNas::Print() const
    {
        NS_LOG_UNCOND("      [SessionManagementNas]\n"
                      << "      Serialized Size = " << (int)GetSerializedSize() << " "
                      << "ExtendedProtocolDiscriminator = " << epd << " "
                      << "PduSessionId = " << (int)pduSessionId << " "
                      << "ProcedureTransactionId = " << (int)procedureTransactionId << " "
                      << "MessageType = " << messageType);

        for (auto vecit = std::begin(*ie); vecit != std::end(*ie); ++vecit)
        {
            (*vecit)->Print();
        }
    }

    /* SecurityProtectedNas */

    uint8_t SecurityProtectedNas::GetSerializedSize(void) const
    {
        return sizeof(epd) + sizeof(securityHeaderType) + sizeof(messageAuthenticationCode) + sizeof(sequenceNumber) + message->GetSerializedSize();
    }

    void SecurityProtectedNas::Serialize(Buffer::Iterator &it) const
    {
        it.WriteU8(securityHeaderType);
        it.WriteU8(messageAuthenticationCode);
        it.WriteU8(sequenceNumber);
        message->Serialize(it);
    }

    void SecurityProtectedNas::Deserialize(Buffer::Iterator &it, uint8_t length)
    {
        securityHeaderType = (SecurityHeaderType)it.ReadU8();
        messageAuthenticationCode = it.ReadU8();
        sequenceNumber = it.ReadU8();

        ExtendedProtocolDiscriminator p_epd = (ExtendedProtocolDiscriminator)it.ReadU8();
        it.Prev();

        if (p_epd == NAS_5GMM)
        {
            message = new MobilityManagementNas();
        }
        if (p_epd == NAS_5GSM)
        {
            message = new SessionManagementNas();
        }

        message->Deserialize(it, length - 6);
    }

    void SecurityProtectedNas::Print() const
    {
        NS_LOG_UNCOND("   [SecurityProtectedNas]\n"
                      << "   Serialized Size = " << GetSerializedSize() << " "
                      << "ExtendedProtocolDiscriminator = " << epd << " "
                      << "SecurityHeaderType = " << securityHeaderType << " "
                      << "MessageAuthenticationCode = " << messageAuthenticationCode << " "
                      << "SequenceNumber = " << (int)sequenceNumber);
        message->Print();
    }

    /* Tlv */

    uint8_t Tlv::GetSerializedSize(void) const
    {
        return sizeof(type) + length + sizeof(length);
    }

    void Tlv::Serialize(Buffer::Iterator &it) const
    {
        it.WriteU8(type);
        it.WriteU8(length);
        it.Write(value, length);
    }

    void Tlv::Deserialize(Buffer::Iterator &it)
    {
        type = (TlvType)it.ReadU8();
        length = it.ReadU8();
        value = new uint8_t[length];
        it.Read(value, length);
    }

    void Tlv::Print() const
    {
        NS_LOG_UNCOND("         [Tlv] "
                      << "Serialized Size = " << (int)GetSerializedSize() << " "
                      << "Type = " << type << " "
                      << "Length = " << (int)length);

        for (int i = 0; i < length; i++)
            NS_LOG_UNCOND("            Value[" << i << "] : " << (int)value[i]);
    }

}
