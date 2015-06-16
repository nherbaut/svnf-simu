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
#include "ns3/stats-module.h"
#include "commons.h"

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


#pragma GCC diagnostic ignored "-Wwrite-strings"


using namespace ns3;


std::map<std::string, ClientDataFromDataSource *> g_clientData;

//std::map<int, ClientStatObservedFromMain> clientStats;
extern std::map<int, int> newResourcesStartTime;

NS_LOG_COMPONENT_DEFINE ("SVNF");


void
DataRateTrace(uint32_t oldValue, uint32_t newValue) {
    std::cout << "Traced " << oldValue << " to " << newValue << std::endl;
}


std::map<int, ServerPlotData> cpDr;
std::map<int, ServerPlotData> popDr;

void PacketTraceCallBackCP(std::string context, const Ptr<const Packet> packet) {


    int second = (int) std::round(Simulator::Now().GetSeconds());
    if (!cpDr.count(second)) {
        cpDr[second] = ServerPlotData();
    }
    cpDr[second].totalSizeTransmitted += packet->GetSize();
}

void PacketTraceCallBackPOP(std::string context, const Ptr<const Packet> packet) {

    int second = (int) std::round(Simulator::Now().GetSeconds());
    if (!popDr.count(second)) {
        popDr[second] = ServerPlotData();
    }
    popDr[second].totalSizeTransmitted += packet->GetSize();
}


/*
void PacketAddressTrace(std::string context, const Ptr<const Packet> packet, const Address &address) {

    int s = boost::lexical_cast<int>(context.substr(10, context.substr(10, context.length() - 10).find("/")));
    clientStats[s].stopDate = Simulator::Now();
    clientStats[s].totalSize += packet->GetSize();


}*/

