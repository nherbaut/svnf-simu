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
// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1   n2   n3   n4
//    point-to-point  |    |    |    |
//                    ================
//                      LAN 10.1.2.0


using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("SVNF");

int
main(int argc, char *argv[]) {
    bool verbose = true;
    uint32_t nGW = 2;

    CommandLine cmd;
    cmd.AddValue("nCsma", "Number of \"extra\" CSMA nodes/devices", nGW);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);

    cmd.Parse(argc, argv);


    //LogComponentEnable("TcpSocketBase", LOG_LEVEL_INFO);
    LogComponentEnable("SVNF", LOG_LEVEL_ALL);
    LogComponentEnable("GwApplication", LOG_LEVEL_ALL);
    LogComponentEnable("ClientApplication", LOG_LEVEL_ALL);
    LogComponentEnable("CachingControllerApplication", LOG_LEVEL_ALL);
    LogComponentEnable("VideoDataSource", LOG_LEVEL_ALL);
    LogComponentEnable("PacketSink", LOG_LEVEL_ALL);


    nGW = 1;


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
    PointToPointHelper pointToPointLan;
    pointToPointLan.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    pointToPointLan.SetChannelAttribute("Delay", StringValue("1ms"));

    Ipv4AddressHelper addressLanScheme;
    addressLanScheme.SetBase("192.168.0.0", "255.255.255.0");


    //P2P configuration for ROUTER -> CP
    PointToPointHelper pointToPointCP;
    pointToPointCP.SetDeviceAttribute("DataRate", StringValue("2Gbps"));
    pointToPointCP.SetChannelAttribute("Delay", StringValue("200ms"));

    Ipv4AddressHelper addressP2P_Router_CP;
    addressP2P_Router_CP.SetBase("192.168.1.0", "255.255.255.0");

    NodeContainer routerCpContainer;
    routerCpContainer.Add(router);
    routerCpContainer.Add(cp);


    NetDeviceContainer routerCPDeviceContainer;
    routerCPDeviceContainer = pointToPointCP.Install(routerCpContainer);


    Ipv4InterfaceContainer routerCp_addrs = addressP2P_Router_CP.Assign(routerCPDeviceContainer);
    const Ipv4Address cpIpV4Addr = routerCp_addrs.GetAddress(1);

    //P2P configuration for ROUTER -> POP
    PointToPointHelper pointToPointPOP;
    pointToPointPOP.SetDeviceAttribute("DataRate", StringValue("2Gbps"));
    pointToPointPOP.SetChannelAttribute("Delay", StringValue("10ms"));

    Ipv4AddressHelper addressP2P_Router_POP;
    addressP2P_Router_POP.SetBase("192.168.2.0", "255.255.255.0");

    NodeContainer routerPOPContainer;
    routerPOPContainer.Add(router);
    routerPOPContainer.Add(pop);


    NetDeviceContainer routerPopDeviceContainer;
    routerPopDeviceContainer = pointToPointPOP.Install(routerPOPContainer);

    Ipv4InterfaceContainer routerPop_addrs = addressP2P_Router_POP.Assign(routerPopDeviceContainer);
    const Ipv4Address popIpV4Addr = routerPop_addrs.GetAddress(1);



    //CSMA Configuration for GW => Router
    CsmaHelper csmaHelper;
    csmaHelper.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csmaHelper.SetChannelAttribute("Delay", TimeValue(MilliSeconds(10)));

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

    uint16_t signalingPort = 18080;
    uint16_t configurationPort = 18081;
    uint16_t dataSourcePort = 18082;


    for (int i = 0; i < nGW; i++) {



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
        gwApp->SetStartTime(Seconds(3.));
        gwApp->SetStopTime(Seconds(20.));


        // CLIENT
        //an application to trigger the download from a datasource
        Ptr<labri::ClientApplication> clientApp = CreateObject<labri::ClientApplication>();
        clientApp->Setup(InetSocketAddress(gwAddress, signalingPort),

                         InetSocketAddress(clientAddress, dataSourcePort));

        clientNodes.Get(i)->AddApplication(clientApp);
        clientApp->SetStartTime(Seconds(15));
        clientApp->SetStopTime(Seconds(20.));

        PacketSinkHelper helper("ns3::TcpSocketFactory", InetSocketAddress(clientAddress, dataSourcePort));
        helper.Install(clientNodes.Get(i));

    }

    //POP Cache Controller
    Ptr<labri::CachingControllerApplication> popApp = CreateObject<labri::CachingControllerApplication>();
    popApp->Setup(
                 InetSocketAddress(popIpV4Addr, configurationPort));

    pop->AddApplication(popApp);
    popApp->SetStartTime(Seconds(0));
    popApp->SetStopTime(Seconds(20.));

    //CP Data Source
    Ptr<labri::VideoDataSource> cpDSApp = CreateObject<labri::VideoDataSource>();
    cpDSApp->Setup(InetSocketAddress(cpIpV4Addr, signalingPort));
    cp->AddApplication(cpDSApp);
    cpDSApp->SetStartTime(Seconds(0));
    cpDSApp->SetStopTime(Seconds(20.));



    Ipv4GlobalRoutingHelper::PopulateRoutingTables();


    //pointToPointCP.EnablePcapAll("CP");
    //pointToPointLan.EnablePcapAll("Lan");
    pointToPointPOP.EnablePcapAll("POP");


    Simulator::Run();
    Simulator::Destroy();
    return 0;
}


