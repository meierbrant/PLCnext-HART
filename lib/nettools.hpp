#pragma once
#include <iostream>

using std::string;

namespace nettools {
    // NETwork InterFace SUMMARY
    struct netif_summary {
        netif_summary *next;
        char name[16];
        char addr[16];
        char network[16];
        char netmask[16];
        char bcast[16];
    };

    int ping(const char *ipv4);
    int get_netif_summaries(netif_summary **netif_p, int *num_ifs);
    int get_feasible_subnets(netif_summary **netif_p, int *count);
    bool ip_is_on_subnet(string addr, string netmask, string network);

}