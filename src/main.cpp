/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */



#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/command-line.h"
#include "ClientApplication.h"
#include "GwApplication.h"
#include "CachingControllerApplication.h"
#include "VideoDataSource.h"
#include <ns3/config.h>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

//
// CLIENT --- GW---- \
// CLIENT --- GW---- \
// CLIENT --- GW-----\
// CLIENT --- GW --- ROUTER --- POP
//                     |
//                     |
//                    CP
//
//





using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("SVNF");

void
DataRateTrace(uint32_t oldValue, uint32_t newValue) {
    std::cout << "Traced " << oldValue << " to " << newValue << std::endl;
}

class PlotData{
public:
    uint32_t totalSizeTransmitted=0;


};
std::map<std::string,std::map<int, PlotData> > cpDr;

void PacketTraceCallBack(std::string context, const Ptr<const Packet> packet) {

    int second = (int) std::round(Simulator::Now().GetSeconds());
    if (!   cpDr[context].count(second)) {
        cpDr[context][second] = PlotData();
    }
    cpDr[context][second].totalSizeTransmitted+=packet->GetSize();
}

class ClientStat {

public:
    Time startDate;
    Time stopDate;
    uint32_t totalSize = 0;
    uint32_t dataSourceId;


};

std::map<int, ClientStat> clientStats;

void PacketAddressTrace(std::string context, const Ptr<const Packet> packet, const Address &address) {

    int s = boost::lexical_cast<int>(context.substr(10, context.substr(10, context.length() - 10).find("/")));
    clientStats[s].stopDate = Simulator::Now();
    clientStats[s].totalSize += packet->GetSize();


}

