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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "StreamingApplicationServer.h"
#include <ns3/data-rate.h>
// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1   n2   n3   n4
//    point-to-point  |    |    |    |
//                    ================
//                      LAN 10.1.2.0


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SecondScriptExample");

int
main(int argc, char *argv[]) {
    bool verbose = true;
    uint32_t nGW = 2;

    CommandLine cmd;
    cmd.AddValue("nCsma", "Number of \"extra\" CSMA nodes/devices", nGW);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);

    cmd.Parse(argc, argv);

    if (verbose) {
        LogComponentEnable("OnOffApplication", LOG_LEVEL_INFO);
        LogComponentEnable("PacketSink", LOG_LEVEL_INFO);
    }

    nGW = nGW == 0 ? 1 : nGW;


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

    NodeContainer cpRouterContainer;
    cpRouterContainer.Add(router);
    cpRouterContainer.Add(cp);


    NetDeviceContainer routerCPDeviceContainer;
    routerCPDeviceContainer = pointToPointCP.Install(cpRouterContainer);

    Ipv4InterfaceContainer routerCp_add = addressP2P_Router_CP.Assign(routerCPDeviceContainer);




    //P2P configuration for ROUTER -> POP
    PointToPointHelper pointToPointPOP;
    pointToPointPOP.SetDeviceAttribute("DataRate", StringValue("2Gbps"));
    pointToPointPOP.SetChannelAttribute("Delay", StringValue("100ms"));

    Ipv4AddressHelper addressP2P_Pop_Router;
    addressP2P_Pop_Router.SetBase("192.168.2.0", "255.255.255.0");

    NodeContainer popRouterContainer;
    popRouterContainer.Add(pop);
    popRouterContainer.Add(router);

    NetDeviceContainer popRouterDeviceContainer;
    routerCPDeviceContainer = pointToPointPOP.Install(popRouterContainer);

    Ipv4InterfaceContainer popRouter_addr = addressP2P_Pop_Router.Assign(popRouterDeviceContainer);


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


    uint16_t port = 50000;


    PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
    ApplicationContainer sinkApp = sinkHelper.Install (cp);
    sinkApp.Start (Seconds (1.0));
    sinkApp.Stop (Seconds (100.0));





    AddressValue remoteAddress
            (InetSocketAddress (routerCp_add.GetAddress(1), port));
    OnOffHelper clientHelper ("ns3::TcpSocketFactory", remoteAddress.Get());

    clientHelper.SetConstantRate(DataRate ("100b/s"),10);




    for (int i = 0; i < nGW; i++) {

        ApplicationContainer clientApps = clientHelper .Install(clientNodes.Get(i));
        clientApps.Start(Seconds(2.0));
        clientApps.Stop(Seconds(100.0));
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();



    pointToPointCP.EnablePcapAll("CP");
    pointToPointLan.EnablePcapAll("Lan");
    pointToPointPOP.EnablePcapAll("POP");


    Simulator::Run();
    Simulator::Destroy();
    return 0;
}