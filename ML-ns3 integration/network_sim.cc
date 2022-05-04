/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*

  Author: Sharday Olowu
  Date: 03/05/2022

  Sources:
  This work is based upon taxi_sim_2_cells.cc (author Sharday Olowu) and sim.cc. Sim.cc is an example 
  program written by Piotr Gawlowicz (available in the ns3-gym Github repository 
  https://github.com/tkn-tub/ns3-gym at scratch/opengym).

  Modifications:
  ➢ Implementation of ns3-gym callback functions, such as MyGetObservationSpace and MyGetActionSpace, 
  to define observation and action space, as well as MyGetObservation for collection of network 
  measurement observations
  ➢ Setting of OpenGym interface parameters and initialisation of interface at appropriate point
  ➢ Setting of callback functions to incorporate the interface into the existing network simulation

 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/config-store-module.h"
#include <iostream>
#include <fstream>
#include <list>
#include <sstream>
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "ns3/log.h"
#include "ns3/opengym-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("OpenGym");



std::vector<int> RSRPs(2, 0); 
std::vector<int> RSRQs(2, 0); 

std::ofstream myfile;
uint16_t numberOfUes = 1;
uint16_t numberOfEnbs = 2;

// std::string fileName = std::to_string(99);
std::string fileName = "test";


// free memory used for x and y co-ordinates

void freeMem(double* x, double* y) {
    free(x);
    free(y);
}

void setPos(double* x, double* y, int i) {

    std::cout <<  Simulator::Now().GetSeconds();

    Ptr<Node> n0 =  ns3::NodeList::GetNode(0);

    Ptr<MobilityModel> m0 = n0->GetObject<MobilityModel> ();

    // std::cout << " setting pos x: " << x[i] << " y: " <<  y[i];

    m0->SetPosition (Vector (x[i], y[i], 0));

    // std::cout << " set pos x: " << m0->GetPosition().x << " y: " << m0->GetPosition().y << std::endl;
}



void
NotifyConnectionEstablishedUe (std::string context,
                               uint64_t imsi,
                               uint16_t cellid,
                               uint16_t rnti)
{
  std::cout << context
            << " UE IMSI " << imsi
            << ": connected to CellId " << cellid
            << " with RNTI " << rnti
            << std::endl;
}

void
NotifyHandoverStartUe (std::string context,
                       uint64_t imsi,
                       uint16_t cellid,
                       uint16_t rnti,
                       uint16_t targetCellId)
{
  std::cout << context
            << " UE IMSI " << imsi
            << ": previously connected to CellId " << cellid
            << " with RNTI " << rnti
            << ", doing handover to CellId " << targetCellId
            << std::endl;
}

void
NotifyHandoverEndOkUe (std::string context,
                       uint64_t imsi,
                       uint16_t cellid,
                       uint16_t rnti)
{
  std::cout << context
            << " UE IMSI " << imsi
            << ": successful handover to CellId " << cellid
            << " with RNTI " << rnti
            << std::endl;
}

void
NotifyConnectionEstablishedEnb (std::string context,
                                uint64_t imsi,
                                uint16_t cellid,
                                uint16_t rnti)
{
  std::cout << context
            << " eNB CellId " << cellid
            << ": successful connection of UE with IMSI " << imsi
            << " RNTI " << rnti
            << std::endl;
}

void
NotifyHandoverStartEnb (std::string context,
                        uint64_t imsi,
                        uint16_t cellid,
                        uint16_t rnti,
                        uint16_t targetCellId)
{
  std::cout << context
            << " eNB CellId " << cellid
            << ": start handover of UE with IMSI " << imsi
            << " RNTI " << rnti
            << " to CellId " << targetCellId
            << std::endl;
}

void
NotifyHandoverEndOkEnb (std::string context,
                        uint64_t imsi,
                        uint16_t cellid,
                        uint16_t rnti)
{
  std::cout << context
            << " eNB CellId " << cellid
            << ": completed handover of UE with IMSI " << imsi
            << " RNTI " << rnti
            << std::endl;
}



void printToTerminal(Vector pos, uint16_t cellid, uint16_t rsrp, uint16_t rsrq, std::list<LteRrcSap::MeasResultEutra> const &list)
{
    // current serving cell
    NS_LOG_UNCOND ("time:" << Simulator::Now().GetSeconds() << "\t" << " x = " << pos.x << "\t" << "y = " << pos.y);
    std::cout<< " CellId=" << cellid 
            << " RSRQ=" 
            << rsrq
            <<" RSRP=" 
            << rsrp
           << "MeasResultEutra: " << std::endl;

    // other cells
    for (auto it = list.cbegin(); it != list.cend(); it++) {
        std::cout << " Cell ID=" << it->physCellId
                  << " RSRQ=" << (uint16_t)it->rsrqResult
                  << " RSRP=" << (uint16_t)it->rsrpResult
                  << "\n" << "------------------------------------------------------"
                  << std::endl;
    }
}



void outputResults(Vector pos, uint16_t cellid, uint16_t rsrp, uint16_t rsrq, std::list<LteRrcSap::MeasResultEutra> measResultListEutra) {

    myfile.open ("/mnt/c/Users/shard/Documents/workspace/sim_outputs_4_cells/spaced/" + fileName + ".txt", std::ios_base::app);

    // record position
    myfile << Simulator::Now().GetSeconds() << ","
            << pos.x << ","
            << pos.y << ",";
    
    // RSRP values
    
    std::map<int, int> rsrpMap;
    rsrpMap.insert({cellid, rsrp}); //current cell
    for (auto it = measResultListEutra.cbegin(); it != measResultListEutra.cend(); it++) {
      rsrpMap.insert({(uint16_t) it->physCellId, (uint16_t) it->rsrpResult});
    }

    for (uint16_t i = 1; i <= numberOfEnbs; ++i) {
      auto it = rsrpMap.find(i);
      myfile << it->second << ",";
      RSRPs[i-1] = it->second;
      
    }

    // RSRQ values
    
    std::map<int, int> rsrqMap;
    rsrqMap.insert({cellid, rsrq}); //current cell
    for (auto it = measResultListEutra.cbegin(); it != measResultListEutra.cend(); it++) {
      rsrqMap.insert({(uint16_t) it->physCellId, (uint16_t) it->rsrqResult});
    }

    for (uint16_t i = 1; i <= numberOfEnbs; ++i) {
      auto it = rsrqMap.find(i);
      myfile << it->second << ",";
      RSRQs[i-1] = it->second;
    }

    myfile << cellid << "\n";
    myfile.close();


}


void ExtractMeasureMentReport (NodeContainer *ueNodes,
                              std::string context, 
                              uint64_t imsi, 
                              uint16_t cellid, 
                              uint16_t rnti,
                              LteRrcSap::MeasurementReport msg)
{


    uint16_t rsrp = (uint16_t) msg.measResults.rsrpResult;  
    uint16_t rsrq = (uint16_t) msg.measResults.rsrqResult;


    std::list<LteRrcSap::MeasResultEutra> measResultListEutra = msg.measResults.measResultListEutra;

    // get current position

    Ptr<Node> n0 =  ueNodes->Get(0);

    Ptr<MobilityModel> m0 = n0->GetObject<MobilityModel> ();

    Vector pos = m0->GetPosition();
    

    // write results to file
    outputResults(pos, cellid, rsrp, rsrq, measResultListEutra);

    // print results to terminal
    printToTerminal(pos, cellid, rsrp, rsrq, measResultListEutra);
    
}


/*
Define observation space
*/
Ptr<OpenGymSpace> MyGetObservationSpace(void)
{
  uint32_t nodeNum = 4;
  float low = 0.0;
  float high = 10.0;
  std::vector<uint32_t> shape = {nodeNum,};
  std::string dtype = TypeNameGet<uint32_t> ();
  Ptr<OpenGymBoxSpace> space = CreateObject<OpenGymBoxSpace> (low, high, shape, dtype);
  NS_LOG_UNCOND ("MyGetObservationSpace: " << space);
  return space;
}

/*
Define action space
*/
Ptr<OpenGymSpace> MyGetActionSpace(void)
{
  uint32_t nodeNum = 3;
  std::vector<uint32_t> shape = {nodeNum,};
  // Ptr<OpenGymDiscreteSpace> space = CreateObject<OpenGymDiscreteSpace> (nodeNum);
  std::string dtype = TypeNameGet<float> (); //
  Ptr<OpenGymBoxSpace> space = CreateObject<OpenGymBoxSpace> (-100, 100, shape, dtype);
  NS_LOG_UNCOND ("MyGetActionSpace: " << space);
  return space;
}

/*
Define game over condition
*/
bool MyGetGameOver(void)
{

  bool isGameOver = false;
  bool test = false;
  static float stepCounter = 0.0;
  stepCounter += 1;
  if (stepCounter == 10 && test) {
      isGameOver = true;
  }
  NS_LOG_UNCOND ("MyGetGameOver: " << isGameOver);
  return isGameOver;
}

/*
Collect observations
*/
Ptr<OpenGymDataContainer> MyGetObservation(void)
{
  uint32_t nodeNum = 4;
  // uint32_t low = 0.0;
  // uint32_t high = 10.0;
  // Ptr<UniformRandomVariable> rngInt = CreateObject<UniformRandomVariable> ();

  std::vector<uint32_t> shape = {nodeNum,};
  Ptr<OpenGymBoxContainer<uint32_t> > box = CreateObject<OpenGymBoxContainer<uint32_t> >(shape);

  // generate random data
  // for (uint32_t i = 0; i<nodeNum; i++){
    // uint32_t value = rngInt->GetInteger(low, high);
    // box->AddValue(value);
  //   box->AddValue(i);
  // }

  box->AddValue(RSRPs[0]);
  box->AddValue(RSRPs[1]);
  box->AddValue(RSRQs[0]);
  box->AddValue(RSRQs[1]);

  NS_LOG_UNCOND ("MyGetObservation: " << box);
  return box;
}

/*
Define reward function
*/
float MyGetReward(void)
{
  static float reward = 0.0;
  reward += 1;
  return reward;
}

/*
Define extra info. Optional
*/
std::string MyGetExtraInfo(void)
{
  std::string myInfo = "testInfo";
  myInfo += "|123";
  NS_LOG_UNCOND("MyGetExtraInfo: " << myInfo);
  return myInfo;
}


/*
Execute received actions
*/
bool MyExecuteActions(Ptr<OpenGymDataContainer> action)
{
  // Ptr<OpenGymDiscreteContainer> discrete = DynamicCast<OpenGymDiscreteContainer>(action);
  Ptr<OpenGymBoxSpace> space = DynamicCast<OpenGymBoxSpace> (action);
  NS_LOG_UNCOND ("MyExecuteActions: " << action);
  return true;
}

void ScheduleNextStateRead(double envStepTime, Ptr<OpenGymInterface> openGym)
{
  Simulator::Schedule (Seconds(envStepTime), &ScheduleNextStateRead, envStepTime, openGym);
  openGym->NotifyCurrentState();
}


int
main (int argc, char *argv[])
{

  uint16_t numBearersPerUe = 0;
  double enbTxPowerDbm = 46.0;

  // Parameters of the scenario
  uint32_t simSeed = 1;
  double simulationTime = 1000; //seconds
  double envStepTime = 0.48; //seconds, ns3gym env step time interval
  uint32_t openGymPort = 5555;
  uint32_t testArg = 0;

  // change some default attributes so that they are reasonable for
  // this scenario, but do this before processing command line
  // arguments, so that the user is allowed to override these settings
  Config::SetDefault ("ns3::UdpClient::Interval", TimeValue (MilliSeconds (10)));
  Config::SetDefault ("ns3::UdpClient::MaxPackets", UintegerValue (1000000));
  Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (true));

  CommandLine cmd;
  // Command line arguments
  cmd.AddValue ("enbTxPowerDbm", "TX power [dBm] used by HeNBs (default = 46.0)", enbTxPowerDbm);


  // required parameters for OpenGym interface
  cmd.AddValue ("openGymPort", "Port number for OpenGym env. Default: 5555", openGymPort);
  cmd.AddValue ("simSeed", "Seed for random generator. Default: 1", simSeed);
  // optional parameters
  cmd.AddValue ("simTime", "Simulation time in seconds. Default: 10s", simulationTime);
  cmd.AddValue ("testArg", "Extra simulation argument. Default: 0", testArg);
  cmd.Parse (argc, argv);

  NS_LOG_UNCOND("Ns3Env parameters:");
  NS_LOG_UNCOND("--simulationTime: " << simulationTime);
  NS_LOG_UNCOND("--openGymPort: " << openGymPort);
  NS_LOG_UNCOND("--envStepTime: " << envStepTime);
  NS_LOG_UNCOND("--seed: " << simSeed);
  NS_LOG_UNCOND("--testArg: " << testArg);

  RngSeedManager::SetSeed (1);
  RngSeedManager::SetRun (simSeed);

  // OpenGym Env
  Ptr<OpenGymInterface> openGym = CreateObject<OpenGymInterface> (openGymPort);
  openGym->SetGetActionSpaceCb( MakeCallback (&MyGetActionSpace) );
  openGym->SetGetObservationSpaceCb( MakeCallback (&MyGetObservationSpace) );
  openGym->SetGetGameOverCb( MakeCallback (&MyGetGameOver) );
  openGym->SetGetObservationCb( MakeCallback (&MyGetObservation) );
  openGym->SetGetRewardCb( MakeCallback (&MyGetReward) );
  openGym->SetGetExtraInfoCb( MakeCallback (&MyGetExtraInfo) );
  openGym->SetExecuteActionsCb( MakeCallback (&MyExecuteActions) );
  Simulator::Schedule (Seconds(0.0), &ScheduleNextStateRead, envStepTime, openGym);

  NS_LOG_UNCOND ("Simulation start");

  // Optional logging:

  // LogLevel logLevel = (LogLevel)(LOG_PREFIX_ALL | LOG_LEVEL_ALL);

  // LogComponentEnable ("netHelper", logLevel);
  // LogComponentEnable ("EpcHelper", logLevel);
  // LogComponentEnable ("EpcEnbApplication", logLevel);
  // LogComponentEnable ("EpcX2", logLevel);
  // LogComponentEnable ("EpcSgwPgwApplication", logLevel);

  // LogComponentEnable ("LteEnbRrc", logLevel);
  // LogComponentEnable ("LteEnbNetDevice", logLevel);
  // LogComponentEnable ("LteUeRrc", logLevel);
  // LogComponentEnable ("LteUeNetDevice", logLevel);
  // LogComponentEnable ("A3RsrpHandoverAlgorithm", logLevel);


  //create nodes
  NodeContainer ueNodes;
  ueNodes.Create (numberOfUes);

  NodeContainer enbNodes;
  enbNodes.Create (numberOfEnbs);


  Ptr<LteHelper> netHelper = CreateObject<LteHelper> (); 
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  netHelper->SetEpcHelper (epcHelper);
  netHelper->SetSchedulerType ("ns3::RrFfMacScheduler");


   netHelper->SetHandoverAlgorithmType ("ns3::A3RsrpHandoverAlgorithm");
   netHelper->SetHandoverAlgorithmAttribute ("Hysteresis",
                                             DoubleValue (3.0));
   netHelper->SetHandoverAlgorithmAttribute ("TimeToTrigger",
                                             TimeValue (MilliSeconds (1024)));

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);


  // Routing of the Internet Host (towards the LTE network)
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  // Interface 0 is localhost, 1 is the p2p device
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);


  //set initial positions of eNBs
  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
  Vector enbPosition (675, 180, 0);
  enbPositionAlloc->Add (enbPosition);
  Vector enbPosition2 (800, 300, 0);
  enbPositionAlloc->Add (enbPosition2);


  // Install Mobility Model in eNBs
  MobilityHelper enbMobility;
  enbMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbMobility.SetPositionAllocator (enbPositionAlloc);
  enbMobility.Install (enbNodes);