int
main(int argc, char *argv[]) {
    bool verbose = true;
    uint32_t nGW = 0;
    double mat = 1;
    Time END = Seconds(1000);

    double vcs = 1000.;
    double vcv = 200.;


    char popDataRate[30] = "100kbps";
    char cpDataRate[30] = "100kbps";
    uint32_t payloadSize = 100000;

    CommandLine cmd;
    cmd.AddValue("nGW", "Number of \"extra\" CSMA nodes/devices", nGW);
    cmd.AddValue("mat", "Mean arrival time for clients", mat);
    cmd.AddValue("pop", "Data Rate for POP", popDataRate);
    cmd.AddValue("cp", "Data Rate for cp", cpDataRate);
    cmd.AddValue("as", "payload average size", payloadSize);
    cmd.AddValue("vcs", "video corpus size", vcs);
    cmd.AddValue("vcv", "video corpus variance", vcv);

    cmd.Parse(argc, argv);



    //Config::SetDefault("ns3::TcpSocket::SndBufSize", ns3::UintegerValue(100000));
    //Config::SetDefault("ns3::TcpSocket::RcvBufSize", ns3::UintegerValue(100000));
    //Config::SetDefault("ns3::TcpSocket::SegmentSize", ns3::UintegerValue(1460));
    //Config::SetDefault("ns3::TcpSocketBase::ReTxTimeout",ns3::TimeValue(Seconds(1000)));



    Ptr<NormalRandomVariable> normal = CreateObject<NormalRandomVariable>();
    normal->SetAttribute("Mean", DoubleValue(vcs / 2.));
    normal->SetAttribute("Variance", DoubleValue(vcv));




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

    Ptr<ParetoRandomVariable> pv = CreateObject<ParetoRandomVariable>();
    pv->SetAttribute("Mean", DoubleValue(payloadSize));
    pv->SetAttribute("Shape", DoubleValue(3));
    pv->SetAttribute("Bound", DoubleValue(10 * payloadSize));





    //LogComponentEnable("Ipv4StaticRouting", LOG_LEVEL_ALL);
    //LogComponentEnable("TcpSocketBase", LOG_LEVEL_ALL);
    //LogComponentEnable("SVNF", LOG_LEVEL_ALL);
    //LogComponentEnable("GwApplication", LOG_LEVEL_ALL);
    //LogComponentEnable("ClientApplication", LOG_LEVEL_ALL);
    //LogComponentEnable("CachingControllerApplication", LOG_LEVEL_ALL);
    //LogComponentEnable("NscTcpSocketImpl", LOG_LEVEL_ALL);
    //LogComponentEnable("PacketSink", LOG_LEVEL_ALL);
    //LogComponentEnable("VideoDataSource", LOG_LEVEL_ALL);
    //LogComponentEnable("TcpTxBuffer", LOG_LEVEL_ALL);
    //LogComponentEnable("TcpNewReno", LOG_LEVEL_ALL);$
    //LogComponentEnable("Ipv4StaticRouting", LOG_LEVEL_ALL);



    ///////////////////////////////////////////////::
    // BUILDING THE TOPOLOGY
    ///////////////////////////////////////////////


    Ipv4StaticRoutingHelper ipv4RoutingHelper;


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

    Ipv4Address routerWanIp = csmaInterfaces.GetAddress(0);


    Ipv4InterfaceContainer lanInterfaceContainers[nGW];
    //now connect the peer to the gw
    for (int i = 0; i < nGW; i++) {

        NodeContainer lan;
        lan.Add(clientNodes.Get(i));
        lan.Add(gwNodes.Get(i));


        NetDeviceContainer lanDevices;
        lanDevices = pointToPointLan.Install(lan);
        lanInterfaceContainers[i] = addressLanScheme.Assign(lanDevices);
        Ipv4Address gwAddress = lanInterfaceContainers[i].GetAddress(1);
        Ipv4Address clientAddress = lanInterfaceContainers[i].GetAddress(0);

        //std::cout <<  "gw=" << gwAddress << " & " << csmaInterfaces.GetAddress(i+1) << " cl=" <<  clientAddress << std::endl;


        //client to pop and cp via gw
        {
            Ptr<Ipv4StaticRouting> staticRouteClient = ipv4RoutingHelper.GetStaticRouting(
                    lanInterfaceContainers[i].Get(0).first);
            staticRouteClient->AddNetworkRouteTo(Ipv4Address("13.0.0.0"), Ipv4Mask("255.0.0.0"), gwAddress, 1, 0);
            staticRouteClient->AddNetworkRouteTo(Ipv4Address("12.0.0.0"), Ipv4Mask("255.0.0.0"), gwAddress, 1, 0);


        }


        {
            Ptr<Ipv4StaticRouting> staticRouteClient = ipv4RoutingHelper.GetStaticRouting(
                    lanInterfaceContainers[i].Get(1).first);
            staticRouteClient->AddNetworkRouteTo(Ipv4Address("13.0.0.0"), Ipv4Mask("255.0.0.0"), routerWanIp, 1, 0);
            staticRouteClient->AddNetworkRouteTo(Ipv4Address("12.0.0.0"), Ipv4Mask("255.0.0.0"), routerWanIp, 1, 0);


        }


    }


    //POP to client via router
    {
        Ptr<Ipv4StaticRouting> staticRoutePOP = ipv4RoutingHelper.GetStaticRouting(routerPop_addrs.Get(1).first);
        staticRoutePOP->AddNetworkRouteTo(Ipv4Address("11.0.0.0"), Ipv4Mask("255.0.0.0"), routerPop_addrs.GetAddress(0),
                                          1, 0);
        staticRoutePOP->AddNetworkRouteTo(Ipv4Address("10.0.0.2"), Ipv4Mask("255.0.0.0"), routerPop_addrs.GetAddress(0),
                                          1, 0);


    }



    //CP to client via router
    {
        Ptr<Ipv4StaticRouting> staticRouteCP = ipv4RoutingHelper.GetStaticRouting(routerCp_addrs.Get(1).first);
        staticRouteCP->AddNetworkRouteTo(Ipv4Address("11.0.0.0"), Ipv4Mask("255.0.0.0"), routerCp_addrs.GetAddress(0),
                                         1, 0);
        staticRouteCP->AddNetworkRouteTo(Ipv4Address("10.0.0.2"), Ipv4Mask("255.0.0.0"), routerCp_addrs.GetAddress(0),
                                         1, 0);


    }



    //Router to client via GW
    {


        Ptr<Ipv4StaticRouting> staticRouterouter = ipv4RoutingHelper.GetStaticRouting(csmaInterfaces.Get(0).first);
        for (int i = 0; i < nGW; i++) {


            staticRouterouter->AddNetworkRouteTo(lanInterfaceContainers[i].GetAddress(0), Ipv4Mask("255.255.255.255"),
                                                 csmaInterfaces.GetAddress(i + 1),
                                                 3, 0);


        }


    }

    ///////////////////////////////////////////:
    // NOW THE APPLICATION PART
    //////////////////////////////////////////

    std::ostringstream ss;
    ss << "\tpop is " << popIpV4Addr << "\n";
    ss << "\tCP is " << cpIpV4Addr << "\n";
    //NS_LOG_UNCOND(ss.str().c_str());

    const uint16_t signalingPort = 18080;
    const uint16_t configurationPort = 18081;
    const uint16_t dataSourcePort = 18082;


    double currentStartTime = 0;
    for (int i = 0; i < nGW; i++) {

        std::ostringstream ss2;
        currentStartTime += ev->GetValue();

        ss2 << "client" << i << "starts at" << currentStartTime;
        //NS_LOG_UNCOND(ss2.str());


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

        ClientDataFromDataSource *cdfs = new ClientDataFromDataSource(
                boost::lexical_cast<std::string>(normal->GetInteger()), pv->GetInteger(), DataRate("320kbps"));
        cdfs->setIp(clientAddress);
        cdfs->setStartDate(Seconds(currentStartTime));
        cdfs->setEndDate(END);
        g_clientData[cdfs->getIp()] = cdfs;


        Ptr<labri::ClientApplication> clientApp = CreateObject<labri::ClientApplication>();
        clientApp->Setup(InetSocketAddress(gwAddress, signalingPort),

                         InetSocketAddress(clientAddress, dataSourcePort), cdfs

        );

        clientNodes.Get(i)->AddApplication(clientApp);


        clientApp->SetStartTime(Seconds(currentStartTime));
        clientApp->SetStopTime(END);

        PacketSinkHelper helper("ns3::TcpSocketFactory", InetSocketAddress(clientAddress, dataSourcePort));
        helper.Install(clientNodes.Get(i));


        Ptr<PacketSink> clientSinkApp = DynamicCast<PacketSink>(clientNodes.Get(i)->GetApplication(1));


        //check if client receive the right amount of bytes
        /*
        std::ostringstream oss;
        oss << "/NodeList/" << clientNodes.Get(i)->GetId() << "/ApplicationList/" << 1 << "/$ns3::PacketSink/Rx";
        Config::Connect(oss.str(), MakeCallback(&PacketAddressTrace));
        ClientStatObservedFromMain stats;
        stats.startDate = Seconds(currentStartTime);
        stats.stopDate = Seconds(currentStartTime);
        clientStats[clientNodes.Get(i)->GetId()] = stats;
         */


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


    std::cout << "computing routing" << std::endl;

    //Ipv4GlobalRoutingHelper::PopulateRoutingTables();



    std::cout << "computing routing done" << std::endl;


    //pointToPointCP.EnablePcapAll("CP");
    //pointToPointLan.EnablePcapAll("Lan");
    //pointToPointPOP.EnablePcapAll("POP");


    //pointToPointLan.EnablePcap("CP", routerCPDeviceContainer.Get(1), false);

    ///////////////////////////////////:
    // Spy for CP and POP datarate
    //////////////////////////////////

    std::ostringstream oss;
    oss << "/NodeList/" << cp->GetId() << "/DeviceList/1/$ns3::PointToPointNetDevice/PhyTxEnd";
    std::string cpConfigPath = oss.str();
    Config::Connect(cpConfigPath, MakeCallback(&PacketTraceCallBackCP));
    oss.str("");
    oss.clear();
    oss << "/NodeList/" << pop->GetId() << "/DeviceList/1/$ns3::PointToPointNetDevice/PhyTxEnd";
    std::string popConfigPath = oss.str();
    Config::Connect(popConfigPath, MakeCallback(&PacketTraceCallBackPOP));

    //std::cout << "sim run" << std::endl;
    Simulator::Run();
    //std::cout << "sim ran" << std::endl;

    /*
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

    }*/

    //////////////////////////////::
    // Plot server Loads
    //////////////////////////////

    std::vector<int> keys;
    transform(cpDr.begin(), cpDr.end(), back_inserter(keys), RetrieveKey());
    transform(popDr.begin(), popDr.end(), back_inserter(keys), RetrieveKey());

    std::sort(keys.begin(), keys.end());
    keys.erase(std::unique(keys.begin(), keys.end()), keys.end());

    std::ofstream ofs("server-loads.csv", std::ofstream::out);


    std::map<int, int> cumulativeDroppedBySeconds;

    for (std::map<std::string, ClientDataFromDataSource *>::const_iterator itr = g_clientData.begin();
         itr != g_clientData.end(); ++itr) {
        if ((*itr).second->isDropped()) {
            int second = (int) std::round((*itr).second->getDroppedDate().GetSeconds());
            if (cumulativeDroppedBySeconds.count(second)) {
                cumulativeDroppedBySeconds[second] = 0;
            }
            cumulativeDroppedBySeconds[second] += 1;
        }
    }

    int count_dropped = 0;
    for (std::vector<int>::const_iterator it = keys.begin(); it != keys.end(); ++it) {

        double popValue = 0;
        double cpValue = 0;


        if (cpDr.count(*it)) {
            cpValue = cpDr[*it].totalSizeTransmitted;
        }

        if (popDr.count(*it)) {
            popValue = popDr[*it].totalSizeTransmitted;
        }

        if (cumulativeDroppedBySeconds.count(*it)) {
            count_dropped += cumulativeDroppedBySeconds[*it];

        }
        ofs << (*it) << "\t" << cpValue << "\t" << popValue << "\t" << count_dropped << std::endl;


    }
    ofs.close();


    Simulator::Destroy();
    for (std::map<std::string, ClientDataFromDataSource *>::const_iterator itr = g_clientData.begin();
         itr != g_clientData.end(); ++itr) {
        delete (*itr).second;
    }


    return 0;
}




