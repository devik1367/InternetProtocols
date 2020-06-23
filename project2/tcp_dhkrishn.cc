#include <string>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/packet-sink.h"

using namespace ns3;

float SD(double arr[]);

NS_LOG_COMPONENT_DEFINE("TCPCubicDCTCP");

int main (int argc, char *argv[])
{
	//int data_size = 500 * 1024 * 1024;
	InternetStackHelper stack;

	std::ofstream myfile;
	myfile.open("tcp_dhkrishn.csv");
	//populate header of csv file
	myfile << "exp,r1_s1,r2_s1,r3_s1,avg_s1,std_s1,unit_s1,r1_s2,r2_s2,r3_s2,avg_s2,std_s2,unit_s2\n";


	//NS_LOG_UNCOND("Creating nodes...");
	NodeContainer network;
	network.Create(6);
	NodeContainer s1r1 = NodeContainer(network.Get(0), network.Get(4));
	NodeContainer s2r1 = NodeContainer(network.Get(1), network.Get(4));
	NodeContainer r1r2 = NodeContainer(network.Get(4), network.Get(5));
	NodeContainer r2d1 = NodeContainer(network.Get(5), network.Get(2));
	NodeContainer r2d2 = NodeContainer(network.Get(5), network.Get(3));

	//add nodes to internet stack
	stack.Install(network);

	//connect the nodes using point-to-point links
	PointToPointHelper ptop;
	ptop.SetDeviceAttribute("DataRate", StringValue ("1Gbps"));
	NetDeviceContainer devices1r1 = ptop.Install(s1r1);
	NetDeviceContainer devices2r1 = ptop.Install(s2r1);
	NetDeviceContainer devicer1r2 = ptop.Install(r1r2);
	NetDeviceContainer devicer2d1 = ptop.Install(r2d1);
	NetDeviceContainer devicer2d2 = ptop.Install(r2d2);

	//set IP addresses
	//NS_LOG_UNCOND("Assigning IP addresses to each node...");
	Ipv4AddressHelper addr;
	//s1-r1
	addr.SetBase("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer ips1r1 = addr.Assign(devices1r1);
	//s2-r1
	addr.SetBase("10.1.2.0", "255.255.255.0");
	Ipv4InterfaceContainer ips2r1 = addr.Assign(devices2r1);
	//r1-r2
	addr.SetBase("10.1.3.0", "255.255.255.0");
	Ipv4InterfaceContainer ipr1r2 = addr.Assign(devicer1r2);
	//r2-d1
	addr.SetBase("10.1.4.0", "255.255.255.0");
	Ipv4InterfaceContainer ipr2d1 = addr.Assign(devicer2d1);
	//r2-d2
	addr.SetBase("10.1.5.0", "255.255.255.0");
	Ipv4InterfaceContainer ipr2d2 = addr.Assign(devicer2d2);

	//turn on global routing so routers can forward pkts
	Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	NS_LOG_UNCOND("Created network topology.");
	//std::cout << "party";


	//Exp-1: S1 sends traffic to D1 using TCP Cubic. S2 and D2 are not used in this experiment.
	uint16_t port = 9;
	//set TCP cubic as congestion control algorithm
	TypeId tid_cubic = TypeId::LookupByName("ns3::TcpBic");
	std::stringstream nodeIds1;
	nodeIds1 << s1r1.Get(0)->GetId();
	std::string specificNodes1 = "/NodeList/" + nodeIds1.str() + "/$ns3::TcpL4Protocol/SocketType";
	Config::Set (specificNodes1, TypeIdValue(tid_cubic));
	uint16_t startTime = 0.0;
	uint16_t stopTime = 10.0;
	for (int i=0; i<3; i++){
		//create BulkSend application on S1
		BulkSendHelper source1 ("ns3::TcpSocketFactory",InetSocketAddress(ipr2d1.GetAddress(1), port));
		//set amount of data to send in bytes
		source1.SetAttribute("MaxBytes", UintegerValue(500*1024*1024));
		ApplicationContainer source_s1 = source1.Install(network.Get(0));
		//create packet sink application on D1
		PacketSinkHelper sink1 ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
		ApplicationContainer sink_d1 = sink1.Install(network.Get(2));
		source_s1.Start(Seconds(startTime));
		sink_d1.Start(Seconds(startTime));
		source_s1.Stop(Seconds(stopTime));
		sink_d1.Stop(Seconds(stopTime));
		NS_LOG_UNCOND("Exp-1 successful.");
		startTime = stopTime;
		stopTime = startTime + 10.0;
	}


	//Exp-2: S1 sends traffic to D1 and S2 sends traffic to D2. Both senders will use TCP Cubic and start sending data to respective destinations simultaneously.
	std::stringstream nodeIds2;
	nodeIds2 << s2r1.Get(0)->GetId();
	std::string specificNodes2 = "/NodeList/" + nodeIds2.str() + "/$ns3::TcpL4Protocol/SocketType";
	Config::Set (specificNodes2, TypeIdValue(tid_cubic));
	for (int i=0; i<3; i++){
		BulkSendHelper source1 ("ns3::TcpSocketFactory",InetSocketAddress(ipr2d1.GetAddress(1), port));
		BulkSendHelper source2 ("ns3::TcpSocketFactory",InetSocketAddress(ipr2d2.GetAddress(1), port));
		//set amount of data to send in bytes
		source1.SetAttribute("MaxBytes", UintegerValue(500*1024*1024));
		source2.SetAttribute("MaxBytes", UintegerValue(500*1024*1024));
		ApplicationContainer source_s1 = source1.Install(network.Get(0));
		ApplicationContainer source_s2 = source2.Install(network.Get(1));
		//create packet sink application on D1
		PacketSinkHelper sink1 ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
		ApplicationContainer sink_d1 = sink1.Install(network.Get(2));
		source_s1.Start(Seconds(startTime));
		sink_d1.Start(Seconds(startTime));
		source_s1.Stop(Seconds(stopTime));
		sink_d1.Stop(Seconds(stopTime));
		PacketSinkHelper sink2 ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
		ApplicationContainer sink_d2 = sink2.Install(network.Get(3));
		source_s2.Start(Seconds(startTime));
		sink_d2.Start(Seconds(startTime));
		source_s2.Stop(Seconds(stopTime));
		sink_d2.Stop(Seconds(stopTime));
		NS_LOG_UNCOND("Exp-2 successful.");
		startTime = stopTime;
		stopTime = startTime + 10.0;
	}


	//Exp-3: S1 sends traffic to D1 using DCTCP. S2 and D2 are not used in this experiment.
	//set TCP DCTCP as congestion control algorithm
	TypeId tid_dctcp = TypeId::LookupByName("ns3::TcpDctcp");
	Config::Set (specificNodes1, TypeIdValue(tid_dctcp));
	//ns3TcpSocket1 = Socket::CreateSocket (s1r1.Get(0), TcpSocketFactory::GetTypeId());
	for(int i=0; i<3; i++){
		//create BulkSend application on S1
		BulkSendHelper source1 ("ns3::TcpSocketFactory",InetSocketAddress(ipr2d1.GetAddress(1), port));
		//set amount of data to send in bytes
		source1.SetAttribute("MaxBytes", UintegerValue(500*1024*1024));
		ApplicationContainer source_s1 = source1.Install(network.Get(0));
		//create packet sink application on D1
		PacketSinkHelper sink1 ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
		ApplicationContainer sink_d1 = sink1.Install(network.Get(2));
		source_s1.Start(Seconds(startTime));
		sink_d1.Start(Seconds(startTime));
		source_s1.Stop(Seconds(stopTime));
		sink_d1.Stop(Seconds(stopTime));
		NS_LOG_UNCOND("Exp-3 successful.");
		startTime = stopTime;
		stopTime = startTime + 10.0;
	}


	//Exp-4: S1 sends traffic to D1 and S2 sends traffic to D2. Both senders will use DCTCP and start sending data to respective destinations simultaneously.
	Config::Set (specificNodes2, TypeIdValue(tid_dctcp));
	for (int i=0; i<3; i++){
		BulkSendHelper source1 ("ns3::TcpSocketFactory",InetSocketAddress(ipr2d1.GetAddress(1), port));
		BulkSendHelper source2 ("ns3::TcpSocketFactory",InetSocketAddress(ipr2d2.GetAddress(1), port));
		//set amount of data to send in bytes
		source1.SetAttribute("MaxBytes", UintegerValue(500*1024*1024));
		source2.SetAttribute("MaxBytes", UintegerValue(500*1024*1024));
		ApplicationContainer source_s1 = source1.Install(network.Get(0));
		ApplicationContainer source_s2 = source2.Install(network.Get(1));
		//create packet sink application on D1
		PacketSinkHelper sink1 ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
		ApplicationContainer sink_d1 = sink1.Install(network.Get(2));
		source_s1.Start(Seconds(startTime));
		sink_d1.Start(Seconds(startTime));
		source_s1.Stop(Seconds(stopTime));
		sink_d1.Stop(Seconds(stopTime));
		PacketSinkHelper sink2 ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
		ApplicationContainer sink_d2 = sink2.Install(network.Get(3));
		source_s2.Start(Seconds(startTime));
		sink_d2.Start(Seconds(startTime));
		source_s2.Stop(Seconds(stopTime));
		sink_d2.Stop(Seconds(stopTime));
		NS_LOG_UNCOND("Exp-4 successful.");
		startTime = stopTime;
		stopTime = startTime + 10.0;
	}


	//Exp-5: S1 sends traffic to D1 using TCP Cubic whereas S2 sends traffic to D2 using DCTCP. Both senders will start sending data to respective destinations simultaneously.
	Config::Set (specificNodes1, TypeIdValue(tid_cubic));
	Config::Set (specificNodes2, TypeIdValue(tid_dctcp));
	for (int i=0; i<3; i++){
		BulkSendHelper source1 ("ns3::TcpSocketFactory",InetSocketAddress(ipr2d1.GetAddress(1), port));
		BulkSendHelper source2 ("ns3::TcpSocketFactory",InetSocketAddress(ipr2d2.GetAddress(1), port));
		//set amount of data to send in bytes
		source1.SetAttribute("MaxBytes", UintegerValue(500*1024*1024));
		source2.SetAttribute("MaxBytes", UintegerValue(500*1024*1024));
		ApplicationContainer source_s1 = source1.Install(network.Get(0));
		ApplicationContainer source_s2 = source2.Install(network.Get(1));
		//create packet sink application on D1
		PacketSinkHelper sink1 ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
		ApplicationContainer sink_d1 = sink1.Install(network.Get(2));
		source_s1.Start(Seconds(startTime));
		sink_d1.Start(Seconds(startTime));
		source_s1.Stop(Seconds(stopTime));
		sink_d1.Stop(Seconds(stopTime));
		PacketSinkHelper sink2 ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
		ApplicationContainer sink_d2 = sink2.Install(network.Get(3));
		source_s2.Start(Seconds(startTime));
		sink_d2.Start(Seconds(startTime));
		source_s2.Stop(Seconds(stopTime));
		sink_d2.Stop(Seconds(stopTime));
		NS_LOG_UNCOND("Exp-5 successful.");
		startTime = stopTime;
		stopTime = startTime + 10.0;
	}


	FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor = flowmon.InstallAll();

	Simulator::Stop(Seconds(stopTime));
	Simulator::Run ();

	//for S1
	float throughput = 0.0;
	float acft = 0.0;
	double e1th[3];
	double e2th[3];
	double e3th[3];
	double e4th[3];
	double e5th[3];
	double e1acft[3];
	double e2acft[3];
	double e3acft[3];
	double e4acft[3];
	double e5acft[3];
	//for s2
	double e2th_s2[3];
	double e2acft_s2[3];
	double e4th_s2[3];
	double e4acft_s2[3];
	double e5th_s2[3];
	double e5acft_s2[3];

	int e1counter = 0;
	int e2counter = 0;
	int e3counter = 0;
	int e4counter = 0;
	int e5counter = 0;
	int e2s2counter = 0;
	int e4s2counter = 0;
	int e5s2counter = 0;

  //monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
		  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
			//std::cout << "Flow #" << i->first << "\n";
			//S1 to D1
	    if (t.sourceAddress=="10.1.1.1")
	    {
				//NS_LOG_UNCOND("S1 ok");
				//Exp-2
				if (i->first == 7 || i->first == 11 || i->first == 15){
						acft = (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds());
	      	  throughput = i->second.rxBytes * 8.0 / acft/1024/1024;
						e2th[e2counter] = throughput;
						e2acft[e2counter] = acft;
						e2counter = e2counter + 1;
				}
				//Exp-3
				if (i->first == 19 || i->first == 21 || i->first == 23){
						acft = (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds());
	      	  throughput = i->second.rxBytes * 8.0 / acft/1024/1024;
						e3th[e3counter] = throughput;
						e3acft[e3counter] = acft;
						e3counter = e3counter + 1;
				}
				//Exp-4
				if (i->first == 25 || i->first == 29 || i->first == 33){
						acft = (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds());
	      	  throughput = i->second.rxBytes * 8.0 / acft/1024/1024;
						e4th[e4counter] = throughput;
						e4acft[e4counter] = acft;
						e4counter = e4counter + 1;
				}
				//Exp-5
				if (i->first == 37 || i->first == 41 || i->first == 45){
						acft = (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds());
	      	  throughput = i->second.rxBytes * 8.0 / acft/1024/1024;
						e5th[e5counter] = throughput;
						e5acft[e5counter] = acft;
						e5counter = e5counter + 1;
				}
				//Exp-1
				if (i->first == 1 || i->first == 3 || i->first == 5){
						acft = (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds());
	      	  throughput = i->second.rxBytes * 8.0 / acft/1024/1024;
						e1th[e1counter] = throughput;
						e1acft[e1counter] = acft;
						e1counter = e1counter + 1;
				}
	    }

			//S2 to D2
			if (t.sourceAddress=="10.1.2.1")
	    {
				//NS_LOG_UNCOND("S2 ok");
					//Exp-2
					if(i->first == 8 || i->first == 12 || i->first == 16)
					{
						acft = (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds());
	      	  throughput = i->second.rxBytes * 8.0 / acft/1024/1024;
						e2th_s2[e2s2counter] = throughput;
						e2acft_s2[e2s2counter] = acft;
						e2s2counter = e2s2counter + 1;
	    		}
					//Exp-4
					if(i->first == 8 || i->first == 12 || i->first == 16)
					{
						acft = (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds());
						throughput = i->second.rxBytes * 8.0 / acft/1024/1024;
						e4th_s2[e4s2counter] = throughput;
						e4acft_s2[e4s2counter] = acft;
						e4s2counter = e4s2counter + 1;
					}
					//Exp-5
					if(i->first == 8 || i->first == 12 || i->first == 16)
					{
						acft = (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds());
						throughput = i->second.rxBytes * 8.0 / acft/1024/1024;
						e5th_s2[e5s2counter] = throughput;
						e5acft_s2[e5s2counter] = acft;
						e5s2counter = e5s2counter + 1;
					}
    	}
		}

		//write to csv file
		//Throughput
		//exp-1
		myfile << "th_1," << e1th[0] << "," << e1th[1] << "," << e1th[2] << "," << (e1th[0]+e1th[1]+e1th[2])/3 << "," << SD(e1th) << "," << "Mbps,,,,,,\n";
		//exp-2
		myfile << "th_2," << e2th[0] << "," << e2th[1] << "," << e2th[2] << "," << (e2th[0]+e2th[1]+e2th[2])/3 << "," << SD(e2th) << "," << "Mbps,";
		myfile << e2th_s2[0] << "," << e2th_s2[1] << "," << e2th_s2[2] << "," << (e2th_s2[0]+e2th_s2[1]+e2th_s2[2])/3 << "," << SD(e2th_s2) << "," << "Mbps\n";
		//exp-3
		myfile << "th_3," << e3th[0] << "," << e3th[1] << "," << e3th[2] << "," << (e3th[0]+e3th[1]+e3th[2])/3 << "," << SD(e3th) << "," << "Mbps,,,,,,\n";
		//exp-4
		myfile << "th_4," << e4th[0] << "," << e4th[1] << "," << e4th[2] << "," << (e4th[0]+e4th[1]+e4th[2])/3 << "," << SD(e4th) << "," << "Mbps,";
		myfile << e4th_s2[0] << "," << e4th_s2[1] << "," << e4th_s2[2] << "," << (e4th_s2[0]+e4th_s2[1]+e4th_s2[2])/3 << "," << SD(e4th_s2) << "," << "Mbps\n";
		//exp-5
		myfile << "th_5," << e5th[0] << "," << e5th[1] << "," << e5th[2] << "," << (e5th[0]+e5th[1]+e5th[2])/3 << "," << SD(e5th) << "," << "Mbps,";
		myfile << e5th_s2[0] << "," << e5th_s2[1] << "," << e5th_s2[2] << "," << (e5th_s2[0]+e5th_s2[1]+e5th_s2[2])/3 << "," << SD(e5th_s2) << "," << "Mbps\n";

		//Average flow completion time
		//exp-1
		myfile << "acft_1," << e1acft[0] << "," << e1acft[1] << "," << e1acft[2] << "," << (e1acft[0]+e1acft[1]+e1acft[2])/3 << "," << SD(e1acft) << "," << "sec,,,,,,\n";
		//exp-2
		myfile << "acft_2," << e2acft[0] << "," << e2acft[1] << "," << e2acft[2] << "," << (e2acft[0]+e2acft[1]+e2acft[2])/3 << "," << SD(e2acft) << "," << "sec,";
		myfile << e2acft_s2[0] << "," << e2acft_s2[1] << "," << e2acft_s2[2] << "," << (e2acft_s2[0]+e2acft_s2[1]+e2acft_s2[2])/3 << "," << SD(e2acft_s2) << "," << "sec\n";
		//exp-3
		myfile << "acft_3," << e3acft[0] << "," << e3acft[1] << "," << e3acft[2] << "," << (e3acft[0]+e3acft[1]+e3acft[2])/3 << "," << SD(e3acft) << "," << "sec,,,,,,\n";
		//exp-4
		myfile << "acft_4," << e4acft[0] << "," << e4acft[1] << "," << e4acft[2] << "," << (e4acft[0]+e4acft[1]+e4acft[2])/3 << "," << SD(e4acft) << "," << "sec,";
		myfile << e4acft_s2[0] << "," << e4acft_s2[1] << "," << e4acft_s2[2] << "," << (e4acft_s2[0]+e4acft_s2[1]+e4acft_s2[2])/3 << "," << SD(e4acft_s2) << "," << "sec\n";
		//exp-5
		myfile << "acft_5," << e5acft[0] << "," << e5acft[1] << "," << e5acft[2] << "," << (e5acft[0]+e5acft[1]+e5acft[2])/3 << "," << SD(e5acft) << "," << "sec,";
		myfile << e5acft_s2[0] << "," << e5acft_s2[1] << "," << e5acft_s2[2] << "," << (e5acft_s2[0]+e5acft_s2[1]+e5acft_s2[2])/3 << "," << SD(e5acft_s2) << "," << "sec\n";


	myfile.close();
	Simulator::Destroy();
	return 0;
}
//end of main function


//calculate standard deviation
float SD(double arr[]){
	float sum = 0.0;
	float mean;
	float sd = 0.0;
	int i;
	for(i=0;i<3;++i){
		sum += arr[i];
	}
	mean = sum/3;
	for(i=0;i<3;++i){
		sd += pow(arr[i] - mean, 2);
	}
	return sqrt(sd/3);
}
