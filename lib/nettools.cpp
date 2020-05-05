#include "nettools.hpp"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <regex>

using std::string;

// TODO: fix ping. It always returns 0;
int nettools::ping(const char *ipv4) {
    int s = socket(PF_INET, SOCK_RAW, 1);

    if(s <= 0) {
        perror("Socket Error");
        exit(0);
    }

    struct icmp_hdr_t {
        uint8_t type;
        uint8_t code;
        uint16_t chksum;
        uint32_t data;
    };

    icmp_hdr_t pckt;
    pckt.type = 8;          // The echo request is 8
    pckt.code = 0;          // No need
    pckt.chksum = 0xfff7;   // Fixed checksum since the data is not changing
    pckt.data = 0;          // We don't send anything.

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = inet_addr("8.8.8.8");

    int actionSendResult = sendto(s, &pckt, sizeof(pckt), 0, (struct sockaddr*)&addr, sizeof(addr));

    if(actionSendResult < 0) {
        perror("Ping Error");
        exit(0);
    }

    unsigned int resAddressSize;
    unsigned char res[30] = "";
    struct sockaddr resAddress;

    struct icmp_response_t {
        uint8_t type;
        uint8_t code;
        uint16_t checksum;
        uint16_t identifier;
        uint16_t sequence_number;
    };

    int r = recvfrom(s, res, sizeof(res), 0, &resAddress, &resAddressSize);

    icmp_response_t* echo_response;

    echo_response = (icmp_response_t *)&res[20];

    printf(
        "type: %x, code: %x, checksum: %x, identifier: %x, sequence: %x\n",
        echo_response->type,
        echo_response->code,
        echo_response->checksum,
        echo_response->identifier,
        echo_response->sequence_number
    );

    if (1) {
    
    } else {
        perror("Response Error");
        return -1;
    }

    return 0;
    
}

// netif_p is a pointer to the first node in a linked list
int nettools::get_netif_summaries(netif_summary **netif_p, int *num_ifs) {
    ifaddrs* addr_item_p;
    int r = getifaddrs(&addr_item_p);

    if (r < 0) {
        perror("getifaddrs()");
        return r;
    }

    netif_summary *cur_if_p = (netif_summary*)malloc(sizeof(netif_summary));
    *netif_p = cur_if_p;
    *num_ifs = 0;
    while (addr_item_p && addr_item_p->ifa_next) {     

        if (addr_item_p->ifa_netmask) {
            inet_ntop(AF_INET, &(((struct sockaddr_in *)addr_item_p->ifa_addr)->sin_addr), cur_if_p->addr, sizeof(cur_if_p->addr));
            in_addr _netmask = ((struct sockaddr_in *)addr_item_p->ifa_netmask)->sin_addr;
            in_addr _addr = ((struct sockaddr_in *)addr_item_p->ifa_addr)->sin_addr;
            struct in_addr _bcast;
            struct in_addr _netw;
            _bcast.s_addr = _addr.s_addr | ~_netmask.s_addr;
            _netw.s_addr  = _addr.s_addr & _netmask.s_addr;
            inet_ntop(AF_INET, &_netmask, cur_if_p->netmask, sizeof(cur_if_p->netmask));
            strcpy(cur_if_p->name, addr_item_p->ifa_name);
            inet_ntop(AF_INET, &_bcast, cur_if_p->bcast, sizeof(cur_if_p->bcast));
            inet_ntop(AF_INET, &_netw, cur_if_p->network, sizeof(cur_if_p->network));

            cur_if_p->next = (netif_summary*)malloc(sizeof(netif_summary));
            cur_if_p = cur_if_p->next;
            (*num_ifs)++;
        }
        
        addr_item_p = addr_item_p->ifa_next;
    };
    return 0;
}

int nettools::get_feasible_subnets(netif_summary **netif_p, int *count) {
    netif_summary *cur_net;
    int totalCount;
    *count = 0;
    get_netif_summaries(&cur_net, &totalCount);

    netif_summary *prev_feas_netif = nullptr;
    for (int i=0; i<totalCount; i++) {
        if (std::regex_match(cur_net->netmask, std::regex("255\\.255\\.\\d{0,3}\\.\\d{0,3}"))) {
            if (prev_feas_netif == nullptr) {
                *netif_p = cur_net;
            } else {
                prev_feas_netif->next = cur_net;
            }
            prev_feas_netif = cur_net;
            (*count)++;
        }
        cur_net = cur_net->next;
    }
}

bool nettools::ip_is_on_subnet(string addr, string netmask, string network) {
    // printf("checking if ip (%s/%s) is on subnet (%s)\n", addr.c_str(), netmask.c_str(), network.c_str());
    struct in_addr _addr, _netmask, _network;
    inet_pton(AF_INET, addr.c_str(), &_addr);
    inet_pton(AF_INET, netmask.c_str(), &_netmask);
    inet_pton(AF_INET, network.c_str(), &_network);
    char calculated_network[16];
    struct in_addr _calculated_network;
    _calculated_network.s_addr = _addr.s_addr & _netmask.s_addr;
    inet_ntop(AF_INET, &_calculated_network, calculated_network, sizeof(calculated_network));
    if (network.compare(calculated_network) == 0) return true;
    return false;
}