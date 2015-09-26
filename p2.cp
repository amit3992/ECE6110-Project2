// Amit Kulkarni
// GT ID: 903038158
// akulkarni72
// CAD Project 2
#include <iostream>
#include <string>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"
#include "ns3/applications-module.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/uinteger.h"
#include "ns3/point-to-point-dumbbell.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/random-variable-stream.h"
#include "ns3/constant-position-mobility-model.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("P2");

int
main (int argc, char *argv[])
{

  Time::SetResolution (Time::NS);

  // Set default values of variables.
  uint nNodes = 11; // initial value of flows
  
  std::string animFile = "P2.xml" ; // Name of file for animation output
  std::string queueType = "DropTail"; // queue type
  std::string UDP1_RATE = "500kb/s";
  std::string UDP2_RATE = "500kb/s";
  std::string Router_BW = "1Mbps";
  std::string Router_Delay = "20ms";

  //DropTail parameters
  uint32_t maxBytes = 64000; // value attricuted to infinity
  uint32_t segSize = 512; // initial value of segment size
  uint32_t queueSize = 64000; // initial value of queue size
  uint32_t windowSize = 64000; // initial value of window size
  uint32_t packetSize = 1024;

  //Red parameters
  double minTh = 60; // Threshold to trigger probabalistic drops
  double maxTh = 180; // Threshold to trigger forced drops
  uint32_t queueLen = (480 * packetSize); // Number of bytes that can be enqueued

  
// To change the value during run-time  
  CommandLine cmd;
  cmd.AddValue ("nNodes","Number of flows", nNodes);
  cmd.AddValue ("segSize","Size of each segment", segSize);
  cmd.AddValue ("queueSize","Size of queue", queueSize);
  cmd.AddValue ("windowSize","Size of window", windowSize);
  cmd.AddValue ("queueType","Set type of queue", queueType);
  cmd.AddValue ("maxBytes","Set maximum bytes", maxBytes);
  cmd.AddValue ("packetSize","Set packet size", packetSize);
  cmd.AddValue ("UDP1_RATE","Set UDP Flow 1 Data Rate", UDP1_RATE);
  cmd.AddValue ("UDP2_RATE","Set UDP Flow 2 Data Rate", UDP2_RATE);
  cmd.AddValue ("Router_BW","Set bottle-neck link bandwidth", Router_BW);
  cmd.AddValue ("Router_Delay","Set bottle-neck link delay", Router_Delay);
  cmd.AddValue ("minTh","Set threshold to trigger probabalistic drops", minTh);
  cmd.AddValue ("maxTh","Set threshold to trigger forced drops", maxTh);
  cmd.AddValue ("queueLen","Set number of bytes that can be enqueued", queueLen);
  cmd.Parse (argc, argv);

  double elapsed = 10.0; 
  double recvUDP1;  
  double recvUDP2;  
  double recvTCP1;  
  double recvTCP2;  
  double opUDP1; 
  double opUDP2; 
  double opTCP1; 
  double opTCP2;

  
  // Select queue type
  std::string setQueue;
  if (queueType == "DropTail") {
    setQueue = "ns3::DropTailQueue";
  }
  else if (queueType == "RED") {
    setQueue = "ns3::RedQueue";
  } 
  else {
    NS_ABORT_MSG ("Invalid queue type: Use --queueType=RED or --queueType=DropTail");
  } 

  // Setting TCP characteristics.
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (false));
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpTahoe"));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(segSize));
  Config::SetDefault("ns3::TcpSocketBase::MaxWindowSize", UintegerValue(windowSize));
  Config::SetDefault("ns3::TcpSocketBase::WindowScaling", BooleanValue(false));

  // Setting DropTailQueue characteristics
  Config::SetDefault ("ns3::DropTailQueue::Mode", EnumValue(DropTailQueue::QUEUE_MODE_BYTES));
  Config::SetDefault ("ns3::DropTailQueue::MaxBytes", UintegerValue(maxBytes));

  //Setting RedQueue characteristics
  minTh *= packetSize; 
  maxTh *= packetSize;
  Config::SetDefault ("ns3::RedQueue::Mode", EnumValue (RedQueue::QUEUE_MODE_BYTES));
  Config::SetDefault ("ns3::RedQueue::MinTh", DoubleValue (minTh));
  Config::SetDefault ("ns3::RedQueue::MaxTh", DoubleValue (maxTh));
  Config::SetDefault ("ns3::RedQueue::QueueLimit", UintegerValue (maxBytes)); // --> check.


  NS_LOG_INFO ("Create nodes and links for the topology.");

 // if (nRouters == 3)
  //{
    // Creating nodes.
    NodeContainer nodes;
    nodes.Create(nNodes);

    NodeContainer r0r1 (nodes.Get(0), nodes.Get(1));
    NodeContainer r1r2 (nodes.Get(1), nodes.Get(2));
    NodeContainer r0a1 (nodes.Get(0), nodes.Get(3));
    NodeContainer r0a2 (nodes.Get(0), nodes.Get(4));
    NodeContainer r0a3 (nodes.Get(0), nodes.Get(5));
    NodeContainer r0a4 (nodes.Get(0), nodes.Get(6));
    NodeContainer r1b1 (nodes.Get(1), nodes.Get(7));
    NodeContainer r1b2 (nodes.Get(1), nodes.Get(8));
    NodeContainer r2b3 (nodes.Get(2), nodes.Get(9));
    NodeContainer r2b4 (nodes.Get(2), nodes.Get(10));

    // To set positions of each node in netAnim.
    Ptr<Node> node0 = nodes.Get(0);
    Ptr<ConstantPositionMobilityModel> pos0 = node0->GetObject<ConstantPositionMobilityModel>();
    pos0 = CreateObject<ConstantPositionMobilityModel>();
    node0->AggregateObject(pos0);
    Vector pos0V(0, 2, 0);
    pos0->SetPosition(pos0V);

    Ptr<Node> node1 = nodes.Get(1);
    Ptr<ConstantPositionMobilityModel> pos1 = node1->GetObject<ConstantPositionMobilityModel>();
    pos1 = CreateObject<ConstantPositionMobilityModel>();
    node1->AggregateObject(pos1);
    Vector pos1V(1, 2, 0);
    pos1->SetPosition(pos1V);

    Ptr<Node> node2 = nodes.Get(2);
    Ptr<ConstantPositionMobilityModel> pos2 = node2->GetObject<ConstantPositionMobilityModel>();
    pos2 = CreateObject<ConstantPositionMobilityModel>();
    node2->AggregateObject(pos2);
    Vector pos2V(2, 2, 0);
    pos2->SetPosition(pos2V);

    Ptr<Node> node3 = nodes.Get(3);
    Ptr<ConstantPositionMobilityModel> pos3 = node3->GetObject<ConstantPositionMobilityModel>();
    pos3 = CreateObject<ConstantPositionMobilityModel>();
    node3->AggregateObject(pos3);
    Vector pos3V(-2, 3, 0);
    pos3->SetPosition(pos3V);

    Ptr<Node> node4 = nodes.Get(4);
    Ptr<ConstantPositionMobilityModel> pos4 = node4->GetObject<ConstantPositionMobilityModel>();
    pos4 = CreateObject<ConstantPositionMobilityModel>();
    node4->AggregateObject(pos4);
    Vector pos4V(-2, 2, 0);
    pos4->SetPosition(pos4V);

    Ptr<Node> node5 = nodes.Get(5);
    Ptr<ConstantPositionMobilityModel> pos5 = node5->GetObject<ConstantPositionMobilityModel>();
    pos5 = CreateObject<ConstantPositionMobilityModel>();
    node5->AggregateObject(pos5);
    Vector pos5V(-2, 1, 0);
    pos5->SetPosition(pos5V);

    Ptr<Node> node6 = nodes.Get(6);
    Ptr<ConstantPositionMobilityModel> pos6 = node6->GetObject<ConstantPositionMobilityModel>();
    pos6 = CreateObject<ConstantPositionMobilityModel>();
    node6->AggregateObject(pos6);
    Vector pos6V(-2, 0, 0);
    pos6->SetPosition(pos6V);

    Ptr<Node> node7 = nodes.Get(7);
    Ptr<ConstantPositionMobilityModel> pos7 = node7->GetObject<ConstantPositionMobilityModel>();
    pos7 = CreateObject<ConstantPositionMobilityModel>();
    node7->AggregateObject(pos7);
    Vector pos7V(3, 3, 0);
    pos7->SetPosition(pos7V);

    Ptr<Node> node8 = nodes.Get(8);
    Ptr<ConstantPositionMobilityModel> pos8 = node8->GetObject<ConstantPositionMobilityModel>();
    pos8 = CreateObject<ConstantPositionMobilityModel>();
    node8->AggregateObject(pos8);
    Vector pos8V(3, 1, 0);
    pos8->SetPosition(pos8V);

    Ptr<Node> node9 = nodes.Get(9);
    Ptr<ConstantPositionMobilityModel> pos9 = node9->GetObject<ConstantPositionMobilityModel>();
    pos9 = CreateObject<ConstantPositionMobilityModel>();
    node9->AggregateObject(pos9);
    Vector pos9V(5, 3, 0);
    pos9->SetPosition(pos9V);

    Ptr<Node> node10 = nodes.Get(10);
    Ptr<ConstantPositionMobilityModel> pos10 = node10->GetObject<ConstantPositionMobilityModel>();
    pos10 = CreateObject<ConstantPositionMobilityModel>();
    node10->AggregateObject(pos10);
    Vector pos10V(5, 1, 0);
    pos10->SetPosition(pos10V);


    //Creating links
    // Set parameters for links connecting routers r0 and r1
    PointToPointHelper RouterLink;
    RouterLink.SetDeviceAttribute ("DataRate", StringValue (Router_BW)); // --> Vary
    RouterLink.SetChannelAttribute ("Delay", StringValue (Router_Delay)); // --> Vary
    RouterLink.SetQueue (setQueue);
    NetDeviceContainer d_r0r1 = RouterLink.Install(r0r1);
    NetDeviceContainer d_r1r2 = RouterLink.Install(r1r2);

    // Set parameters for links connecting the left leaf
    PointToPointHelper LeftLink;
    LeftLink.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    LeftLink.SetChannelAttribute ("Delay", StringValue ("2ms"));
    NetDeviceContainer d_r0a1 = LeftLink.Install(r0a1);
    NetDeviceContainer d_r0a2 = LeftLink.Install(r0a2);
    NetDeviceContainer d_r0a3 = LeftLink.Install(r0a3);
    NetDeviceContainer d_r0a4 = LeftLink.Install(r0a4);

    // Set parameters for links connecting the right leaf
    PointToPointHelper RightLink;
    RightLink.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    RightLink.SetChannelAttribute ("Delay", StringValue ("2ms"));
    NetDeviceContainer d_r1b1 = RightLink.Install(r1b1);
    NetDeviceContainer d_r1b2 = RightLink.Install(r1b2);
    NetDeviceContainer d_r2b3 = RightLink.Install(r2b3);
    NetDeviceContainer d_r2b4 = RightLink.Install(r2b4);

    InternetStackHelper stack; // Install internet stack
    stack.Install (nodes);

    // Assigning addresses to all devices.
    Ipv4AddressHelper address;
 
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interface1 = address.Assign (d_r0r1);

    address.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interface2 = address.Assign (d_r1r2);

    address.SetBase ("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer interface3 = address.Assign (d_r0a1);

    address.SetBase ("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer interface4 = address.Assign (d_r0a2);

    address.SetBase ("10.1.5.0", "255.255.255.0");
    Ipv4InterfaceContainer interface5 = address.Assign (d_r0a3);

    address.SetBase ("10.1.6.0", "255.255.255.0");
    Ipv4InterfaceContainer interface6 = address.Assign (d_r0a4);

    address.SetBase ("10.1.7.0", "255.255.255.0");
    Ipv4InterfaceContainer interface7 = address.Assign (d_r1b1);

    address.SetBase ("10.1.8.0", "255.255.255.0");
    Ipv4InterfaceContainer interface8 = address.Assign (d_r1b2);

    address.SetBase ("10.1.9.0", "255.255.255.0");
    Ipv4InterfaceContainer interface9 = address.Assign (d_r2b3);

    address.SetBase ("10.1.10.0", "255.255.255.0");
    Ipv4InterfaceContainer interface10 = address.Assign (d_r2b4);


    // Install TCP and UDP applications to get both flows on the link.

    uint16_t port = 9;
    // On Off UDP application

    OnOffHelper Udp1("ns3::UdpSocketFactory", Address(InetSocketAddress(interface7.GetAddress(1), port)));
    Udp1.SetConstantRate(DataRate(UDP1_RATE)); // vary this value
    Udp1.SetAttribute ("PacketSize", UintegerValue (packetSize)); // vary for UDP
    Udp1.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=0.5]")); // 50% duty cycle
    Udp1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.5]"));

    OnOffHelper Udp2("ns3::UdpSocketFactory", Address(InetSocketAddress(interface9.GetAddress(1), port)));
    Udp2.SetConstantRate(DataRate(UDP2_RATE)); // vary this value
    Udp2.SetAttribute ("PacketSize", UintegerValue (packetSize)); // vary for UDP
    Udp2.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=0.5]")); // 50% duty cycle
    Udp2.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.5]"));

    BulkSendHelper Tcp1 ("ns3::TcpSocketFactory",InetSocketAddress (interface8.GetAddress(1), port));
    Tcp1.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
    Tcp1.SetAttribute ("SendSize", UintegerValue (packetSize));

    BulkSendHelper Tcp2 ("ns3::TcpSocketFactory",InetSocketAddress (interface10.GetAddress(1), port));
    Tcp1.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
    Tcp2.SetAttribute ("SendSize", UintegerValue (packetSize));    

    ApplicationContainer sourceApp1 = Udp1.Install (nodes.Get (3));
    ApplicationContainer sourceApp2 = Udp2.Install (nodes.Get (5));
    ApplicationContainer sourceApp3 = Tcp1.Install (nodes.Get (4));
    ApplicationContainer sourceApp4 = Tcp2.Install (nodes.Get (6));

    sourceApp1.Start (Seconds (0.0));
    sourceApp1.Stop (Seconds (10.0));

    sourceApp2.Start (Seconds (0.0));
    sourceApp2.Stop (Seconds (10.0));

    sourceApp3.Start (Seconds (0.0));
    sourceApp3.Stop (Seconds (10.0));

    sourceApp4.Start (Seconds (0.0));
    sourceApp4.Stop (Seconds (10.0));


    // Creating UDP and TCP sink apps
    // UDP sink 1
    PacketSinkHelper UdpSink1 ("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));

    // UDP sink 2
    PacketSinkHelper UdpSink2 ("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));

    // TCP sink 1
    PacketSinkHelper TcpSink1 ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    
    // TCP sink 2
    PacketSinkHelper TcpSink2 ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));

    ApplicationContainer sinkApp1;
    sinkApp1.Add (UdpSink1.Install (nodes.Get (7)));

    ApplicationContainer sinkApp2;
    sinkApp2.Add (UdpSink2.Install (nodes.Get (9)));

    ApplicationContainer sinkApp3;
    sinkApp3.Add (TcpSink1.Install (nodes.Get (8)));

    ApplicationContainer sinkApp4;
    sinkApp4.Add (TcpSink2.Install (nodes.Get (10)));

    sinkApp1.Start (Seconds (0.0));
    sinkApp1.Stop (Seconds (10.0));

    sinkApp2.Start (Seconds (0.0));
    sinkApp2.Stop (Seconds (10.0));

    sinkApp3.Start (Seconds (0.0));
    sinkApp3.Stop (Seconds (10.0));

    sinkApp4.Start (Seconds (0.0));
    sinkApp4.Stop (Seconds (10.0));
    
    // Set the bounding box for animation
    //db.BoundingBox (1, 1, 100, 100);

    // Create the animation object and configure for specified output
    AnimationInterface anim (animFile);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    Simulator::Stop (Seconds (10.0));
    Simulator::Run ();
    Simulator::Destroy ();

    cout<<"\nSimulation parameters: "<<endl;
    cout<<"Nodes: " <<nNodes<<endl;
    cout<<"queueType "<<queueType<<endl;
    cout<<"Bandwidth of bottle-neck link: "<<Router_BW<<endl;
    cout<<"Delay of bottle-neck link: "<<Router_Delay<<endl;
    cout<<"Data Rate of UDP flow 1: "<<UDP1_RATE<<endl;
    cout<<"Data Rate of UDP flow 2: "<<UDP2_RATE<<endl;
    cout<<" "<<endl;

    if (queueType == "DropTail") 
    {
      cout<<"\nDropTail Queue Parameters"<<endl;
      cout<<"SegSize: " <<segSize<<endl;
      cout<<"queueSize: "<<queueSize<<endl;
      cout<<"windowSize: "<<windowSize<<endl;
      cout<<"PacketSize: "<<packetSize<<endl;

    } 
    
    else if (queueType == "RED")
    {
      cout<<"\nRED Queue Parameters"<<endl;
      cout<<"Threshold to trigger probablistic drops: " <<minTh<<endl;
      cout<<"Threshold to trigger forced drops: "<<maxTh<<endl;
      cout<<"PacketSize: "<<packetSize<<endl;
    }
  
    cout <<"\nGoodput of each flow: " <<endl; 
    Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkApp1.Get(0));
    recvUDP1 = sink1->GetTotalRx ();
    opUDP1 = recvUDP1/elapsed;
    cout << "Goodput of UDP flow 1: " << opUDP1 <<endl;

    Ptr<PacketSink> sink2 = DynamicCast<PacketSink> (sinkApp2.Get(0));
    recvUDP2 = sink2->GetTotalRx ();
    opUDP2 = recvUDP2/elapsed;
    cout << "Goodput of UDP flow 2: " << opUDP2 <<endl;

    Ptr<PacketSink> sink3 = DynamicCast<PacketSink> (sinkApp3.Get(0));
    recvTCP1 = sink3->GetTotalRx ();
    opTCP1 = recvTCP1/elapsed;
    cout << "Goodput of TCP flow 1: " << opTCP1 <<endl;

    Ptr<PacketSink> sink4 = DynamicCast<PacketSink> (sinkApp4.Get(0));
    recvTCP2 = sink4->GetTotalRx ();
    opTCP2 = recvTCP2/elapsed;
    cout << "Goodput of TCP flow 2: " << opTCP2 <<endl;


return 0;

}
