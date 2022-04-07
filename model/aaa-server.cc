
#include "ns3/aaa-server.h"

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("AaaServer");

    NS_OBJECT_ENSURE_REGISTERED(AaaServer);

    TypeId AaaServer::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::AaaServer")
                                .SetParent<BasicNetDevice>()
                                .SetGroupName("LoRaWAN5GIntegration")
                                .AddConstructor<AaaServer>();

        return tid;
    }

    AaaServer::AaaServer()
    {
        NS_LOG_FUNCTION(this);
        deviceInfoVector = new std::vector<AaaServerDeviceInfo *>();

        AaaServerDeviceInfo *di = new AaaServerDeviceInfo();
        di->supi = 151;
        di->appKey = 48;

        deviceInfoVector->push_back(di);
    }

    AaaServer::~AaaServer()
    {
        NS_LOG_FUNCTION(this);
    }

    void AaaServer::ReceiveFromPrimaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType)
    {
        NS_LOG_FUNCTION(this);

        SessionManagementNas *smn = ReceiveNas<SessionManagementNas>(GetPacketDataIterator(packet), packet->GetSize());
        ProcessSessionManagementNas(smn);
    }

    void AaaServer::ReceiveFromSecondaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType)
    {
        NS_LOG_FUNCTION(this);
    }

    void AaaServer::ProcessSessionManagementNas(SessionManagementNas *smn)
    {
        NS_LOG_FUNCTION(this);

        switch (smn->messageType)
        {
        case SESSION_ESTABLISHMENT:
        {
            AaaServerDeviceInfo *di = CreateAaaServerDeviceInfo(smn);
            deviceInfoVector->push_back(di);
            SessionManagementNas *r_smn = GenerateEAPRequest(di);
            NS_LOG_INFO("[+] AAA -> SMF : EAP-LoRaWAN-DN/EAP-RQST");
            SendNas<SessionManagementNas>(GetPrimaryNetDevice(), r_smn);
            break;
        }

        case EAP_RESP_DN:
        {
            AaaServerDeviceInfo *di = GetDeviceInfoBySupi<AaaServerDeviceInfo>(deviceInfoVector, smn->ie->at(0));
            SaveNewDeviceInfo(di, smn);
            uint8_t appSKey = DeriveAppSKey(di);
            SessionManagementNas *r_smn = GenerateEAPSuccess(di);
            NS_LOG_INFO("[+] AAA -> SMF : EAP-LoRaWAN-DN/EAP-SUCCESS [+] AAA -> Application Server: AppSKey");
            SendNas<SessionManagementNas>(GetPrimaryNetDevice(), r_smn);
            DeliverAppSKey(di->supi, appSKey);

            break;
        }

        default:
            break;
        }
    }

    AaaServerDeviceInfo *AaaServer::CreateAaaServerDeviceInfo(SessionManagementNas *smn)
    {
        AaaServerDeviceInfo *di = new AaaServerDeviceInfo();

        di->supi = smn->ie->at(0)->value[0];
        di->sessionId = smn->pduSessionId;

        return di;
    }

    SessionManagementNas *AaaServer::GenerateEAPRequest(AaaServerDeviceInfo *di)
    {
        SessionManagementNas *smn = new SessionManagementNas();

        smn->pduSessionId = di->sessionId;
        smn->messageType = EAP_RQST_DN;

        Tlv *supi = new Tlv();
        supi->type = SUPI;
        supi->length = 1;
        supi->value = new uint8_t[supi->length]{di->supi};

        smn->ie->push_back(supi);

        return smn;
    }

    void AaaServer::SaveNewDeviceInfo(AaaServerDeviceInfo *di, SessionManagementNas *smn)
    {
        NS_LOG_FUNCTION(this);

        di->joinEui = smn->ie->at(1)->value[0];
        di->devNonce = smn->ie->at(2)->value[0];
        di->joinNonce = smn->ie->at(3)->value[0];
    }

    uint8_t AaaServer::DeriveAppSKey(AaaServerDeviceInfo *di)
    {
        NS_LOG_FUNCTION(this);
        uint8_t appSKey = di->appKey ^ di->joinEui ^ di->devNonce ^ di->joinNonce ^ 0x02;

        AccumulateProcessingDelay(4 * XOR_TIME);

        NS_LOG_INFO("[+] AAA deriving AppSKey = " << (int)appSKey);
        return appSKey;
    }

    SessionManagementNas *AaaServer::GenerateEAPSuccess(AaaServerDeviceInfo *di)
    {
        SessionManagementNas *smn = new SessionManagementNas();

        smn->pduSessionId = di->sessionId;
        smn->messageType = EAP_SUCCESS_DN;

        Tlv *supi = new Tlv();
        supi->type = SUPI;
        supi->length = 1;
        supi->value = new uint8_t[supi->length]{di->supi};

        smn->ie->push_back(supi);

        return smn;
    }

    void AaaServer::DeliverAppSKey(uint8_t supi, uint8_t appSKey)
    {
        uint8_t buf[2] = {supi, appSKey};
        GetSecondaryNetDevice()->Send(new Packet(buf, 2), GetBroadcast(), Ipv4L3Protocol::PROT_NUMBER);
    }

}
