#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("AnimatedNetworkSlicing");

// Global variables for animation control
AnimationInterface* pAnim = nullptr;
double simTime = 15.0;

void LogInputFeatures(std::string sliceType,
                    double trafficVolume,
                    double packetArrivalRate,
                    double latencyRequirement,
                    double jitterRequirement,
                    double packetLossTolerance,
                    double cpuUtilization,
                    double memoryUtilization,
                    double bandwidthUtilization,
                    int numActiveUsers) {
    std::ofstream file;
    file.open("slice_input_metrics.csv", std::ios::app);

    static bool first = true;
    if (first) {
        file << "Traffic_Volume,Packet_Arrival_Rate,Latency_Requirement,"
             << "Jitter_Requirement,Packet_Loss_Tolerance,CPU_Utilization,"
             << "Memory_Utilization,Bandwidth_Utilization,Num_Active_Users,Slice_Type\n";
        first = false;
    }

    file << trafficVolume << "," << packetArrivalRate << "," << latencyRequirement << "," 
         << jitterRequirement << "," << packetLossTolerance << "," << cpuUtilization << "," 
         << memoryUtilization << "," << bandwidthUtilization << "," << numActiveUsers << "," 
         << sliceType << "\n";

    file.close();
}

void GenerateSliceParameters() {
    std::vector<std::string> slices = {"eMBB", "URLLC", "mMTC"};
    
    for (const auto& slice : slices) {
        double trafficVolume = 0.0;
        double packetArrivalRate = 0.0;
        double latencyRequirement = 0.0;
        double jitterRequirement = 0.0;
        double packetLossTolerance = 0.0;
        double cpuUtilization = 0.0;
        double memoryUtilization = 0.0;
        double bandwidthUtilization = 0.0;
        int numActiveUsers = 0;

	if (slice == "eMBB") {
	    trafficVolume = 300 + rand() % 700;                // 300-1000 MB (observed max: 970.21)
	    packetArrivalRate = 10 + rand() % 490;             // 10-500 packets/sec (observed max: 496.82)
	    latencyRequirement = 3 + rand() % 40;              // 3-43 ms (observed min: 3.19, max: 42.73)
	    jitterRequirement = 0.5 + rand() % 10;             // 0.5-10.5 ms (observed min: 0.36, max: 9.86)
	    packetLossTolerance = 0.04 + static_cast<double>(rand() % 500) / 100.0; // 0.04-5.04% (observed max: 4.98)
	    cpuUtilization = 10 + rand() % 80;                 // 10-90% (observed max: 89.50)
	    memoryUtilization = 10 + rand() % 80;              // 10-90 units (observed max: 88.22)
	    bandwidthUtilization = 10 + rand() % 80;           // 10-90% (observed max: 89.10)
	    numActiveUsers = 1 + rand() % 49;                  // 1-50 users (observed max: 49)
	}
	else if (slice == "URLLC") {
	    trafficVolume = 15 + rand() % 985;                 // 15-1000 MB (observed max: 990.15)
	    packetArrivalRate = 10 + rand() % 490;             // 10-500 packets/sec (observed max: 492.59)
	    latencyRequirement = 3 + rand() % 100;             // 3-103 ms (observed min: 3.29, max: 105.40)
	    jitterRequirement = 0.3 + rand() % 10;             // 0.3-10.3 ms (observed min: 0.32, max: 10.57)
	    packetLossTolerance = 0.01 + static_cast<double>(rand() % 500) / 100.0; // 0.01-5.01% (observed max: 5.39)
	    cpuUtilization = 10 + rand() % 80;                 // 10-90% (observed max: 89.54)
	    memoryUtilization = 10 + rand() % 80;              // 10-90 units (observed max: 89.18)
	    bandwidthUtilization = 10 + rand() % 80;           // 10-90% (observed max: 89.77)
	    numActiveUsers = 1 + rand() % 49;                  // 1-50 users (observed max: 49)
	}
	else if (slice == "mMTC") {
	    trafficVolume = 8 + rand() % 992;                  // 8-1000 MB (observed min: 8.86, max: 987.02)
	    packetArrivalRate = 10 + rand() % 490;             // 10-500 packets/sec (observed max: 492.24)
	    latencyRequirement = 5 + rand() % 100;              // 5-105 ms (observed min: 5.48, max: 99.62)
	    jitterRequirement = 0.5 + rand() % 10;             // 0.5-10.5 ms (observed min: 0.49, max: 11.72)
	    packetLossTolerance = 0.02 + static_cast<double>(rand() % 500) / 100.0; // 0.02-5.02% (observed max: 5.22)
	    cpuUtilization = 10 + rand() % 80;                 // 10-90% (observed max: 89.58)
	    memoryUtilization = 10 + rand() % 80;              // 10-90 units (observed max: 88.92)
	    bandwidthUtilization = 10 + rand() % 80;           // 10-90% (observed max: 89.10)
	    numActiveUsers = 1 + rand() % 49;                  // 1-50 users (observed max: 48)
	}
        LogInputFeatures(slice, trafficVolume, packetArrivalRate, latencyRequirement,
                       jitterRequirement, packetLossTolerance, cpuUtilization, memoryUtilization,
                       bandwidthUtilization, numActiveUsers);
    }
}