int
main(int argc, char *argv[]) {
    bool verbose = true;
    uint32_t nGW = 0;
    double mat=1;
    Time END = Seconds(1000);


    const std::string popDataRate("100kbps");
    const std::string cpDataRate("100kbps");

    CommandLine cmd;
    cmd.AddValue("nGW", "Number of \"extra\" CSMA nodes/devices", nGW);
    cmd.AddValue("mat", "Mean arrival time for clients", mat);

    cmd.Parse(argc, argv);



    //Config::SetDefault("ns3::TcpSocket::SndBufSize", ns3::UintegerValue(100000));
    //Config::SetDefault("ns3::TcpSocket::RcvBufSize", ns3::UintegerValue(100000));
    //Config::SetDefault("ns3::TcpSocket::SegmentSize", ns3::UintegerValue(1460));


    //LAN
    PointToPointHelper pointToPointLan;
    pointToPointLan.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
    pointToPointLan.SetChannelAttribute("Delay", StringValue("0.2ms"));

    //CSMA
    CsmaHelper csmaHelper;
    csmaHelper.SetChannelAttribute("DataRate", StringValue("1Gbps"));
    csmaHelper.SetChannelAttribute("Delay", StringValue("1ns"));

    //POP
    PointToPointHelper pointToPointPOP;
    pointToPointPOP.SetDeviceAttribute("DataRate", DataRateValue(DataRate(popDataRate)));
    pointToPointPOP.SetChannelAttribute("Delay", StringValue("1ns"));

    //CP
    PointToPointHelper pointToPointCP;
    pointToPointCP.SetDeviceAttribute("DataRate", DataRateValue(DataRate(cpDataRate)));
    pointToPointCP.SetChannelAttribute("Delay", StringValue("1ns"));


    Ptr<ExponentialRandomVariable> ev = CreateObject<ExponentialRandomVariable>();
    ev->SetAttribute("Mean", DoubleValue(mat));
    ev->SetAttribute("Bound", DoubleValue(100.));






    LogComponentEnable("TcpSocketBase", LOG_LEVEL_ALL);
    //LogComponentEnable("SVNF", LOG_LEVEL_ALL);
    LogComponentEnable("GwApplication", LOG_LEVEL_ALL);
    //LogComponentEnable("ClientApplication", LOG_LEVEL_ALL);
    //LogComponentEnable("CachingControllerApplication", LOG_LEVEL_ALL);
    //LogComponentEnable("NscTcpSocketImpl", LOG_LEVEL_ALL);
    LogComponentEnable("PacketSink", LOG_LEVEL_ALL);
    //LogComponentEnable("VideoDataSource", LOG_LEVEL_ALL);
    //LogComponentEnable("TcpTxBuffer", LOG_LEVEL_ALL);
    LogComponentEnable("TcpNewReno", LOG_LEVEL_ALL);





    NodeContainer gwNodes;
    gwNodes.Create(nGW);

    NodeContainer clientNodes;
    clientNodes.Create(nGW);

    NodeContainer servers;
    servers.Create(3);

    Ptr<Node> cp = servers.Get(0);
    Ptr<Node> pop = servers.Get(1);
    Ptr<Node> router = servers.Get(2);


    InternetStackHelper stack;
    stack.Install(gwNodes);
    stack.Install(clientNodes);
    stack.Install(router);
    stack.Install(cp);
    stack.Install(pop);

    //P2P configuration for LAN


    Ipv4AddressHelper addressLanScheme;
    addressLanScheme.SetBase("11.0.0.0", "255.0.0.0");


    //P2P configuration for ROUTER -> CP


    Ipv4AddressHelper addressP2P_Router_CP;
    addressP2P_Router_CP.SetBase("12.0.0.0", "255.0.0.0");

    NodeContainer routerCpContainer;
    routerCpContainer.Add(router);
    routerCpContainer.Add(cp);


    NetDeviceContainer routerCPDeviceContainer;
    routerCPDeviceContainer = pointToPointCP.Install(routerCpContainer);


    Ipv4InterfaceContainer routerCp_addrs = addressP2P_Router_CP.Assign(routerCPDeviceContainer);
    const Ipv4Address cpIpV4Addr = routerCp_addrs.GetAddress(1);

    //P2P configuration for ROUTER -> POP


    Ipv4AddressHelper addressP2P_Router_POP;
    addressP2P_Router_POP.SetBase("13.0.0.0", "255.0.0.0");

    NodeContainer routerPOPContainer;
    routerPOPContainer.Add(router);
    routerPOPContainer.Add(pop);


    NetDeviceContainer routerPopDeviceContainer;
    routerPopDeviceContainer = pointToPointPOP.Install(routerPOPContainer);

    Ipv4InterfaceContainer routerPop_addrs = addressP2P_Router_POP.Assign(routerPopDeviceContainer);
    const Ipv4Address popIpV4Addr = routerPop_addrs.GetAddress(1);



    //CSMA Configuration for GW => Router


    Ipv4AddressHelper wanAddressScheme;
    wanAddressScheme.SetBase("10.0.0.0", "255.0.0.0");

    //plug all the gw with the router

    NodeContainer wanNodes;
    wanNodes.Add(router);
    wanNodes.Add(gwNodes);


    NetDeviceContainer wanDevices;
    wanDevices = csmaHelper.Install(wanNodes);

    Ipv4InterfaceContainer csmaInterfaces;
    csmaInterfaces = wanAddressScheme.Assign(wanDevices);


    Ipv4InterfaceContainer lanInterfaceContainers[nGW];
    //now connect the peer to the gw
    for (int i = 0; i < nGW; i++) {
        NodeContainer lan;
        lan.Add(clientNodes.Get(i));
        lan.Add(gwNodes.Get(i));


        NetDeviceContainer lanDevices;
        lanDevices = pointToPointLan.Install(lan);
        lanInterfaceContainers[i] = addressLanScheme.Assign(lanDevices);

    }




    ///////////////////////////////////////////:
    // NOW THE APPLICATION PART
    //////////////////////////////////////////

    std::ostringstream ss;
    ss << "\tpop is " << popIpV4Addr << "\n";
    ss << "\tCP is " << cpIpV4Addr << "\n";
    NS_LOG_UNCOND(ss.str().c_str());

    uint16_t signalingPort = 18080;
    uint16_t configurationPort = 18081;
    uint16_t dataSourcePort = 18082;


    double currentStartTime = 0;
    for (int i = 0; i < nGW; i++) {

        std::ostringstream ss2;
        currentStartTime += ev->GetValue();

        ss2 << "client" << i << "starts at" << currentStartTime;
        NS_LOG_UNCOND(ss2 .str() );


        // GATEWAY
        const Ipv4Address gwAddress = lanInterfaceContainers[i].GetAddress(1);
        const Ipv4Address clientAddress = lanInterfaceContainers[i].GetAddress(0);
        //create the app
        Ptr<GwApplication> gwApp = CreateObject<GwApplication>();
        //create the soket to put in the app


        gwApp->Setup(InetSocketAddress(gwAddress, signalingPort),
                     InetSocketAddress(popIpV4Addr, signalingPort),
                     InetSocketAddress(cpIpV4Addr, signalingPort),
                     InetSocketAddress(popIpV4Addr, configurationPort));

        //put the application in the node
        gwNodes.Get(i)->AddApplication(gwApp);

        //setup simulator start/end date
        gwApp->SetStartTime(Seconds(0));
        gwApp->SetStopTime(END);


        // CLIENT
        //an application to trigger the download from a datasource
        Ptr<labri::ClientApplication> clientApp = CreateObject<labri::ClientApplication>();
        clientApp->Setup(InetSocketAddress(gwAddress, signalingPort),

                         InetSocketAddress(clientAddress, dataSourcePort));

        clientNodes.Get(i)->AddApplication(clientApp);


        clientApp->SetStartTime(Seconds(currentStartTime));
        clientApp->SetStopTime(END);

        PacketSinkHelper helper("ns3::TcpSocketFactory", InetSocketAddress(clientAddress, dataSourcePort));
        helper.Install(clientNodes.Get(i));


        Ptr<PacketSink> clientSinkApp = DynamicCast<PacketSink>(clientNodes.Get(i)->GetApplication(1));
        //clientNodes.Get(i)->GetId()s
        std::ostringstream oss;
        oss << "/NodeList/" << clientNodes.Get(i)->GetId() << "/ApplicationList/" << 1 << "/$ns3::PacketSink/Rx";
        Config::Connect(oss.str(), MakeCallback(&PacketAddressTrace));
        ClientStat stats;
        stats.startDate = Seconds(currentStartTime);
        stats.stopDate = Seconds(currentStartTime);
        clientStats[clientNodes.Get(i)->GetId()] = stats;


    }

    //POP Cache Controller
    Ptr<labri::CachingControllerApplication> popApp = CreateObject<labri::CachingControllerApplication>();
    popApp->Setup(
            InetSocketAddress(popIpV4Addr, configurationPort));

    pop->AddApplication(popApp);
    popApp->SetStartTime(Seconds(0));
    popApp->SetStopTime(END);

    //CP Data Source
    Ptr<labri::VideoDataSource> cpDSApp = CreateObject<labri::VideoDataSource>();
    cpDSApp->Setup(InetSocketAddress(cpIpV4Addr, signalingPort), DataRate(cpDataRate));
    cp->AddApplication(cpDSApp);
    cpDSApp->SetStartTime(Seconds(0));
    cpDSApp->SetStopTime(END);


    //POP Data Source
    Ptr<labri::VideoDataSource> popDSApp = CreateObject<labri::VideoDataSource>();
    popDSApp->Setup(InetSocketAddress(popIpV4Addr, signalingPort), DataRate(popDataRate));
    pop->AddApplication(popDSApp);
    popDSApp->SetStartTime(Seconds(0));
    popDSApp->SetStopTime(END);


    Ipv4GlobalRoutingHelper::PopulateRoutingTables();


    //pointToPointCP.EnablePcapAll("CP");
    //pointToPointLan.EnablePcapAll("Lan");
    //pointToPointPOP.EnablePcapAll("POP");




    pointToPointLan.EnablePcap("CP",routerCPDeviceContainer.Get(1),false);
    std::ostringstream oss;
    oss << "/NodeList/"<< cp->GetId()<< "/DeviceList/1/$ns3::PointToPointNetDevice/PhyTxEnd";
    std::string cpConfigPath=oss.str();
    Config::Connect(cpConfigPath, MakeCallback(&PacketTraceCallBack));
    oss.clear();
    oss << "/NodeList/"<< pop->GetId()<< "/DeviceList/1/$ns3::PointToPointNetDevice/PhyTxEnd";
    std::string popConfigPath=oss.str();
    Config::Connect(popConfigPath, MakeCallback(&PacketTraceCallBack));

    cpDr[cpConfigPath]=std::map<int,PlotData>();
    cpDr[popConfigPath]=std::map<int,PlotData>();

    Simulator::Run();

    for (int i = 0; i < nGW; i++) {
        Ptr<PacketSink> clientApp = DynamicCast<PacketSink>(clientNodes.Get(i)->GetApplication(1));
        std::stringstream ss;
        Time duration =
                clientStats[clientNodes.Get(i)->GetId()].stopDate - clientStats[clientNodes.Get(i)->GetId()].startDate;
        uint32_t totalSize = clientStats[clientNodes.Get(i)->GetId()].totalSize;
        ss << "client" << clientNodes.Get(i)->GetId() << " " << clientApp->GetTotalRx() << " (" << totalSize << ")" <<
        " in " <<
        duration.As(Time::S) << " " << clientApp->GetTotalRx() / duration.GetSeconds() / 1000 << " kBps";
        NS_LOG_UNCOND(ss.str());

    }

    std::ostringstream gnuplot;
    for(std::map<int,PlotData>::const_iterator it=cpDr[cpConfigPath].begin();it!=cpDr[cpConfigPath].end();++it){

        gnuplot << (*it).first << "\t" << (*it).second.totalSizeTransmitted << "\n";
    }
    std::cout << gnuplot.str() << std::endl;

    Simulator::Destroy();


    return 0;
}


