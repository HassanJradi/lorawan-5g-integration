
#include "authentication-server-function.h"

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE("AuthenticationServerFunction");

    NS_OBJECT_ENSURE_REGISTERED(AuthenticationServerFunction);

    TypeId AuthenticationServerFunction::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::AuthenticationServerFunction")
                                .SetParent<BasicNetDevice>()
                                .SetGroupName("LoRaWAN5GIntegration")
                                .AddConstructor<AuthenticationServerFunction>();

        return tid;
    }

    AuthenticationServerFunction::AuthenticationServerFunction()
    {
        NS_LOG_FUNCTION(this);
        deviceInfoVector = new std::vector<AusfDeviceInfo *>();
    }

    AuthenticationServerFunction::~AuthenticationServerFunction()
    {
        NS_LOG_FUNCTION(this);
    }

    void AuthenticationServerFunction::ReceiveFromPrimaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType)
    {
        NS_LOG_FUNCTION(this);

        MobilityManagementNas *mmn = ReceiveNas<MobilityManagementNas>(GetPacketDataIterator(packet), packet->GetSize());
        ProcessMobilityManagementNas(mmn);
    }

    void AuthenticationServerFunction::ReceiveFromSecondaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType)
    {
        NS_LOG_FUNCTION(this);

        MobilityManagementNas *mmn = ReceiveNas<MobilityManagementNas>(GetPacketDataIterator(packet), packet->GetSize());
        ProcessMobilityManagementNas(mmn);
    }

    void AuthenticationServerFunction::ProcessMobilityManagementNas(MobilityManagementNas *mmn)
    {
        NS_LOG_FUNCTION(this);

        switch (mmn->messageType)
        {
        case SERVICE_REQUEST:
        {
            NS_LOG_INFO("[+] AUSF -> UDM : 5GMM NAS");
            SendNas<MobilityManagementNas>(GetSecondaryNetDevice(), mmn);
            break;
        }

        case EAP_RQST_CN:
        {
            AusfDeviceInfo *di = CreateAusfDeviceInfo(mmn);
            deviceInfoVector->push_back(di);
            MobilityManagementNas *r_mmn = GenerateEAPRequest(di);
            NS_LOG_INFO("[+] AUSF -> AMF : EAP-LoRaWAN-CN/EAP-RQST");
            SendNas<MobilityManagementNas>(GetPrimaryNetDevice(), r_mmn);
            break;
        }

        case EAP_RESP_CN:
        {
            AusfDeviceInfo *di = GetDeviceInfoBySupi<AusfDeviceInfo>(deviceInfoVector, mmn->ie->at(0));
            JoinRequestParams *jrqstp = GetJoinRequestParams(mmn);

            if (IsMicValid(di, jrqstp))
            {
                uint8_t joinNonce = (uint8_t)(rand() % 100);
                uint8_t *sessionkeys = GenerateLoRaWANSessionKeys(di, jrqstp, joinNonce);
                JoinResponseParams *jrespp = GenerateJoinRespParams(di, joinNonce, sessionkeys[2]);
                MobilityManagementNas *r_mmn = GenerateEAPSuccess(di, sessionkeys, jrespp);
                NS_LOG_INFO("[+] AUSF -> AMF : EAP-LoRaWAN-CN/EAP-SUCCESS");
                SendNas<MobilityManagementNas>(GetPrimaryNetDevice(), r_mmn);
            }
            else
            {
                NS_LOG_FUNCTION("[---] Primary Authentication Failed");
            }
            break;
        }

        default:
            break;
        }
    }

    AusfDeviceInfo *AuthenticationServerFunction::CreateAusfDeviceInfo(MobilityManagementNas *mmn)
    {
        NS_LOG_FUNCTION(this);

        AusfDeviceInfo *di = new AusfDeviceInfo();

        di->suci = mmn->ie->at(0)->value[0];
        di->supi = mmn->ie->at(1)->value[0];
        di->snid = mmn->ie->at(2)->value[0];
        di->kAusf = mmn->ie->at(3)->value[0];
        di->kAmf = di->kAusf + 5;

        NS_LOG_INFO("[*] AUSF saving authentication vector and device information : "
                    << "SUCI = " << (int)di->suci << ", "
                    << "SUPI = " << (int)di->supi << ", "
                    << "SNID = " << (int)di->snid << ", "
                    << "KAUSF = " << (int)di->kAusf);

        return di;
    }

    MobilityManagementNas *AuthenticationServerFunction::GenerateEAPRequest(AusfDeviceInfo *di)
    {
        NS_LOG_FUNCTION(this);

        MobilityManagementNas *mmn = new MobilityManagementNas();

        mmn->securityHeaderType = PLAIN_MESSAGE;
        mmn->messageType = EAP_RQST_CN;

        Tlv *suci = new Tlv();
        suci->type = SUCI;
        suci->length = 1;
        suci->value = new uint8_t[suci->length]{di->suci};

        Tlv *supi = new Tlv();
        supi->type = SUPI;
        supi->length = 1;
        supi->value = new uint8_t[supi->length]{di->supi};

        mmn->ie->push_back(suci);
        mmn->ie->push_back(supi);

        NS_LOG_INFO("[*] AUSF generating EAP-LoRaWAN-CN/EAP-RQST : "
                    << "SUCI = " << (int)di->suci << ", "
                    << "SUPI = " << (int)di->supi);

        return mmn;
    }

    JoinRequestParams *AuthenticationServerFunction::GetJoinRequestParams(MobilityManagementNas *mmn)
    {
        NS_LOG_FUNCTION(this);

        JoinRequestParams *jrqstp = new JoinRequestParams();

        jrqstp->joinEui = mmn->ie->at(1)->value[0];
        jrqstp->devEui = mmn->ie->at(2)->value[0];
        jrqstp->devNonce = mmn->ie->at(3)->value[0];
        jrqstp->mic = mmn->ie->at(4)->value[0];

        return jrqstp;
    }

    bool AuthenticationServerFunction::IsMicValid(AusfDeviceInfo *di, JoinRequestParams *jrqstp)
    {
        NS_LOG_FUNCTION(this);

        uint8_t expectedMIC = Hash32(std::to_string(jrqstp->joinEui) +
                                     std::to_string(jrqstp->devEui) +
                                     std::to_string(jrqstp->devNonce) +
                                     std::to_string(di->kAusf));

        AccumulateProcessingDelay(HASH_TIME + 3 * CONC_TIME);

        NS_LOG_INFO("[*] AUSF checking MIC : "
                    << "Expected MIC = " << (int)expectedMIC << ", "
                    << "Received MIC = " << (int)jrqstp->mic);

        if (expectedMIC == jrqstp->mic)
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

    uint8_t *AuthenticationServerFunction::GenerateLoRaWANSessionKeys(AusfDeviceInfo *di, JoinRequestParams *jrqstp, uint8_t joinNonce)
    {
        NS_LOG_FUNCTION(this);

        uint8_t *sessionKeys = new uint8_t[4];

        uint8_t nwkSEncKey = di->kAmf ^ joinNonce ^ jrqstp->joinEui ^ jrqstp->devNonce ^ 0x03;
        uint8_t sNwkSIntKey = di->kAmf ^ joinNonce ^ jrqstp->joinEui ^ jrqstp->devNonce ^ 0x04;
        uint8_t jSIntKey = di->kAmf ^ jrqstp->devEui ^ 0x05;
        uint8_t jSEncKey = di->kAmf ^ jrqstp->devEui ^ 0x06;

        AccumulateProcessingDelay(12 * XOR_TIME);

        sessionKeys[0] = nwkSEncKey;
        sessionKeys[1] = sNwkSIntKey;
        sessionKeys[2] = jSIntKey;
        sessionKeys[3] = jSEncKey;

        NS_LOG_INFO("[*] AUSF generating LoRaWAN session keys : "
                    << "SUPI = " << (int)di->supi << ", "
                    << "NwkSEncKey = " << (int)nwkSEncKey << ", "
                    << "SNwkSIntKey = " << (int)sNwkSIntKey << ", "
                    << "JSIntKey = " << (int)jSIntKey << ", "
                    << "JSEncKey = " << (int)jSEncKey);

        return sessionKeys;
    }

    JoinResponseParams *AuthenticationServerFunction::GenerateJoinRespParams(AusfDeviceInfo *di, uint8_t joinNonce, uint8_t jSIntKey)
    {
        NS_LOG_FUNCTION(this);

        JoinResponseParams *jrespp = new JoinResponseParams();

        jrespp->joinNonce = (uint8_t)(rand() % 100);
        jrespp->params = (uint8_t)(rand() % 100);
        jrespp->mic = Hash32(std::to_string(jrespp->joinNonce) +
                             std::to_string(jrespp->params) +
                             std::to_string(jSIntKey));

        AccumulateProcessingDelay(3 * CONC_TIME + HASH_TIME);

        NS_LOG_INFO("[*] AUSF generating join response params : "
                    << "SUPI = " << (int)di->supi << ", "
                    << "JoinNonce = " << (int)jrespp->joinNonce << ", "
                    << "Params = " << (int)jrespp->params << ", "
                    << "MIC = " << (int)jrespp->mic);

        return jrespp;
    }

    MobilityManagementNas *AuthenticationServerFunction::GenerateEAPSuccess(AusfDeviceInfo *di, uint8_t *sessionkeys, JoinResponseParams *jrespp)
    {
        NS_LOG_FUNCTION(this);

        MobilityManagementNas *mmn = new MobilityManagementNas();

        mmn->securityHeaderType = INTEGRITY_PROTECTED_AND_CIPHERED;
        mmn->messageType = EAP_SUCCESS_CN;

        Tlv *supi = new Tlv();
        supi->type = SUPI;
        supi->length = 1;
        supi->value = new uint8_t[supi->length]{di->supi};

        Tlv *joinNonce = new Tlv();
        joinNonce->type = JOIN_NONCE;
        joinNonce->length = 1;
        joinNonce->value = new uint8_t[joinNonce->length]{jrespp->joinNonce};

        Tlv *params = new Tlv();
        params->type = PARAMS;
        params->length = 1;
        params->value = new uint8_t[params->length]{jrespp->params};

        Tlv *mic = new Tlv();
        mic->type = MIC;
        mic->length = 1;
        mic->value = new uint8_t[mic->length]{jrespp->mic};

        Tlv *kAmf = new Tlv();
        kAmf->type = KAMF;
        kAmf->length = 1;
        kAmf->value = new uint8_t[kAmf->length]{di->kAmf};

        Tlv *sessionKeysTlv = new Tlv();
        sessionKeysTlv->type = SESSION_KEYS;
        sessionKeysTlv->length = 4;
        sessionKeysTlv->value = new uint8_t[sessionKeysTlv->length]{sessionkeys[0], sessionkeys[1], sessionkeys[2], sessionkeys[3]};

        mmn->ie->push_back(supi);
        mmn->ie->push_back(joinNonce);
        mmn->ie->push_back(params);
        mmn->ie->push_back(mic);
        mmn->ie->push_back(kAmf);
        mmn->ie->push_back(sessionKeysTlv);

        NS_LOG_INFO("[*] AUSF generating EAP-LoRaWAN-CN/EAP-SUCCESS containing join response params and session keys");

        return mmn;
    }
};