void UpdateResourceAllocation(int frame) {
    if (!pAnim) return;
    
    // Simulate dynamic resource allocation
    double embb_bw = 60 + 10 * sin(frame * 0.3);
    double urllc_bw = 20 + 5 * cos(frame * 0.2);
    double mtc_bw = 20 - 5 * sin(frame * 0.1);
    
    // Update node colors based on utilization
    pAnim->UpdateNodeColor(1, // Node ID for eMBB
                          std::min(255, (int)(embb_bw * 2.5)), 
                          std::min(255, 255 - (int)(embb_bw * 2.5)), 
                          0);
    pAnim->UpdateNodeColor(2, // Node ID for URLLC
                          std::min(255, (int)(urllc_bw * 5)), 
                          std::min(255, 255 - (int)(urllc_bw * 5)), 
                          0);
    pAnim->UpdateNodeColor(3, // Node ID for mMTC
                          std::min(255, (int)(mtc_bw * 5)), 
                          std::min(255, 255 - (int)(mtc_bw * 5)), 
                          0);
    
    // Update node sizes (using node IDs directly)
    pAnim->UpdateNodeSize(1, 15 + embb_bw/5, 15 + embb_bw/5);
    pAnim->UpdateNodeSize(2, 15 + urllc_bw/2, 15 + urllc_bw/2);
    pAnim->UpdateNodeSize(3, 15 + mtc_bw/2, 15 + mtc_bw/2);
    
    // Update descriptions
    char desc[100];
    sprintf(desc, "eMBB\nBW:%.1f%%", embb_bw);
    pAnim->UpdateNodeDescription(1, desc);
    sprintf(desc, "URLLC\nBW:%.1f%%", urllc_bw);
    pAnim->UpdateNodeDescription(2, desc);
    sprintf(desc, "mMTC\nBW:%.1f%%", mtc_bw);
    pAnim->UpdateNodeDescription(3, desc);
    
    // Schedule next update
    if (Simulator::Now().GetSeconds() < simTime - 1) {
        Simulator::Schedule(Seconds(0.5), &UpdateResourceAllocation, frame + 1);
    }
}

int main(int argc, char *argv[]) {
    GenerateSliceParameters();
    // Create nodes (gNB=0, eMBB=1, URLLC=2, mMTC=3)
    NodeContainer nodes;
    nodes.Create(4);

    // Setup mobility
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    // Create links with different QoS
    PointToPointHelper embbLink, urllcLink, mtcLink;
    embbLink.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    embbLink.SetChannelAttribute("Delay", StringValue("10ms"));
    urllcLink.SetDeviceAttribute("DataRate", StringValue("50Mbps"));
    urllcLink.SetChannelAttribute("Delay", StringValue("1ms"));
    mtcLink.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    mtcLink.SetChannelAttribute("Delay", StringValue("50ms"));

    NetDeviceContainer devices1 = embbLink.Install(nodes.Get(0), nodes.Get(1));
    NetDeviceContainer devices2 = urllcLink.Install(nodes.Get(0), nodes.Get(2));
    NetDeviceContainer devices3 = mtcLink.Install(nodes.Get(0), nodes.Get(3));

    // Install internet stack
    InternetStackHelper stack;
    stack.Install(nodes);

    // Assign IP addresses
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer if1 = address.Assign(devices1);
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer if2 = address.Assign(devices2);
    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer if3 = address.Assign(devices3);

    // Setup Animation
    AnimationInterface anim("network_slicing_animation.xml");
    pAnim = &anim;
    anim.EnablePacketMetadata(true);
    
    // Initial node positioning
    anim.SetConstantPosition(nodes.Get(0), 50, 50);  // gNB at center
    anim.SetConstantPosition(nodes.Get(1), 20, 20);  // eMBB
    anim.SetConstantPosition(nodes.Get(2), 80, 20);  // URLLC
    anim.SetConstantPosition(nodes.Get(3), 50, 80);  // mMTC
    
    // Initial colors and descriptions
    anim.UpdateNodeColor(0, 255, 165, 0);  // Orange gNB
    anim.UpdateNodeDescription(0, "gNB");
    anim.UpdateNodeDescription(1, "eMBB");
    anim.UpdateNodeDescription(2, "URLLC");
    anim.UpdateNodeDescription(3, "mMTC");

    // Create traffic
    uint16_t port = 5000;
    
    // eMBB traffic - large packets
    UdpEchoServerHelper server1(port);
    ApplicationContainer apps1 = server1.Install(nodes.Get(1));
    apps1.Start(Seconds(0.5));
    apps1.Stop(Seconds(simTime));

    UdpEchoClientHelper client1(if1.GetAddress(0), port);
    client1.SetAttribute("MaxPackets", UintegerValue(1000));
    client1.SetAttribute("Interval", TimeValue(Seconds(0.05)));
    client1.SetAttribute("PacketSize", UintegerValue(1472));
    apps1 = client1.Install(nodes.Get(0));
    apps1.Start(Seconds(1.0));
    apps1.Stop(Seconds(simTime-1));

    // URLLC traffic - small, frequent packets
    UdpEchoServerHelper server2(port+1);
    ApplicationContainer apps2 = server2.Install(nodes.Get(2));
    apps2.Start(Seconds(0.5));
    apps2.Stop(Seconds(simTime));

    UdpEchoClientHelper client2(if2.GetAddress(0), port+1);
    client2.SetAttribute("MaxPackets", UintegerValue(2000));
    client2.SetAttribute("Interval", TimeValue(Seconds(0.01)));
    client2.SetAttribute("PacketSize", UintegerValue(64));
    apps2 = client2.Install(nodes.Get(0));
    apps2.Start(Seconds(1.5));
    apps2.Stop(Seconds(simTime-1));

    // Start dynamic updates
    Simulator::Schedule(Seconds(1.0), &UpdateResourceAllocation, 0);

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
