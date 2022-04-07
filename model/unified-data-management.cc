
#include "unified-data-management.h"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("UnifiedDataManagement");

  NS_OBJECT_ENSURE_REGISTERED(UnifiedDataManagement);

  TypeId UnifiedDataManagement::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::UnifiedDataManagement")
                            .SetParent<BasicNetDevice>()
                            .SetGroupName("LoRaWAN5GIntegration")
                            .AddConstructor<UnifiedDataManagement>();

    return tid;
  }

  UnifiedDataManagement::UnifiedDataManagement()
  {
    NS_LOG_FUNCTION(this);
    deviceInfoVector = new std::vector<UdmDeviceInfo *>();

    UdmDeviceInfo *di = new UdmDeviceInfo();
    di->supi = 151;
    di->am = EAP_LORAWAN;
    di->sharedKey = 15;

    deviceInfoVector->push_back(di);
  }

  UnifiedDataManagement::~UnifiedDataManagement()
  {
    NS_LOG_FUNCTION(this);
  }

  void UnifiedDataManagement::ReceiveFromPrimaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType)
  {
    NS_LOG_FUNCTION(this);

    MobilityManagementNas *mmn = ReceiveNas<MobilityManagementNas>(GetPacketDataIterator(packet), packet->GetSize());
    ProcessMobilityManagementNas(mmn);
  }

  void UnifiedDataManagement::ReceiveFromSecondaryDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, Address const &source, Address const &destination, PacketType packetType)
  {
    NS_LOG_ERROR(this);
  }

  void UnifiedDataManagement::ProcessMobilityManagementNas(MobilityManagementNas *mmn)
  {
    NS_LOG_FUNCTION(this);

    switch (mmn->messageType)
    {
    case SERVICE_REQUEST:
    {
      Tlv *suci = mmn->ie->at(0);
      Tlv *snid = mmn->ie->at(1);

      Tlv *supi = GetSupi(suci);
      UdmDeviceInfo *di = GetDeviceInfoBySupi<UdmDeviceInfo>(deviceInfoVector, suci);

      if (di != nullptr)
      {
        switch (di->am)
        {
        case _5G_AKA:
        case EAP_AKA:
        case EAP_TLS:
          break;

        case EAP_LORAWAN:
        {
          Tlv *kAusf = DeriveKAusf(di->sharedKey);
          MobilityManagementNas *r_mmn = GenerateAuthenticationVectorMmn(suci, supi, snid, kAusf);
          NS_LOG_INFO("[+] UDM -> AUSF : Authentication Vector");
          SendNas<MobilityManagementNas>(GetPrimaryNetDevice(), r_mmn);
        }

        default:
          break;
        }
      }
    }

    default:
      break;
    }
  }

  Tlv *UnifiedDataManagement::GetSupi(Tlv *suci)
  {
    NS_LOG_FUNCTION(this);

    Tlv *supi = new Tlv();
    supi->type = SUPI;
    supi->length = suci->length;
    supi->value = new uint8_t[supi->length];

    memcpy(supi->value, suci->value, supi->length);

    NS_LOG_INFO("[*] UDM getting SUPI from SUCI : "
                << "SUCI = " << (int)suci->value[0] << ", "
                << "SUPI = " << (int)supi->value[0]);

    return supi;
  }

  Tlv *UnifiedDataManagement::DeriveKAusf(uint8_t sharedKey)
  {
    NS_LOG_FUNCTION(this);

    Tlv *kAusf = new Tlv();

    kAusf->type = KAUSF;
    kAusf->length = 1;
    kAusf->value = new uint8_t[1]{(uint8_t)(2 * sharedKey + 5)};

    NS_LOG_INFO("[*] UDM deriving KAUSF = NwKKey =  " << (int)kAusf->value[0]);

    return kAusf;
  }

  MobilityManagementNas *UnifiedDataManagement::GenerateAuthenticationVectorMmn(Tlv *suci, Tlv *supi, Tlv *snid, Tlv *kAusf)
  {
    NS_LOG_FUNCTION(this);

    MobilityManagementNas *mmn = new MobilityManagementNas();
    mmn->securityHeaderType = PLAIN_MESSAGE;
    mmn->messageType = EAP_RQST_CN;

    mmn->ie->push_back(suci);
    mmn->ie->push_back(supi);
    mmn->ie->push_back(snid);
    mmn->ie->push_back(kAusf);

    NS_LOG_INFO("[*] UDM generating authentication vector : "
                << "SUCI = " << (int)suci->value[0] << ", "
                << "SUPI = " << (int)supi->value[0] << ", "
                << "SNID = " << (int)snid->value[0] << ", "
                << "KAUSF = " << (int)kAusf->value[0]);

    return mmn;
  }
}