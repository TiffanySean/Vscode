#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#define SRC_IP "192.168.39.7"
#define DST_IP "192.168.39.8"
#define SRC_PORT 5001
#define DST_PORT 6001

void build_and_send_packet(int sockfd, struct sockaddr_in *dst_addr, char *data, int len) {
    struct iphdr *ip_header;
    struct udphdr *udp_header;
    char *packet;
    int packet_size = sizeof(struct iphdr) + sizeof(struct udphdr) + len;

    // 分配内存
    packet = malloc(packet_size);
    memset(packet, 0, packet_size);

    // 构建IP头部
    ip_header = (struct iphdr *)packet;
    ip_header->ihl = 5;
    ip_header->version = 4;
    ip_header->tos = 0;
    ip_header->tot_len = htons(packet_size);
    ip_header->id = htonl(54321);
    ip_header->frag_off = 0;
    ip_header->ttl = 255;
    ip_header->protocol = IPPROTO_UDP;
    ip_header->saddr = inet_addr(SRC_IP);
    ip_header->daddr = dst_addr->sin_addr.s_addr;

    // 构建UDP头部
    udp_header = (struct udphdr *)(packet + sizeof(struct iphdr));
    udp_header->source = htons(SRC_PORT);
    udp_header->dest = htons(DST_PORT);
    udp_header->len = htons(sizeof(struct udphdr) + len);

    // 拷贝数据
    memcpy(packet + sizeof(struct iphdr) + sizeof(struct udphdr), data, len);

    // 发送数据包
    sendto(sockfd, packet, packet_size, 0, (struct sockaddr *)dst_addr, sizeof(struct sockaddr_in));
    free(packet);
}

int main() {
    int sockfd;
    struct sockaddr_in src_addr, dst_addr;
    char *message = "Hello from Host 1";

    // 创建原始套接字
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // 绑定到本地端口
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.sin_family = AF_INET;
    src_addr.sin_port = htons(SRC_PORT);
    src_addr.sin_addr.s_addr = inet_addr(SRC_IP);
    if (bind(sockfd, (struct sockaddr *)&src_addr, sizeof(src_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    memset(&dst_addr, 0, sizeof(dst_addr));
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_port = htons(DST_PORT);
    dst_addr.sin_addr.s_addr = inet_addr(DST_IP);

    while(1) {
        build_and_send_packet(sockfd, &dst_addr, message, strlen(message));
        sleep(1); // 每秒发送一次
    }

    close(sockfd);
    return 0;
}