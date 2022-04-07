
#include "ns3/lorawan-mac-header.h"
#include "ns3/lora-frame-header.h"

#include "ns3/g-gateway.h"

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("gGateway");

    NS_OBJECT_ENSURE_REGISTERED(gGateway);

    TypeId gGateway::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::gGateway")
                                .SetParent<BasicNetDevice>()
                                .SetGroupName("LoRaWAN5GIntegration")
                                .AddConstructor<gGateway>();

        return tid;
    }

    gGateway::gGateway()
    {
        NS_LOG_FUNCTION(this);
        deviceInfoVector = new std::vector<gGwDeviceInfo *>();
    }

    gGateway::~gGateway()
    {
        NS_LOG_FUNCTION(this);
    }

    void gGateway::ReceiveFromPrimaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType)
    {
        NS_LOG_FUNCTION(this);

        // LoRaWAN channel
        Ptr<Packet> packetCopy = packet->Copy();
        lorawan::LorawanMacHeader mHdr;
        packetCopy->RemoveHeader(mHdr);
        lorawan::LoraFrameHeader fHdr;
        packetCopy->RemoveHeader(fHdr);
        Buffer::Iterator it = GetPacketDataIterator(packetCopy);

        // // csma channel
        // Buffer::Iterator it = GetPacketDataIterator(packet);
        
        if (it.ReadU8() == JOIN_REQUEST)
        {
            m_timeTrace();
            JoinRequestParams *jrqstp = GetJoinRequestParams(it);
            gGwDeviceInfo *di = CreategGwDeviceInfo(jrqstp);
            di->mHdr = mHdr;
            di->fHdr = fHdr;
            deviceInfoVector->push_back(di);
            MobilityManagementNas *mmn = TranslateAuthenticationRequest(di);

            SecurityProtectedNas *spn = EncapsulateNas(mmn);

            NS_LOG_INFO("[+] gGW -> AMF : SP 5GMM NAS");
            AccumulateProcessingDelay(PACKET_DECAPSULATION_DELAY + PACKET_GENERATION_DELAY + PACKET_ENCAPSULATION_DELAY);
            SendNas<SecurityProtectedNas>(GetSecondaryNetDevice(), spn);
        }
    }

    void gGateway::ReceiveFromSecondaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType)
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

    JoinRequestParams *gGateway::GetJoinRequestParams(Buffer::Iterator it)
    {
        NS_LOG_FUNCTION(this);

        JoinRequestParams *jrqstp = new JoinRequestParams();

        jrqstp->joinEui = (int)it.ReadU8();
        jrqstp->devEui = it.ReadU8();
        jrqstp->devNonce = it.ReadU8();
        jrqstp->mic = it.ReadU8();

        return jrqstp;
    }

    gGwDeviceInfo *gGateway::CreategGwDeviceInfo(JoinRequestParams *jrqstp)
    {
        NS_LOG_FUNCTION(this);

        gGwDeviceInfo *di = new gGwDeviceInfo();

        di->suci = di->supi = DeriveSuci(jrqstp->devEui);
        di->jrqstp = jrqstp;

        NS_LOG_INFO("[*] gGw saving device information : "
                    << "SUCI = " << (int)di->suci << " with Join Request Parameters");

        return di;
    }

    uint8_t gGateway::DeriveSuci(uint8_t devEui)
    {
        NS_LOG_FUNCTION(this);
        uint8_t suci = 2 * devEui - 9;

        NS_LOG_INFO("[*] gGw deriving device SUCI = " << (int)suci);
        return suci;
    }

    MobilityManagementNas *gGateway::TranslateAuthenticationRequest(gGwDeviceInfo *di)
    {
        NS_LOG_FUNCTION(this);
        NS_LOG_INFO("[*] gGw translating LoRaWAN JoinRequest to 5G AuthenticationRequest (5GMM NAS)");

        MobilityManagementNas *mmn = new MobilityManagementNas();
        mmn->securityHeaderType = PLAIN_MESSAGE;
        mmn->messageType = SERVICE_REQUEST;

        Tlv *suci = new Tlv();
        suci->type = SUCI;
        suci->length = 1;
        suci->value = new uint8_t[suci->length]{di->suci};

        mmn->ie->push_back(suci);

        return mmn;
    }

    void gGateway::ProcessMobilityManagementNas(MobilityManagementNas *mmn)
    {
        NS_LOG_FUNCTION(this);

        switch (mmn->messageType)
        {
        case EAP_RQST_CN:
        {
            gGwDeviceInfo *di = GetDeviceInfoBySupi<gGwDeviceInfo>(deviceInfoVector, mmn->ie->at(0));
            di->supi = mmn->ie->at(0)->value[0];
            MobilityManagementNas *r_mmn = GenerateEAPResponse(di);
            SecurityProtectedNas *spn = EncapsulateNas(r_mmn);
            NS_LOG_INFO("[+] gGw -> AMF : EAP-LoRaWAN-CN/EAP-RESP");
            SendNas<SecurityProtectedNas>(GetSecondaryNetDevice(), spn);

            break;
        }

        case EAP_SUCCESS_CN:
        {
            gGwDeviceInfo *di = GetDeviceInfoBySupi<gGwDeviceInfo>(deviceInfoVector, mmn->ie->at(0));
            di->jrespp = GetJoinResponseParams(mmn);
            di->kAmf = mmn->ie->at(4)->value[0];

            if (IsMicValid(di))
            {
                NS_LOG_INFO("[^$^] Primary Authentication Done");
                m_timeTrace();
                SetDeviceSessionParams(di);
                SessionManagementNas *smn = GeneratePduSessionEstablishmentRequest(di);
                SecurityProtectedNas *spn = EncapsulateNas(smn);
                NS_LOG_INFO("[+] gGW -> AMF : SP 5GSM NAS");
                SendNas<SecurityProtectedNas>(GetSecondaryNetDevice(), spn);
            }
            else
            {
                NS_LOG_FUNCTION("[<$>] Primary Authentication Failed");
            }
            break;
        }

        default:
            break;
        }
    }

    MobilityManagementNas *gGateway::GenerateEAPResponse(gGwDeviceInfo *di)
    {
        NS_LOG_FUNCTION(this);

        MobilityManagementNas *mmn = new MobilityManagementNas();

        mmn->securityHeaderType = PLAIN_MESSAGE;
        mmn->messageType = EAP_RESP_CN;

        Tlv *supi = new Tlv();
        supi->type = SUPI;
        supi->length = 1;
        supi->value = new uint8_t[supi->length]{di->supi};

        Tlv *joinEui = new Tlv();
        joinEui->type = JOIN_EUI;
        joinEui->length = 1;
        joinEui->value = new uint8_t[joinEui->length]{di->jrqstp->joinEui};

        Tlv *devEui = new Tlv();
        devEui->type = DEV_EUI;
        devEui->length = 1;
        devEui->value = new uint8_t[devEui->length]{di->jrqstp->devEui};

        Tlv *devNonce = new Tlv();
        devNonce->type = DEV_NONCE;
        devNonce->length = 1;
        devNonce->value = new uint8_t[devNonce->length]{di->jrqstp->devNonce};

        Tlv *mic = new Tlv();
        mic->type = MIC;
        mic->length = 1;
        mic->value = new uint8_t[mic->length]{di->jrqstp->mic};

        mmn->ie->push_back(supi);
        mmn->ie->push_back(joinEui);
        mmn->ie->push_back(devEui);
        mmn->ie->push_back(devNonce);
        mmn->ie->push_back(mic);

        NS_LOG_INFO("[*] gGw generating EAP-LoRaWAN-CN/EAP-RESP : "
                    << "JoinEui = " << (int)di->jrqstp->joinEui << ", "
                    << "DevEui = " << (int)di->jrqstp->devEui << ", "
                    << "DevNonce = " << (int)di->jrqstp->devNonce << ", "
                    << "MIC = " << (int)di->jrqstp->mic);

        return mmn;
    }

    JoinResponseParams *gGateway::GetJoinResponseParams(MobilityManagementNas *mmn)
    {
        NS_LOG_FUNCTION(this);

        JoinResponseParams *jrespp = new JoinResponseParams();

        jrespp->joinNonce = mmn->ie->at(1)->value[0];
        jrespp->params = mmn->ie->at(2)->value[0];
        jrespp->mic = mmn->ie->at(3)->value[0];

        NS_LOG_INFO("[*] gGw saving join response params");

        return jrespp;
    }

    bool gGateway::IsMicValid(gGwDeviceInfo *di)
    {
        NS_LOG_FUNCTION(this);

        uint8_t jSIntKey = di->kAmf ^ di->jrqstp->devEui ^ 0x05;

        uint8_t expectedMIC = Hash32(std::to_string(di->jrespp->joinNonce) +
                                     std::to_string(di->jrespp->params) +
                                     std::to_string(jSIntKey));

        AccumulateProcessingDelay(2 * XOR_TIME + HASH_TIME + 2 * CONC_TIME);

        NS_LOG_INFO("[*] gGW checking MIC : "
                    << "Expected MIC = " << (int)expectedMIC << ", "
                    << "Received MIC = " << (int)di->jrespp->mic);

        if (expectedMIC == di->jrespp->mic)
        {
            NS_LOG_INFO("[*] MIC is valid");
            return true;
        }
        else
        {
            NS_LOG_INFO("[*] MIC is invalid");
            return false;
        }
    }

    void gGateway::SetDeviceSessionParams(gGwDeviceInfo *di)
    {
        NS_LOG_FUNCTION(this);

        di->sp = new SessionParams();
        di->sp->pduSessionId = (uint8_t)(rand() % 100);
        di->sp->procedureTransactionId = (uint8_t)(rand() % 100);
        di->sp->nssai = (uint8_t)(rand() % 100);
        di->sp->sessionType = (uint8_t)(rand() % 100);

        NS_LOG_INFO("[*] gGw setting device session information : "
                    << "SUPI = " << (int)di->supi << ", "
                    << "PDU Session ID = " << (int)di->sp->pduSessionId << ", "
                    << "Procedure Transaction ID = " << (int)di->sp->pduSessionId << ", "
                    << "NSSAI = " << (int)di->sp->nssai << ", "
                    << "Session Type = " << (int)di->sp->sessionType);
    }

    SessionManagementNas *gGateway::GeneratePduSessionEstablishmentRequest(gGwDeviceInfo *di)
    {
        NS_LOG_FUNCTION(this);

        SessionManagementNas *smn = new SessionManagementNas();

        smn->pduSessionId = di->sp->pduSessionId;
        smn->procedureTransactionId = di->sp->procedureTransactionId;
        smn->messageType = SESSION_ESTABLISHMENT;

        Tlv *supi = new Tlv();
        supi->type = SUPI;
        supi->length = 1;
        supi->value = new uint8_t[supi->length]{di->supi};

        Tlv *nssai = new Tlv();
        nssai->type = NSSAI;
        nssai->length = 1;
        nssai->value = new uint8_t[nssai->length]{di->sp->nssai};

        Tlv *sessionType = new Tlv();
        sessionType->type = SESSION_TYPE;
        sessionType->length = 1;
        sessionType->value = new uint8_t[sessionType->length]{di->sp->sessionType};

        smn->ie->push_back(supi);
        smn->ie->push_back(nssai);
        smn->ie->push_back(sessionType);

        NS_LOG_INFO("[*] gGw generating PDU session establishment 5GSM NAS");

        return smn;
    }

    void gGateway::ProcessSessionManagementNas(SessionManagementNas *smn)
    {
        NS_LOG_FUNCTION(this);

        switch (smn->messageType)
        {

        case EAP_RQST_DN:
        {
            gGwDeviceInfo *di = GetDeviceInfoBySupi<gGwDeviceInfo>(deviceInfoVector, smn->ie->at(0));
            SessionManagementNas *r_smn = CreateEAPResponse(di);
            SecurityProtectedNas *spn = EncapsulateNas(r_smn);
            NS_LOG_INFO("[+] gGw -> AMF : EAP-LoRaWAN-DN/EAP-RESP");
            SendNas<SecurityProtectedNas>(GetSecondaryNetDevice(), spn);
            break;
        }

        case EAP_SUCCESS_DN:
        {
            m_timeTrace();
            NS_LOG_INFO("[^$^] Secondary Authentication Done");
      
            //gGwDeviceInfo *di = GetDeviceInfoBySupi<gGwDeviceInfo>(deviceInfoVector, smn->ie->at(0));
            //GetPrimaryNetDevice()->Send(CreateJoinAccpetPacket(di), GetBroadcast(), Ipv4L3Protocol::PROT_NUMBER);

            break;
        }

        default:
            break;
        }
    }

    SessionManagementNas *gGateway::CreateEAPResponse(gGwDeviceInfo *di)
    {
        NS_LOG_FUNCTION(this);

        SessionManagementNas *smn = new SessionManagementNas();

        smn->pduSessionId = di->sp->pduSessionId;
        smn->messageType = EAP_RESP_DN;

        Tlv *supi = new Tlv();
        supi->type = SUPI;
        supi->length = 1;
        supi->value = new uint8_t[supi->length]{di->supi};

        Tlv *joinEui = new Tlv();
        joinEui->type = JOIN_EUI;
        joinEui->length = 1;
        joinEui->value = new uint8_t[joinEui->length]{di->jrqstp->joinEui};

        Tlv *devNonce = new Tlv();
        devNonce->type = DEV_NONCE;
        devNonce->length = 1;
        devNonce->value = new uint8_t[devNonce->length]{di->jrqstp->devNonce};

        Tlv *joinNonce = new Tlv();
        joinNonce->type = JOIN_NONCE;
        joinNonce->length = 1;
        joinNonce->value = new uint8_t[joinNonce->length]{di->jrespp->joinNonce};

        smn->ie->push_back(supi);
        smn->ie->push_back(joinEui);
        smn->ie->push_back(devNonce);
        smn->ie->push_back(joinNonce);

        NS_LOG_INFO("[*] gGw generating EAP-LoRaWAN-DN/EAP-RESP : "
                    << "JoinEui = " << (int)di->jrqstp->joinEui << ", "
                    << "DevNonce = " << (int)di->jrqstp->devNonce << ", "
                    << "JoinNonce = " << (int)di->jrespp->joinNonce);

        return smn;
    }

    Ptr<Packet> gGateway::CreateJoinAccpetPacket(gGwDeviceInfo *di)
    {
        NS_LOG_FUNCTION(this);

        uint8_t length = 12;
        uint8_t *buf = new uint8_t[length]{0};

        // buf[0] = JOIN_ACCEPT;
        // buf[1] = joinEui;
        // buf[2] = devEui;
        // buf[3] = devNonce = (uint8_t)(rand() % 100);
        // buf[4] = Hash32(std::to_string(joinEui) +
        //                 std::to_string(devEui) +
        //                 std::to_string(devNonce) +
        //                 std::to_string(nwkKey));

        // NS_LOG_INFO("[+] End Device -> gGW : JoinRequest : "
        //             << "JoinEui = " << (int)joinEui << ", "
        //             << "DevEui = " << (int)devEui << ", "
        //             << "DevNonce = " << (int)devNonce << ", "
        //             << "MIC = " << (int)buf[4] << ", with "
        //             << "NwKKey = " << (int)nwkKey);
        
        Ptr<Packet> packet = Create<Packet>(buf, length);
        
        packet->AddHeader(di->fHdr);
        packet->AddHeader(di->mHdr);
        
        return packet;
    }
}