//   Install Mobility Model in UE
  MobilityHelper ueMobility;
  ueMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  ueMobility.Install (ueNodes);



  // Install LTE Devices in eNB and UEs
  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (enbTxPowerDbm));
  NetDeviceContainer enbLteDevs = netHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = netHelper->InstallUeDevice (ueNodes);

  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIfaces;
  ueIpIfaces = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));

  // Attach all UEs to the first eNodeB
  for (uint16_t i = 0; i < numberOfUes; i++)
    {
      netHelper->Attach (ueLteDevs.Get (i), enbLteDevs.Get (0));
    }


  NS_LOG_LOGIC ("setting up applications");

  // Install and start applications on UEs and remote host
  uint16_t dlPort = 10000;
  uint16_t ulPort = 20000;

  // randomise start times to avoid simulation artefacts
  // (e.g., buffer overflows due to packet transmissions happening at exactly the same time)
  Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();
  startTimeSeconds->SetAttribute ("Min", DoubleValue (0));
  startTimeSeconds->SetAttribute ("Max", DoubleValue (0.010));

  //1 UE for current use case
  for (uint32_t u = 0; u < numberOfUes; ++u)
    {
      Ptr<Node> ue = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

      for (uint32_t b = 0; b < numBearersPerUe; ++b)
        {
          ++dlPort;
          ++ulPort;

          ApplicationContainer clientApps;
          ApplicationContainer serverApps;

          NS_LOG_LOGIC ("installing UDP DL app for UE " << u);
          UdpClientHelper dlClientHelper (ueIpIfaces.GetAddress (u), dlPort);
          clientApps.Add (dlClientHelper.Install (remoteHost));
          PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory",
                                               InetSocketAddress (Ipv4Address::GetAny (), dlPort));
          serverApps.Add (dlPacketSinkHelper.Install (ue));

          NS_LOG_LOGIC ("installing UDP UL app for UE " << u);
          UdpClientHelper ulClientHelper (remoteHostAddr, ulPort);
          clientApps.Add (ulClientHelper.Install (ue));
          PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory",
                                               InetSocketAddress (Ipv4Address::GetAny (), ulPort));
          serverApps.Add (ulPacketSinkHelper.Install (remoteHost));

          Ptr<EpcTft> tft = Create<EpcTft> ();
          EpcTft::PacketFilter dlpf;
          dlpf.localPortStart = dlPort;
          dlpf.localPortEnd = dlPort;
          tft->Add (dlpf);
          EpcTft::PacketFilter ulpf;
          ulpf.remotePortStart = ulPort;
          ulpf.remotePortEnd = ulPort;
          tft->Add (ulpf);
          EpsBearer bearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
          netHelper->ActivateDedicatedEpsBearer (ueLteDevs.Get (u), bearer, tft);

          Time startTime = Seconds (startTimeSeconds->GetValue ());
          serverApps.Start (startTime);
          clientApps.Start (startTime);

        } // end for b
    }


  // Add X2 interface
  netHelper->AddX2Interface (enbNodes);


  // Uncomment to enable PCAP tracing
  // p2ph.EnablePcapAll("handover_mobility");

  // netHelper->EnablePhyTraces ();
  // netHelper->EnableMacTraces ();
  // netHelper->EnableRlcTraces ();
  // netHelper->EnablePdcpTraces ();
  // Ptr<RadioBearerStatsCalculator> rlcStats = netHelper->GetRlcStats ();
  // rlcStats->SetAttribute ("EpochDuration", TimeValue (Seconds (1.0)));
  // Ptr<RadioBearerStatsCalculator> pdcpStats = netHelper->GetPdcpStats ();
  // pdcpStats->SetAttribute ("EpochDuration", TimeValue (Seconds (1.0)));



  //set up output trace file
  myfile.open ("/mnt/c/Users/shard/Documents/workspace/sim_outputs_4_cells/spaced/" + fileName + ".txt");
  myfile << "time" << "," << "x" << "," << "y" << ",";

  for (uint16_t i = 1; i <= numberOfEnbs; i++) {
    myfile << "RSRP_eNB_" << i << ",";
  }

  for (uint16_t i = 1; i <= numberOfEnbs; i++) {
    myfile << "RSRQ_eNB_" << i << ",";
  }

  myfile << "S-PCI" << "," << "\n";


  std::string enb_access = "/NodeList/*/DeviceList/*/LteEnbRrc/";
  std::string ue_access = "/NodeList/*/DeviceList/*/LteUeRrc/";

  // connect custom trace sinks for RRC connection establishment, handover notification and measurment reports
  Config::Connect (enb_access + "ConnectionEstablished",
                   MakeCallback (&NotifyConnectionEstablishedEnb));
  Config::Connect (ue_access + "ConnectionEstablished",
                   MakeCallback (&NotifyConnectionEstablishedUe));
  Config::Connect (enb_access + "HandoverStart",
                   MakeCallback (&NotifyHandoverStartEnb));
  Config::Connect (ue_access + "HandoverStart",
                   MakeCallback (&NotifyHandoverStartUe));
  Config::Connect (enb_access + "HandoverEndOk",
                   MakeCallback (&NotifyHandoverEndOkEnb));
  Config::Connect (ue_access + "HandoverEndOk",
                   MakeCallback (&NotifyHandoverEndOkUe));

  // custom trace sink for measurement reports with bound callback
  Config::Connect (enb_access + "RecvMeasurementReport", 
                  MakeBoundCallback (&ExtractMeasureMentReport, &ueNodes));


  

