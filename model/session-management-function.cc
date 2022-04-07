
#include "ns3/session-management-function.h"

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("SessionManagementFunction");

    NS_OBJECT_ENSURE_REGISTERED(SessionManagementFunction);

    TypeId SessionManagementFunction::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::SessionManagementFunction")
                                .SetParent<BasicNetDevice>()
                                .SetGroupName("LoRaWAN5GIntegration")
                                .AddConstructor<SessionManagementFunction>();

        return tid;
    }

    SessionManagementFunction::SessionManagementFunction()
    {
        NS_LOG_FUNCTION(this);
        deviceInfoVector = new std::vector<SmfDeviceInfo *>();
    }

    SessionManagementFunction::~SessionManagementFunction()
    {
    }

    void SessionManagementFunction::ReceiveFromPrimaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType)
    {
        NS_LOG_FUNCTION(this);

        SessionManagementNas *smn = ReceiveNas<SessionManagementNas>(GetPacketDataIterator(packet), packet->GetSize());
        ProcessSessionManagementNas(smn);
    }

    void SessionManagementFunction::ReceiveFromSecondaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType)
    {
        NS_LOG_FUNCTION(this);

        SessionManagementNas *smn = ReceiveNas<SessionManagementNas>(GetPacketDataIterator(packet), packet->GetSize());
        ProcessSessionManagementNas(smn);
    }

    void SessionManagementFunction::ProcessSessionManagementNas(SessionManagementNas *smn)
    {
        NS_LOG_FUNCTION(this);

        switch (smn->messageType)
        {
        case SESSION_ESTABLISHMENT:
        {
            SmfDeviceInfo *di = CreateSmfDeviceInfo(smn);
            deviceInfoVector->push_back(di);
            SessionManagementNas *r_smn = CreateAuthenticationNotification(di);
            NS_LOG_INFO("[+] SMF -> AAA : Authentication notification for device : "
                        << "SUPI = " << (int)di->supi);
            SendNas<SessionManagementNas>(GetSecondaryNetDevice(), r_smn);
            break;
        }

        case EAP_RQST_DN:
        {
            NS_LOG_INFO("[+] SMF -> AMF : EAP-LoRaWAN-DN/EAP-RQST");
            SendNas<SessionManagementNas>(GetPrimaryNetDevice(), smn);
            break;
        }

        case EAP_RESP_DN:
        {
            SendNas<SessionManagementNas>(GetSecondaryNetDevice(), smn);
            break;
        }

        case EAP_SUCCESS_DN:
        {
            NS_LOG_INFO("[+] SMF -> AMF : EAP-LoRaWAN-DN/EAP-SUCCESS");
            SendNas<SessionManagementNas>(GetPrimaryNetDevice(), smn);
            break;
        }

        default:
            break;
        }
    }

    SmfDeviceInfo *SessionManagementFunction::CreateSmfDeviceInfo(SessionManagementNas *smn)
    {
        SmfDeviceInfo *di = new SmfDeviceInfo();
        di->supi = smn->ie->at(0)->value[0];

        di->sp = new SessionParams();
        di->sp->pduSessionId = smn->pduSessionId;
        di->sp->procedureTransactionId = smn->procedureTransactionId;
        di->sp->nssai = smn->ie->at(1)->value[0];
        di->sp->sessionType = smn->ie->at(2)->value[0];

        return di;
    }

    SessionManagementNas *SessionManagementFunction::CreateAuthenticationNotification(SmfDeviceInfo *di)
    {
        SessionManagementNas *smn = new SessionManagementNas();

        smn->pduSessionId = di->sp->pduSessionId;
        smn->procedureTransactionId = di->sp->procedureTransactionId;
        smn->messageType = SESSION_ESTABLISHMENT;

        Tlv *supi = new Tlv();
        supi->type = SUPI;
        supi->length = 1;
        supi->value = new uint8_t[supi->length]{di->supi};

        smn->ie->push_back(supi);

        return smn;
    }
}
