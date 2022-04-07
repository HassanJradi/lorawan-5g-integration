
#include "ns3/my-one-shot-sender.h"
#include "ns3/class-a-end-device-lorawan-mac.h"
#include "ns3/pointer.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/basic-struct.h"
#include "ns3/lora-net-device.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/csma-net-device.h"
#include "ns3/net-device.h"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("MyOneShotSender");

  NS_OBJECT_ENSURE_REGISTERED(MyOneShotSender);

  TypeId MyOneShotSender::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::MyOneShotSender")
                            .SetParent<Application>()
                            .AddConstructor<MyOneShotSender>()
                            .SetGroupName("LoRaWAN5GIntegration");
    return tid;
  }

  MyOneShotSender::MyOneShotSender()
  {
    NS_LOG_FUNCTION_NOARGS();
  }

  MyOneShotSender::MyOneShotSender(Time sendTime)
      : m_sendTime(sendTime)
  {
    NS_LOG_FUNCTION_NOARGS();
  }

  MyOneShotSender::~MyOneShotSender()
  {
    NS_LOG_FUNCTION_NOARGS();
  }

  void MyOneShotSender::SetSendTime(Time sendTime)
  {
    NS_LOG_FUNCTION(this << sendTime);

    m_sendTime = sendTime;
  }

  void MyOneShotSender::StartApplication(void)
  {
    NS_LOG_FUNCTION(this);

    if (m_mac == 0)
    {
      Ptr<lorawan::LoraNetDevice> loraNetDevice = m_node->GetDevice(0)->GetObject<lorawan::LoraNetDevice>();
      m_mac = loraNetDevice->GetMac();
      NS_ASSERT(m_mac != 0);
    }

    Simulator::Cancel(m_sendEvent);
    m_sendEvent = Simulator::Schedule(m_sendTime, &MyOneShotSender::SendJoinRequest, this);
  }

  void MyOneShotSender::StopApplication(void)
  {
    NS_LOG_FUNCTION_NOARGS();
    Simulator::Cancel(m_sendEvent);
  }

  void MyOneShotSender::SendJoinRequest(void)
  {
    NS_LOG_FUNCTION(this);
    m_mac->Send(CreateJoinRequestPacket());
    // NS_LOG_INFO("Datarate: " << (int)m_node->GetDevice(0)->GetObject<ns3::lorawan::LoraNetDevice>()->GetMac()->GetObject<ns3::lorawan::ClassAEndDeviceLorawanMac>()->GetDataRate());
    // m_node->GetDevice(0)->Send(CreateJoinRequestPacket(), m_node->GetDevice(0)->GetBroadcast(), Ipv4L3Protocol::PROT_NUMBER);
  }

  Ptr<Packet> MyOneShotSender::CreateJoinRequestPacket()
  {
    NS_LOG_FUNCTION(this);

    uint8_t nwkKey = DeriveNetworkKey(sharedkey);

    uint8_t length = 26;
    uint8_t *buf = new uint8_t[length]{0};

    buf[0] = JOIN_REQUEST;
    buf[1] = joinEui;
    buf[2] = devEui;
    buf[3] = devNonce = (uint8_t)(rand() % 100);
    buf[4] = Hash32(std::to_string(joinEui) +
                    std::to_string(devEui) +
                    std::to_string(devNonce) +
                    std::to_string(nwkKey));

    NS_LOG_INFO("[+] End Device -> gGW : JoinRequest : "
                << "JoinEui = " << (int)joinEui << ", "
                << "DevEui = " << (int)devEui << ", "
                << "DevNonce = " << (int)devNonce << ", "
                << "MIC = " << (int)buf[4] << ", with "
                << "NwKKey = " << (int)nwkKey);

    return Create<Packet>(buf, length);
  }

  uint8_t MyOneShotSender::DeriveNetworkKey(uint8_t sharedkey)
  {
    NS_LOG_FUNCTION(this);

    uint8_t nwkKey = 2 * sharedkey + 5;

    NS_LOG_INFO("[*] End Device deriving Network Key = " << (int)nwkKey
                                                         << " from Shared Key = " << (int)sharedkey);

    return nwkKey;
  }

}
