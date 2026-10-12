#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <cmath>
#include <fstream>

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("A2ThreeGppVideo");

// ---- 3GPP video streaming model parameters (verify against the spec) ----
static const double FRAME_INTERVAL = 0.100; // s
static const uint32_t PKTS_PER_FRAME = 8;
static const double SIZE_K = 20, SIZE_ALPHA = 1.2, PKT_SIZE_MAX = 250;     // bytes
static const double IAT_K = 0.0025, IAT_ALPHA = 1.2, IAT_MAX = 0.0125; // seconds

static Ptr<Socket> g_sock;
static Ptr<UniformRandomVariable> g_u;
static std::ofstream g_txLog;
static double g_stop = 60.0;

// Inverse-CDF sampling of a truncated Pareto on [k, m]
static double
TruncPareto(double k, double alpha, double m)
{
    double u = g_u->GetValue(0.0, 1.0);
    double tail = 1.0 - std::pow(k / m, alpha);
    return k / std::pow(1.0 - u * tail, 1.0 / alpha);
}

static void
SendPacket(uint32_t size, double gap)
{
    g_sock->Send(Create<Packet>(size));
    // columns: time, size, gap (gap < 0 means first packet of a frame)
    g_txLog << Simulator::Now().GetSeconds() << "," << size << "," << gap << "\n";
}

static void
SendFrame()
{
    if (Simulator::Now().GetSeconds() >= g_stop)
    {
        return;
    }
    double offset = 0.0;
    for (uint32_t i = 0; i < PKTS_PER_FRAME; ++i)
    {
        double gap = -1.0;
        if (i > 0)
        {
            gap = TruncPareto(IAT_K, IAT_ALPHA, IAT_MAX);
            offset += gap;
        }
        uint32_t size = static_cast<uint32_t>(std::round(TruncPareto(SIZE_K, SIZE_ALPHA, PKT_SIZE_MAX)));
        Simulator::Schedule(Seconds(offset), &SendPacket, size, gap);
    }
    Simulator::Schedule(Seconds(FRAME_INTERVAL), &SendFrame);
}

int
main(int argc, char* argv[])
{
    double simTime = 60.0;
    CommandLine cmd(__FILE__);
    cmd.AddValue("simTime", "Simulation time (s)", simTime);
    cmd.Parse(argc, argv);
    g_stop = simTime;

    RngSeedManager::SetSeed(1);
    RngSeedManager::SetRun(1);
    g_u = CreateObject<UniformRandomVariable>();

    NodeContainer nodes;
    nodes.Create(2);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("10ms"));
    NetDeviceContainer devs = p2p.Install(nodes);

    InternetStackHelper stack;
    stack.Install(nodes);
    Ipv4AddressHelper addr;
    addr.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ifs = addr.Assign(devs);

    uint16_t port = 9;
    PacketSinkHelper sink("ns3::UdpSocketFactory",
                          InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApp = sink.Install(nodes.Get(1));
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(simTime + 2));

    g_sock = Socket::CreateSocket(nodes.Get(0), UdpSocketFactory::GetTypeId());
    g_sock->Connect(InetSocketAddress(ifs.GetAddress(1), port));

    g_txLog.open("a2_tx_log.csv");
    g_txLog << "time,size,gap\n";
    Simulator::Schedule(Seconds(1.0), &SendFrame);

    p2p.EnablePcapAll("a2-video"); // open the .pcap files in Wireshark

    FlowMonitorHelper fmh;
    Ptr<FlowMonitor> monitor = fmh.InstallAll();

    Simulator::Stop(Seconds(simTime + 2));
    Simulator::Run();

    monitor->CheckForLostPackets();
    for (auto& kv : monitor->GetFlowStats())
    {
        const FlowMonitor::FlowStats& s = kv.second;
        double dur = s.timeLastRxPacket.GetSeconds() - s.timeFirstTxPacket.GetSeconds();
        std::cout << "Flow " << kv.first << "\n"
                  << "  Tx packets : " << s.txPackets << "\n"
                  << "  Rx packets : " << s.rxPackets << "\n"
                  << "  Throughput : " << (s.rxBytes * 8.0 / dur / 1e3) << " kbps\n"
                  << "  Mean delay : " << (s.delaySum.GetSeconds() / s.rxPackets * 1e3) << " ms\n";
    }

    g_txLog.close();
    Simulator::Destroy();
    return 0;
}