/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/log.h"
#include "ns3/core-module.h"
#include "ns3/mobility-helper.h"
#include "ns3/point-to-point-module.h"

#include "ns3/end-device-lora-phy.h"
#include "ns3/end-device-lorawan-mac.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/gateway-lorawan-mac.h"
#include "ns3/forwarder-helper.h"
#include "ns3/lora-helper.h"

#include "ns3/g-gateway.h"
#include "ns3/access-mobility-management-function.h"
#include "ns3/authentication-server-function.h"
#include "ns3/unified-data-management.h"
#include "ns3/session-management-function.h"
#include "ns3/aaa-server.h"
#include "ns3/application-server.h"
#include "ns3/basic-net-device-helper.h"
#include "ns3/my-one-shot-sender-helper.h"
#include "ns3/forwarder.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("Lorawan5gIntegrationExample");

int main(int argc, char *argv[])
{
  bool verbose = true;
  uint8_t sf = 7;

  CommandLine cmd(__FILE__);
  cmd.AddValue("verbose", "Tell application to log if true", verbose);
  cmd.AddValue("sf", "Set the used Spreading Factor (SF) [SF7-SF12], default SF=7", sf);

  cmd.Parse(argc, argv);

  if (verbose)
  {
    LogComponentEnable("Lorawan5gIntegrationExample", LOG_LEVEL_INFO);
    LogComponentEnable("BasicNetDevice", LOG_LEVEL_INFO);
    LogComponentEnable("gGateway", LOG_LEVEL_INFO);
    LogComponentEnable("AccessMobilityManagementFunction", LOG_LEVEL_INFO);
    LogComponentEnable("AuthenticationServerFunction", LOG_LEVEL_INFO);
    LogComponentEnable("UnifiedDataManagement", LOG_LEVEL_INFO);
    LogComponentEnable("SessionManagementFunction", LOG_LEVEL_INFO);
    LogComponentEnable("AaaServer", LOG_LEVEL_INFO);
    LogComponentEnable("ApplicationServer", LOG_LEVEL_INFO);
    LogComponentEnable("MyOneShotSender", LOG_LEVEL_INFO);
  }

  /******************
   *  Create Nodes  *
   ******************/

  NodeContainer endDevices;
  endDevices.Create(1);

  Ptr<Node> gateway = CreateObject<Node>();
  Ptr<Node> gGw = CreateObject<Node>();
  Ptr<Node> amf = CreateObject<Node>();
  Ptr<Node> ausf = CreateObject<Node>();
  Ptr<Node> udm = CreateObject<Node>();
  Ptr<Node> smf = CreateObject<Node>();
  Ptr<Node> aaaServer = CreateObject<Node>();
  Ptr<Node> appServer = CreateObject<Node>();

  /********************************
   *  Create the LoRaWAN channel  *
   ********************************/

  NS_LOG_INFO("Creating the channel...");
  Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel>();
  loss->SetPathLossExponent(2.5);
  loss->SetReference(1, 2.5);

  Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel>();

  Ptr<LoraChannel> channel = CreateObject<LoraChannel>(loss, delay);

  /********************
   *  Create helpers  *
   ********************/

  // Create MobilityHelper
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> allocator = CreateObject<ListPositionAllocator>();
  allocator->Add(Vector(100, 0, 0));
  allocator->Add(Vector(0, 0, 0));
  mobility.SetPositionAllocator(allocator);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

  // Create LoraPhyHelper
  LoraPhyHelper phyHelper = LoraPhyHelper();
  phyHelper.SetChannel(channel);

  // Create LorawanMacHelper
  LorawanMacHelper macHelper = LorawanMacHelper();
  macHelper.SetRegion(LorawanMacHelper::EU);

  // Create LoraHelper
  LoraHelper helper = LoraHelper();

  // Create ForwarderHelper
  ForwarderHelper forwarderHelper;

  // Create PointToPointHelper
  PointToPointHelper p2pHelper;
  p2pHelper.SetChannelAttribute("Delay", StringValue("0ms"));

  // Create MyOneShotSenderHelper
  MyOneShotSenderHelper myOneShotSenderHelper;
  myOneShotSenderHelper.SetSendTime(Seconds(0));

  // Create AsciiTraceHelper
  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream("lorawan-channel.dat", std::ios_base::app);

  // Create BasicNetDeviceHelper
  BasicNetDeviceHelper basicNetDeviceHelper;

  /************************
   *  Create LoRaWAN RAN  *
   ************************/

  std::vector<int> sfQuantity(6);
  std::vector<double> distribution(6, 0);

  distribution.at(sf - 7) = 1;

  // Create the LoraNetDevices of the end devices
  phyHelper.SetDeviceType(LoraPhyHelper::ED);
  macHelper.SetDeviceType(LorawanMacHelper::ED_A);
  helper.Install(phyHelper, macHelper, endDevices);

  // Create a netdevice for each gateway
  phyHelper.SetDeviceType(LoraPhyHelper::GW);
  macHelper.SetDeviceType(LorawanMacHelper::GW);
  helper.Install(phyHelper, macHelper, gateway);

  myOneShotSenderHelper.Install(endDevices);

  mobility.Install(endDevices);
  mobility.Install(gateway);

  sfQuantity = macHelper.SetSpreadingFactorsGivenDistribution(endDevices, gateway, distribution);

  /****************
   *  Create 5GC  *
   ****************/

  Ptr<gGateway> gGwNetDevice = CreateObject<gGateway>();
  Ptr<AccessMobilityManagementFunction> amfNetDevice = CreateObject<AccessMobilityManagementFunction>();
  Ptr<AuthenticationServerFunction> ausfNetDevice = CreateObject<AuthenticationServerFunction>();
  Ptr<UnifiedDataManagement> udmNetDevice = CreateObject<UnifiedDataManagement>();
  Ptr<SessionManagementFunction> smfNetDevice = CreateObject<SessionManagementFunction>();
  Ptr<AaaServer> aaaServerNetDevice = CreateObject<AaaServer>();
  Ptr<ApplicationServer> appServerNetDevice = CreateObject<ApplicationServer>();

  NetDeviceContainer gatewaygGw = p2pHelper.Install(gateway, gGw);
  NetDeviceContainer gWAmf = p2pHelper.Install(gGw, amf);
  NetDeviceContainer amfAusf = p2pHelper.Install(amf, ausf);
  NetDeviceContainer ausfUdm = p2pHelper.Install(ausf, udm);
  NetDeviceContainer amfSmf = p2pHelper.Install(amf, smf);
  NetDeviceContainer smfAaaServer = p2pHelper.Install(smf, aaaServer);
  NetDeviceContainer aaaServerAppServer = p2pHelper.Install(aaaServer, appServer);

  gGw->AddDevice(gGwNetDevice);
  gGwNetDevice->SetNode(gGw);
  gGwNetDevice->SetPrimaryNetDevice(gatewaygGw.Get(1));
  gGwNetDevice->SetSecondaryNetDevice(gWAmf.Get(0));
  forwarderHelper.Install(gateway);

  amf->AddDevice(amfNetDevice);
  amfNetDevice->SetNode(amf);
  amfNetDevice->SetPrimaryNetDevice(gWAmf.Get(1));
  amfNetDevice->SetSecondaryNetDevice(amfAusf.Get(0));
  amfNetDevice->SetTertiaryNetDevice(amfSmf.Get(0));

  ausf->AddDevice(ausfNetDevice);
  ausfNetDevice->SetNode(ausf);
  ausfNetDevice->SetPrimaryNetDevice(amfAusf.Get(1));
  ausfNetDevice->SetSecondaryNetDevice(ausfUdm.Get(0));

  udm->AddDevice(udmNetDevice);
  udmNetDevice->SetNode(udm);
  udmNetDevice->SetPrimaryNetDevice(ausfUdm.Get(1));

  smf->AddDevice(smfNetDevice);
  smfNetDevice->SetNode(smf);
  smfNetDevice->SetPrimaryNetDevice(amfSmf.Get(1));
  smfNetDevice->SetSecondaryNetDevice(smfAaaServer.Get(0));

  aaaServer->AddDevice(aaaServerNetDevice);
  aaaServerNetDevice->SetNode(aaaServer);
  aaaServerNetDevice->SetPrimaryNetDevice(smfAaaServer.Get(1));
  aaaServerNetDevice->SetSecondaryNetDevice(aaaServerAppServer.Get(0));

  appServer->AddDevice(appServerNetDevice);
  appServerNetDevice->SetNode(appServer);
  appServerNetDevice->SetPrimaryNetDevice(aaaServerAppServer.Get(1));

  /* Tracing */
  basicNetDeviceHelper.EnableAscii(stream, gGwNetDevice);

  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
