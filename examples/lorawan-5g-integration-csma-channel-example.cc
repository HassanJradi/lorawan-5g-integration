/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/log.h"
#include "ns3/core-module.h"
#include "ns3/csma-helper.h"
#include "ns3/point-to-point-module.h"

#include "ns3/g-gateway.h"
#include "ns3/access-mobility-management-function.h"
#include "ns3/authentication-server-function.h"
#include "ns3/unified-data-management.h"
#include "ns3/session-management-function.h"
#include "ns3/aaa-server.h"
#include "ns3/application-server.h"
#include "ns3/basic-net-device-helper.h"
#include "ns3/my-one-shot-sender-helper.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("Lorawan5gIntegrationCsmaChannelExample");

int main(int argc, char *argv[])
{
  bool verbose = true;
  uint64_t dataRate = 21900;
  uint64_t delay = 20;

  CommandLine cmd(__FILE__);
  cmd.AddValue("verbose", "Tell application to log if true", verbose);
  cmd.AddValue("data-rate", "Channel data rate (bps)", dataRate);
  cmd.AddValue("delay", "Channel delay (ms)", delay);

  cmd.Parse(argc, argv);

  if (verbose)
  {
    LogComponentEnable("BasicNetDevice", LOG_LEVEL_INFO);
    LogComponentEnable("gGateway", LOG_LEVEL_INFO);
    LogComponentEnable("AccessMobilityManagementFunction", LOG_LEVEL_INFO);
    LogComponentEnable("AuthenticationServerFunction", LOG_LEVEL_INFO);
    LogComponentEnable("UnifiedDataManagement", LOG_LEVEL_INFO);
    LogComponentEnable("SessionManagementFunction", LOG_LEVEL_INFO);
    LogComponentEnable("AaaServer", LOG_LEVEL_INFO);
    LogComponentEnable("ApplicationServer", LOG_LEVEL_INFO);
    LogComponentEnable("MyOneShotSender", LOG_LEVEL_INFO);
    LogComponentEnable("BasicNetDeviceHelper", LOG_INFO);
  }

  NS_LOG_INFO("Creating the gateway...");
  Ptr<Node> endDevice = CreateObject<Node>();
  Ptr<Node> gateway = CreateObject<Node>();
  Ptr<Node> gGw = CreateObject<Node>();
  Ptr<Node> amf = CreateObject<Node>();
  Ptr<Node> ausf = CreateObject<Node>();
  Ptr<Node> udm = CreateObject<Node>();
  Ptr<Node> smf = CreateObject<Node>();
  Ptr<Node> aaaServer = CreateObject<Node>();
  Ptr<Node> appServer = CreateObject<Node>();

  Ptr<gGateway> gGwNetDevice = CreateObject<gGateway>();
  Ptr<AccessMobilityManagementFunction> amfNetDevice = CreateObject<AccessMobilityManagementFunction>();
  Ptr<AuthenticationServerFunction> ausfNetDevice = CreateObject<AuthenticationServerFunction>();
  Ptr<UnifiedDataManagement> udmNetDevice = CreateObject<UnifiedDataManagement>();
  Ptr<SessionManagementFunction> smfNetDevice = CreateObject<SessionManagementFunction>();
  Ptr<AaaServer> aaaServerNetDevice = CreateObject<AaaServer>();
  Ptr<ApplicationServer> appServerNetDevice = CreateObject<ApplicationServer>();

  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", StringValue(std::to_string(dataRate) + "bps"));
  csma.SetChannelAttribute("Delay", StringValue(std::to_string(delay) + "ms"));

  PointToPointHelper p2pHelper;
  p2pHelper.SetChannelAttribute("Delay", StringValue("0ms"));

  NodeContainer radioNetwork(endDevice, gGw);

  NetDeviceContainer endDevicegGw = csma.Install(radioNetwork);
  NetDeviceContainer gWAmf = p2pHelper.Install(gGw, amf);
  NetDeviceContainer amfAusf = p2pHelper.Install(amf, ausf);
  NetDeviceContainer ausfUdm = p2pHelper.Install(ausf, udm);
  NetDeviceContainer amfSmf = p2pHelper.Install(amf, smf);
  NetDeviceContainer smfAaaServer = p2pHelper.Install(smf, aaaServer);
  NetDeviceContainer aaaServerAppServer = p2pHelper.Install(aaaServer, appServer);

  gGw->AddDevice(gGwNetDevice);
  gGwNetDevice->SetNode(gGw);
  gGwNetDevice->SetPrimaryNetDevice(endDevicegGw.Get(1));
  gGwNetDevice->SetSecondaryNetDevice(gWAmf.Get(0));

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

  MyOneShotSenderHelper myOneShotSenderHelper;
  myOneShotSenderHelper.SetSendTime(Seconds(0));
  myOneShotSenderHelper.Install(endDevice);

  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream(std::to_string(delay) + "ms" + ".dat", std::ios_base::app);
  Ptr<OutputStreamWrapper> stream2 = asciiTraceHelper.CreateFileStream("csma.dat", std::ios_base::app);

  BasicNetDeviceHelper basicNetDeviceHelper;
  basicNetDeviceHelper.EnableAscii(stream, gGwNetDevice);

  csma.EnableAscii(stream2, endDevicegGw.Get(0));

  Simulator::Run();
  Simulator::Destroy();
  return 0;
}