//Read pre-rocessed mobility trace
  
    std::ifstream file("/mnt/c/Users/shard/Documents/workspace/for_sim/" + fileName + ".txt");
    std::string str;
    char delimiter = ',';
    double time;
    int counter = 0;
    double* x = (double*) malloc(49500*sizeof(double));
    double* y = (double*) malloc(49500*sizeof(double)); 


    while (std::getline(file, str)) {
        std::vector<std::string> splits;                                                                                                                                                           
        std::string split;                                                                                                                                                                         
        std::istringstream ss(str);                                                                                                                                                                  
        while (std::getline(ss, split, delimiter))                                                                                                                                                 
        {                                                                                                                                                                                          
            splits.push_back(split);                                                                                                                                                                
        }    
        
        time = std::stod(splits[0]);
        // store x and y co-ordinates
        x[counter] = std::stod(splits[1]);
        y[counter] = std::stod(splits[2]);

        // schedule UE to be repositioned at given time
        Simulator::Schedule (Seconds (time), &setPos, x, y, counter);
        counter++;
    }

    Simulator::Schedule (Seconds (simulationTime), &freeMem, x, y);

  Simulator::Stop (Seconds (simulationTime));

  Simulator::Run ();
  NS_LOG_UNCOND ("Simulation stop");

  openGym->NotifySimulationEnd();
  Simulator::Destroy ();



 

  return 0;

}
