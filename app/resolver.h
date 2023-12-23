#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>

#define TYPE_A 1
#define TYPE_NS 2
#define TYPE_MD 3
#define TYPE_MF 4
#define TYPE_CNAME 5
#define TYPE_SOA 6
#define TYPE_MB 7
#define TYPE_MG 8
#define TYPE_MR 9
#define TYPE_NULL 10
#define TYPE_WKS 11
#define TYPE_PTR 12
#define TYPE_HINFO 13
#define TYPE_MINFO 14
#define TYPE_MX 15
#define TYPE_TXT 16

#define CLASS_IN 1

typedef struct ns_hdr {
    uint16_t id;
    uint8_t qr: 1;
    uint8_t opcode: 4;
    uint8_t aa: 1;
    uint8_t tc: 1;
    uint8_t rd: 1;
    uint8_t ra: 1;
    uint8_t z: 3;
    uint8_t rcode: 4;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
} ns_hdr_t;

typedef struct ns_q {
    // uint16_t qname;
    uint16_t qtype;
    uint16_t qclass;
} ns_q_t;

#pragma pack(push, 1)
typedef struct ns_rr_data {
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t rdlength;
} ns_rr_data_t;
#pragma pack(pop)

typedef struct ns_rr {
    unsigned char *name;
    ns_rr_data_t *resource;
    unsigned char *rdata;
} ns_rr_t;

struct resolver {
    struct sockaddr_in saddr;

    unsigned char buffer[65536];    
    ns_hdr_t *hdr;
    unsigned char *qname;
    ns_q_t *q;
    int length;
    
    unsigned char recv_buffer[65536];
    ns_hdr_t *recv_hdr;
    unsigned char *recv_qname;
    ns_q_t *recv_q;
    int resv_length;

    ns_rr_t rr[10];
    int rr_length;
};

static void message_compression(unsigned char *qname, unsigned char *domain);
static unsigned char *message_decompression(unsigned char *buffer, unsigned char *recv_buffer);

char *resolver_ttoa(unsigned int i);

int resolver_setup(struct resolver *r, char *ns_ip, int port, char *domain, int qtype, int class);

int resolver_UDP_send(struct resolver *r);
int resolver_UDP_keep_send(struct resolver *r, int timeout);
int resolver_UDP_send_recv(struct resolver *r);
int resolver_UDP_send_recv_timeout(struct resolver *r, int timeout);

int resolver_build_rr(struct resolver *r);
int resolver_print(struct resolver r);

unsigned char *resolver_rrtoa(unsigned char *rdata);
unsigned char *resolver_rrtons(unsigned char *rdata, uint16_t rdlength, unsigned char *buffer);
unsigned char *resolver_rrtosoa(unsigned char *rdata, uint16_t rdlength, unsigned char *buffer);