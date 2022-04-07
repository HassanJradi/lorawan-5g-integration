
#ifndef MY_ONE_SHOT_SENDER_H
#define MY_ONE_SHOT_SENDER_H

#include "ns3/application.h"
#include "ns3/nstime.h"
#include "ns3/lorawan-mac.h"
#include "ns3/attribute.h"

namespace ns3
{
  class MyOneShotSender : public Application
  {
  public:
    MyOneShotSender();
    MyOneShotSender(Time sendTime);
    ~MyOneShotSender();

    static TypeId GetTypeId(void);

    /**
     * Send a join request using the LoraNetDevice's Send method.
     */
    void SendJoinRequest(void);

    /**
     * Set the time at which this app will send a packet.
     */
    void SetSendTime(Time sendTime);

    /**
     * Start the application by scheduling the first JoinRequest event.
     */
    void StartApplication(void);

    /**
     * Stop the application.
     */
    void StopApplication(void);

  private:
    Ptr<Packet> CreateJoinRequestPacket();
    /**/ uint8_t DeriveNetworkKey(uint8_t sharedkey);

    /**
     * The time at which to send the packet.
     */
    Time m_sendTime;

    /**
     * The sending event.
     */
    EventId m_sendEvent;

    /**
     * The MAC layer of this node.
     */
    Ptr<lorawan::LorawanMac> m_mac;

    uint8_t sharedkey = 15;
    uint8_t joinEui = 25;
    uint8_t devEui = 80;
    uint8_t devNonce;
  };

} // namespace ns3
#endif /* ONE_SHOT_APPLICATION */
