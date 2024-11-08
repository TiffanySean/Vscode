#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#define DST_PORT 6001

void print_ip_header(struct iphdr *ip_header) {
    printf("IP Header:\n");
    printf("  Version: %d\n", ip_header->version);
    printf("  IHL: %d\n", ip_header->ihl);
    printf("  Type of Service: %d\n", ip_header->tos);
    printf("  Total Length: %d\n", ntohs(ip_header->tot_len));
    printf("  Identification: %d\n", ntohs(ip_header->id));
    printf("  TTL: %d\n", ip_header->ttl);
    printf("  Protocol: %d\n", ip_header->protocol);
    printf("  Source Address: %s\n", inet_ntoa(*((struct in_addr *)&ip_header->saddr)));
    printf("  Dest Address: %s\n", inet_ntoa(*((struct in_addr *)&ip_header->daddr)));
}

void print_udp_header(struct udphdr *udp_header) {
    printf("UDP Header:\n");
    printf("  Source Port: %d\n", ntohs(udp_header->source));
    printf("  Dest Port: %d\n", ntohs(udp_header->dest));
    printf("  Length: %d\n", ntohs(udp_header->len));
    printf("  Checksum: %d\n", ntohs(udp_header->check));
}

int main() {
    int sockfd;
    struct sockaddr_in addr;
    char buffer[2048];
    int read_size;
    struct iphdr *ip_header;
    struct udphdr *udp_header;

    // 创建原始套接字
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // 允许套接字绑定到本机地址
    int one = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    // 绑定套接字到本机IP和端口
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DST_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // 接收任何IP地址
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    while(1) {
        // 读取数据
        read_size = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
        if (read_size < 0) {
            perror("recvfrom");
            continue;
        }

        // 解析IP头部
        ip_header = (struct iphdr *)buffer;
        if (ip_header->protocol == IPPROTO_UDP) {
            udp_header = (struct udphdr *)((char *)ip_header + ip_header->ihl * 4);
            if (ntohs(udp_header->dest) == DST_PORT) {
                char *data = (char *)udp_header + sizeof(struct udphdr);
                int data_len = ntohs(udp_header->len) - sizeof(struct udphdr);

                printf("Received data:\n");
                for (int i = 0; i < data_len; i++) {
                    printf("%02x ", (unsigned char)data[i]);
                }
                printf("\n");

                print_ip_header(ip_header);
                print_udp_header(udp_header);
            }
        }
    }

    close(sockfd);
    return 0;
}