
#include "access-mobility-management-function.h"

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("AccessMobilityManagementFunction");

    NS_OBJECT_ENSURE_REGISTERED(AccessMobilityManagementFunction);

    TypeId AccessMobilityManagementFunction::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::AccessMobilityManagementFunction")
                                .SetParent<BasicNetDevice>()
                                .SetGroupName("LoRaWAN5GIntegration")
                                .AddConstructor<AccessMobilityManagementFunction>();

        return tid;
    }

    Ptr<NetDevice> AccessMobilityManagementFunction::GetTertiaryNetDevice() const
    {
        NS_LOG_FUNCTION(this);
        NS_ASSERT_MSG(m_tertiaryNetDevice != 0, "AccessMobilityManagementFunction: can't find teriary net device " << m_tertiaryNetDevice);

        return m_tertiaryNetDevice;
    }

    void AccessMobilityManagementFunction::SetTertiaryNetDevice(Ptr<NetDevice> device)
    {
        NS_LOG_FUNCTION(this << device << m_protocolNumber);
        m_tertiaryNetDevice = device;
        m_node->RegisterProtocolHandler(MakeCallback(&AccessMobilityManagementFunction::ReceiveFromTertiaryDevice, this), m_protocolNumber, device, false);
    }

    AccessMobilityManagementFunction::AccessMobilityManagementFunction()
    {
        NS_LOG_FUNCTION(this);
        deviceInfoVector = new std::vector<AmfDeviceInfo *>();
    }

    AccessMobilityManagementFunction::~AccessMobilityManagementFunction()
    {
        NS_LOG_FUNCTION(this);
    }

    void AccessMobilityManagementFunction::ReceiveFromPrimaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType)
    {
        NS_LOG_FUNCTION(this);

        SecurityProtectedNas *spn = ReceiveNas<SecurityProtectedNas>(GetPacketDataIterator(packet), packet->GetSize());
        PlainNasMessage *pnm = DecapsulateNas(spn);

        if (pnm->GetExtendedProtocolDiscriminator() == NAS_5GMM)
        {
            ProcessMobilityManagementNas(dynamic_cast<MobilityManagementNas *>(pnm));
        }

        if (pnm->GetExtendedProtocolDiscriminator() == NAS_5GSM)
        {
            ProcessSessionManagementNas(dynamic_cast<SessionManagementNas *>(pnm));
        }
    }

    void AccessMobilityManagementFunction::ReceiveFromSecondaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType)
    {
        NS_LOG_FUNCTION(this);

        MobilityManagementNas *mmn = ReceiveNas<MobilityManagementNas>(GetPacketDataIterator(packet), packet->GetSize());
        ProcessMobilityManagementNas(mmn);
    }

    void AccessMobilityManagementFunction::ReceiveFromTertiaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType)
    {
        NS_LOG_FUNCTION(this);

        SessionManagementNas *smn = ReceiveNas<SessionManagementNas>(GetPacketDataIterator(packet), packet->GetSize());
        ProcessSessionManagementNas(smn);
    }

    void AccessMobilityManagementFunction::ProcessMobilityManagementNas(MobilityManagementNas *mmn)
    {
        NS_LOG_FUNCTION(this);

        switch (mmn->messageType)
        {
        case SERVICE_REQUEST:
        {
            NS_LOG_INFO("[*] AMF adding SNID to 5GMM NAS");
            mmn->ie->push_back(GenerateSnidTlv());

            NS_LOG_INFO("[+] AMF -> AUSF : 5GMM NAS");
            SendNas<MobilityManagementNas>(GetSecondaryNetDevice(), mmn);
            break;
        }
        case EAP_RQST_CN:
        {
            SecurityProtectedNas *spn = EncapsulateNas(mmn);
            NS_LOG_INFO("[+] AMF -> gGW : EAP-LoRaWAN-CN/EAP-RQST");
            SendNas<SecurityProtectedNas>(GetPrimaryNetDevice(), spn);
            break;
        }
        case EAP_RESP_CN:
        {
            NS_LOG_INFO("[+] AMF -> AUSF : EAP-LoRaWAN-CN/EAP-RESP");
            SendNas<MobilityManagementNas>(GetSecondaryNetDevice(), mmn);
            break;
        }
        case EAP_SUCCESS_CN:
        {
            deviceInfoVector->push_back(CreateAmfDeviceInfo(mmn));
            SecurityProtectedNas *spn = EncapsulateNas(mmn);
            NS_LOG_INFO("[+] AMF -> gGW : EAP-LoRaWAN-CN/EAP-SUCCESS");
            SendNas<SecurityProtectedNas>(GetPrimaryNetDevice(), spn);
            break;
        }
        default:
            break;
        }
    }

    Tlv *AccessMobilityManagementFunction::GenerateSnidTlv()
    {
        NS_LOG_FUNCTION(this);

        Tlv *snid = new Tlv();

        snid->type = SNID;
        snid->length = 1;
        snid->value = new uint8_t[snid->length]{SNid};

        return snid;
    }

    AmfDeviceInfo *AccessMobilityManagementFunction::CreateAmfDeviceInfo(MobilityManagementNas *mmn)
    {
        AmfDeviceInfo *di = new AmfDeviceInfo();

        di->supi = mmn->ie->at(0)->value[0];
        di->kAmf = mmn->ie->at(4)->value[0];
        di->nwkSEncKey = mmn->ie->at(5)->value[1];
        di->sNwkSIntKey = mmn->ie->at(5)->value[2];
        di->jSIntKey = mmn->ie->at(5)->value[3];
        di->jSEncKey = mmn->ie->at(5)->value[4];

        return di;
    }

    void AccessMobilityManagementFunction::ProcessSessionManagementNas(SessionManagementNas *smn)
    {
        NS_LOG_FUNCTION(this);

        switch (smn->messageType)
        {
        case SESSION_ESTABLISHMENT:
        {
            AmfDeviceInfo *di = GetDeviceInfoBySupi<AmfDeviceInfo>(deviceInfoVector, smn->ie->at(0));
            AddSessionParamsToDeviceInfo(di, smn);

            NS_LOG_INFO("[+] AMF -> SMF : 5GSM NAS");
            SendNas<SessionManagementNas>(GetTertiaryNetDevice(), smn);
            break;
        }

        case EAP_RQST_DN:
        {
            SecurityProtectedNas *spn = EncapsulateNas(smn);
            NS_LOG_INFO("[+] AMF -> gGW : EAP-LoRaWAN/EAP-RQST");
            SendNas<SecurityProtectedNas>(GetPrimaryNetDevice(), spn);
            break;
        }

        case EAP_RESP_DN:
        {
            NS_LOG_INFO("[+] AMF -> SMF : EAP-LoRaWAN/EAP-RESP");
            SendNas<SessionManagementNas>(GetTertiaryNetDevice(), smn);
            break;
        }

        case EAP_SUCCESS_DN:
        {
            SecurityProtectedNas *spn = EncapsulateNas(smn);
            NS_LOG_INFO("[+] AMF -> gGW : EAP-LoRaWAN/EAP-SUCCESS");
            SendNas<SecurityProtectedNas>(GetPrimaryNetDevice(), spn);
            break;
        }

        default:
            break;
        }
    }

    void AccessMobilityManagementFunction::AddSessionParamsToDeviceInfo(AmfDeviceInfo *di, SessionManagementNas *smn)
    {
        NS_LOG_FUNCTION(this);
        NS_LOG_INFO("[*] AMF saving device session params information");

        di->sp = new SessionParams();

        di->sp->pduSessionId = smn->pduSessionId;
        di->sp->procedureTransactionId = smn->procedureTransactionId;
        di->sp->nssai = smn->ie->at(1)->value[0];
        di->sp->sessionType = smn->ie->at(2)->value[0];
    }
}