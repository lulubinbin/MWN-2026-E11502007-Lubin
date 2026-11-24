/*
 * Reproduces the scenario of Figure 9 from Mangold et al.,
 * "Analysis of IEEE 802.11e for QoS Support in Wireless LANs".
 *
 * Topology: N contending STAs (1..16) -> 1 AP (HC), where each STA
 * transmits 4 streams (AC_VO, AC_VI, AC_BE, AC_BK) at 250 kb/s each
 * (1 Mb/s total offered traffic per STA; 4*N streams total).
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Fig9EdcaThroughput");

namespace
{
std::string
FormatRate(double bps)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0) << bps << "bps";
    return oss.str();
}

void
ConfigureAc(Ptr<QosTxop> txop, uint8_t aifsn, uint32_t cwMin, uint32_t cwMax, Time txopLimit)
{
    NS_ABORT_MSG_IF(!txop, "Null QosTxop pointer");
    txop->SetAifsn(aifsn);
    txop->SetMinCw(cwMin);
    txop->SetMaxCw(cwMax);
    txop->SetTxopLimit(txopLimit);
}
} // namespace

int
main(int argc, char* argv[])
{
    uint32_t nStas = 4;               // Number of contending stations (1..16)
    double perStreamKbps = 250.0;     // 250 kb/s per AC per STA (1 Mb/s per STA total)
    uint32_t packetSize = 512;        // 512-byte MSDUs per paper evaluation
    double simTime = 10.0;            // Simulation duration in seconds
    double warmupTime = 1.0;          // Warmup before measuring throughput
    uint32_t run = 1;                 // RNG run number

    CommandLine cmd(__FILE__);
    cmd.AddValue("nStas", "Number of contending transmitting stations (1..16)", nStas);
    cmd.AddValue("perStreamKbps", "Offered traffic per AC stream per station in kb/s", perStreamKbps);
    cmd.AddValue("packetSize", "MSDU payload size in bytes", packetSize);
    cmd.AddValue("simTime", "Total simulation time in seconds", simTime);
    cmd.AddValue("warmupTime", "Warmup time in seconds before stats reset", warmupTime);
    cmd.AddValue("run", "RNG run index (RngSeedManager::SetRun)", run);
    cmd.Parse(argc, argv);

    NS_ABORT_MSG_IF(nStas == 0, "nStas must be at least 1");
    NS_ABORT_MSG_IF(simTime <= warmupTime, "simTime must be greater than warmupTime");

    RngSeedManager::SetSeed(1);
    RngSeedManager::SetRun(run);

    NodeContainer apNode;
    apNode.Create(1);
    NodeContainer staNodes;
    staNodes.Create(nStas);

    // Arrange stations in a small circle around the AP so there are no hidden stations
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> posAlloc = CreateObject<ListPositionAllocator>();
    posAlloc->Add(Vector(0.0, 0.0, 0.0)); // AP at center
    const double radius = 5.0;
    for (uint32_t i = 0; i < nStas; ++i)
    {
        double angle = (2.0 * M_PI * static_cast<double>(i)) / static_cast<double>(nStas);
        posAlloc->Add(Vector(radius * std::cos(angle), radius * std::sin(angle), 0.0));
    }
    mobility.SetPositionAllocator(posAlloc);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(apNode);
    mobility.Install(staNodes);

    // 802.11a at 5 GHz, all frames transmitted at 24 Mb/s, RTS/CTS disabled
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211a);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                   "DataMode",
                                   StringValue("OfdmRate24Mbps"),
                                   "ControlMode",
                                   StringValue("OfdmRate24Mbps"),
                                   "RtsCtsThreshold",
                                   UintegerValue(65535));

    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper wifiPhy;
    wifiPhy.SetChannel(wifiChannel.Create());

    WifiMacHelper wifiMac;
    Ssid ssid = Ssid("80211e-fig9");

    wifiMac.SetType("ns3::StaWifiMac",
                    "Ssid",
                    SsidValue(ssid),
                    "QosSupported",
                    BooleanValue(true));
    NetDeviceContainer staDevices = wifi.Install(wifiPhy, wifiMac, staNodes);

    wifiMac.SetType("ns3::ApWifiMac",
                    "Ssid",
                    SsidValue(ssid),
                    "QosSupported",
                    BooleanValue(true));
    NetDeviceContainer apDevice = wifi.Install(wifiPhy, wifiMac, apNode);

    // Apply Table 1 (throughput evaluation) EDCA parameters on AP and STAs
    NetDeviceContainer allDevices;
    allDevices.Add(apDevice);
    allDevices.Add(staDevices);

    for (uint32_t i = 0; i < allDevices.GetN(); ++i)
    {
        Ptr<WifiNetDevice> wifiDev = DynamicCast<WifiNetDevice>(allDevices.Get(i));
        Ptr<WifiMac> mac = wifiDev->GetMac();
        NS_ABORT_MSG_IF(!mac, "Null WifiMac pointer");

        ConfigureAc(mac->GetQosTxop(AC_VO), 2, 3, 7, MicroSeconds(0));
        ConfigureAc(mac->GetQosTxop(AC_VI), 2, 7, 15, MicroSeconds(0));
        ConfigureAc(mac->GetQosTxop(AC_BE), 3, 15, 1023, MicroSeconds(0));
        ConfigureAc(mac->GetQosTxop(AC_BK), 7, 15, 1023, MicroSeconds(0));
    }

    InternetStackHelper stack;
    stack.Install(apNode);
    stack.Install(staNodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer apInterface = address.Assign(apDevice);
    Ipv4InterfaceContainer staInterfaces = address.Assign(staDevices);

    struct AcSpec
    {
        std::string name;
        uint16_t basePort;
        uint8_t tos;
    };

    const std::vector<AcSpec> acSpecs = {
        {"AC_VO", 5000, 0xc0}, // Voice (highest priority)
        {"AC_VI", 5100, 0x80}, // Video
        {"AC_BE", 5200, 0x00}, // Best effort
        {"AC_BK", 5300, 0x20}, // Background (lowest priority)
    };

    const double perStreamBps = perStreamKbps * 1e3;
    const std::string rateStr = FormatRate(perStreamBps);

    std::map<uint16_t, std::string> portToAcName;
    ApplicationContainer sinkApps;
    ApplicationContainer sourceApps;

    // Each STA transmits 4 uplink streams (one per AC) to the AP
    for (const auto& ac : acSpecs)
    {
        for (uint32_t s = 0; s < nStas; ++s)
        {
            uint16_t port = static_cast<uint16_t>(ac.basePort + s);
            portToAcName[port] = ac.name;

            InetSocketAddress sinkAddr(Ipv4Address::GetAny(), port);
            PacketSinkHelper sinkHelper("ns3::UdpSocketFactory", sinkAddr);
            sinkApps.Add(sinkHelper.Install(apNode.Get(0)));

            if (perStreamBps > 0.0)
            {
                Ptr<ExponentialRandomVariable> expOn = CreateObject<ExponentialRandomVariable>();
                expOn->SetAttribute("Mean", DoubleValue(0.05));
                Ptr<ConstantRandomVariable> zeroOff = CreateObject<ConstantRandomVariable>();
                zeroOff->SetAttribute("Constant", DoubleValue(0.0));

                OnOffHelper onOff("ns3::UdpSocketFactory",
                                  InetSocketAddress(apInterface.GetAddress(0), port));
                onOff.SetAttribute("PacketSize", UintegerValue(packetSize));
                onOff.SetAttribute("DataRate", StringValue(rateStr));
                onOff.SetAttribute("OnTime", PointerValue(expOn));
                onOff.SetAttribute("OffTime", PointerValue(zeroOff));
                onOff.SetAttribute("Tos", UintegerValue(ac.tos));

                ApplicationContainer app = onOff.Install(staNodes.Get(s));
                app.Start(Seconds(0.1));
                app.Stop(Seconds(simTime));
                sourceApps.Add(app);
            }
        }
    }

    sinkApps.Start(Seconds(0.0));
    sinkApps.Stop(Seconds(simTime));

    FlowMonitorHelper flowmonHelper;
    Ptr<FlowMonitor> monitor = flowmonHelper.InstallAll();
    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowmonHelper.GetClassifier());

    Simulator::Schedule(Seconds(warmupTime), &FlowMonitor::ResetAllStats, monitor);

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

    monitor->CheckForLostPackets();
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();

    std::map<std::string, double> rxBytesPerAc;
    for (const auto& ac : acSpecs)
    {
        rxBytesPerAc[ac.name] = 0.0;
    }

    for (const auto& kv : stats)
    {
        Ipv4FlowClassifier::FiveTuple tuple = classifier->FindFlow(kv.first);
        if (tuple.destinationAddress == apInterface.GetAddress(0))
        {
            auto it = portToAcName.find(tuple.destinationPort);
            if (it != portToAcName.end())
            {
                rxBytesPerAc[it->second] += static_cast<double>(kv.second.rxBytes);
            }
        }
    }

    const double measureDuration = simTime - warmupTime;
    const double totalOfferedPerAcMbps = (nStas * perStreamKbps) / 1000.0;
    std::cout << "Stations: " << nStas << " (offered per AC: " << totalOfferedPerAcMbps
              << " Mb/s)\n";
    for (const auto& ac : acSpecs)
    {
        double throughputMbps = (rxBytesPerAc[ac.name] * 8.0) / (measureDuration * 1e6);
        std::cout << "  " << ac.name << " throughput: " << std::fixed << std::setprecision(3)
                  << throughputMbps << " Mb/s\n";
    }

    Simulator::Destroy();
    return 0;
}
