#include "resolver.h"

static void message_compression(unsigned char *qname, unsigned char *domain) {
    int cur_begin = 0;
    unsigned char *qname_ptr = qname;
    strcat(domain, ".");
    for(int i = 0; i < strlen(domain); i++) {
        if(domain[i] == '.') {
            int size = i - cur_begin;
            *qname = size;
            qname++;
            for(int j = cur_begin; j < i; j++) {
                *qname = domain[j];
                qname++;
            }
            cur_begin = i + 1;
        }
    }
    *qname = 0;
    qname = qname_ptr;
    return;
}

static unsigned char *message_decompression(unsigned char *buffer, unsigned char *recv_buffer) {
    unsigned char *addr = malloc(256 * sizeof(unsigned char));
    int addr_length;
    int i;

    i = 0;
    addr_length = 0;
    memset(addr, 0, 256);

    while(buffer[i]) {
        if(buffer[i] < (0B11 << 6)) {
            for(int j = 0; j < buffer[i]; j++) {
                addr[addr_length++] = buffer[i + j + 1];
            }
            i += buffer[i] + 1;

            strcat(addr, ".");
            addr_length += 1;
        } 
        else {
            int oct_sum;
            unsigned char *name;

            oct_sum = (buffer[i] << 8) + buffer[i + 1];
            oct_sum &= (0B1 << 14) - 1;

            name = message_decompression(&recv_buffer[oct_sum], recv_buffer);
            strcat(addr, name);
            addr_length += strlen(name);
            i += 2;
            break;
        }
    }
    return addr;
}

char *resolver_ttoa(unsigned int i) {
    char *c;
    c = malloc(6 * sizeof(char));
    switch(i) {
        case TYPE_A:
            strcpy(c, "A");
            break;
        case TYPE_NS:
            strcpy(c, "NS");
            break;
        case TYPE_MD:
            strcpy(c, "MD");
            break;
        case TYPE_MF:
            strcpy(c, "MF");
            break;
        case TYPE_CNAME:
            strcpy(c, "CNAME");
            break;
        case TYPE_SOA:
            strcpy(c, "SOA");
            break;
        case TYPE_MB:
            strcpy(c, "MB");
            break;
        case TYPE_MG:
            strcpy(c, "MG");
            break;
        case TYPE_MR:
            strcpy(c, "MR");
            break;
        case TYPE_NULL:
            strcpy(c, "NULL");
            break;
        case TYPE_WKS:
            strcpy(c, "WKS");
            break;
        case TYPE_PTR:
            strcpy(c, "PTR");
            break;
        case TYPE_HINFO:
            strcpy(c, "HINFO");
            break;
        case TYPE_MINFO:
            strcpy(c, "MINFO");
            break;
        case TYPE_MX:
            strcpy(c, "MX");
            break;
        case TYPE_TXT:
            strcpy(c, "TXT");
            break;
        default:
            strcpy(c, "UNDEF");
    }
    return c;
}

int resolver_setup(struct resolver *r, char *ns_ip, int port, char *domain, int qclass, int qtype) {
    memset(r->buffer, 0, sizeof(char));

    r->hdr = (ns_hdr_t *)&r->buffer;
    r->hdr->id = htons(rand() % 65535);
    r->hdr->qr = 0;
    r->hdr->aa = 0;
    r->hdr->tc = 0;
    r->hdr->rd = 0;
    r->hdr->ra = 0;
    r->hdr->z = 0;
    r->hdr->qdcount = htons(1);
    r->hdr->ancount = 0;
    r->hdr->nscount = 0;
    r->hdr->arcount = 0;
    r->qname = &r->buffer[sizeof(ns_hdr_t)];
    message_compression(r->qname, domain);
    r->q = (ns_q_t *)&r->buffer[sizeof(ns_hdr_t) + strlen(r->qname) + 1];
    r->q->qtype = htons(qtype);
    r->q->qclass = htons(qclass);
    r->length = sizeof(r->hdr) + strlen(r->qname) + 1 + sizeof(r->q);
    
    memset(&r->saddr, 0, sizeof(r->saddr));
    r->saddr.sin_family = AF_INET;
    r->saddr.sin_port = htons(port);
    r->saddr.sin_addr.s_addr = inet_addr(ns_ip);
    return 0;
}

int resolver_UDP_send(struct resolver *r) {
    int fd, res;
    
    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(fd == -1) {
        printf("resolver_UDP_send: socket error\n");
        return -1;
    }

    res = sendto(fd, r->buffer, r->length, 0, (struct sockaddr *)&r->saddr, sizeof(r->saddr));
    close(fd);
    if(res == -1) {    
        printf("resolver_UDP_send: sendto error\n");
        return -1;
    }
    return 0;
}

int resolver_UDP_keep_send(struct resolver *r, int timeout) {
    struct itimerval it;
    int fd, res;

    it.it_interval.tv_sec = 1;
    it.it_interval.tv_usec = 0;
    it.it_value.tv_sec = timeout;
    it.it_value.tv_usec = 0;

    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(fd == -1) {
        printf("resolver_UDP_keep_send: socket error\n");
        return -1;
    }

    res = setitimer(ITIMER_REAL, &it, NULL);
    if(res == -1) {
        printf("resolver_UDP_keep_send: settimer error\n");
        close(fd);
        return -1;
    }

    do {
        res = sendto(fd, r->buffer, r->length, 0, (struct sockaddr *)&r->saddr, sizeof(r->saddr));
    
        if(res == -1) {    
            printf("resolver_UDP_keep_send: sendto error\n");
            close(fd);
            return -1;
        }

        getitimer(ITIMER_REAL, &it);
    } while(it.it_value.tv_sec > 0);
    close(fd);
    return 0;
}

int resolver_UDP_send_recv(struct resolver *r) {
    int fd, res;

    memset(r->recv_buffer, 0, sizeof(r->recv_buffer));

    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(fd == -1) {
        printf("resolver_UDP_send_recv: socket error\n");
        return -1;
    }

    res = sendto(fd, r->buffer, r->length, 0, (struct sockaddr *)&r->saddr, sizeof(r->saddr));
    if(res == -1) {
        close(fd);
        printf("resolver_UDP_send_recv: sendto error\n");
        return -1;
    }
    
    res = recvfrom(fd, r->recv_buffer, sizeof(r->recv_buffer), 0, NULL, NULL);
    close(fd);
    if(res == -1) {
        printf("resolver_UDP_send_recv: recvfrom error\n");
        return -1;
    }
    return 0;
}

int resolver_UDP_send_recv_timeout(struct resolver *r, int timeout) {
    struct timeval tv;
    int fd, res;

    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    memset(r->recv_buffer, 0, sizeof(r->recv_buffer));

    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(fd == -1) {
        printf("resolver_UDP_send_recv_timeout: socket error\n");
        return -1;
    }

    res = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    if(res == -1) {
        printf("resolver_UDP_send_recv_timeout: setsockopt error\n");
        close(fd);
        return -1;
    }

    res = sendto(fd, r->buffer, r->length, 0, (struct sockaddr *)&r->saddr, sizeof(r->saddr));
    if(res == -1) {
        close(fd);
        printf("resolver_UDP_send_recv_timeout: sendto error\n");
        return -1;
    }
    
    res = recvfrom(fd, r->recv_buffer, sizeof(r->recv_buffer), 0, NULL, NULL);
    close(fd);
    if(res == -1) {
        if(errno != EAGAIN && errno != EWOULDBLOCK) {
            printf("resolver_UDP_send_recv_timeout: recvfrom error\n");
        }
        return -1;
    }
    return 0;
}

int resolver_build_rr(struct resolver *r) {
    r->recv_hdr = (ns_hdr_t *)&r->recv_buffer;
    r->recv_qname = &r->recv_buffer[sizeof(ns_hdr_t)];
    r->recv_q = (ns_q_t *)&r->recv_buffer[sizeof(ns_hdr_t) + strlen(r->recv_qname) + 1];
    unsigned char *recv_an = &r->recv_buffer[sizeof(ns_hdr_t) + strlen(r->recv_qname) + 1 + sizeof(ns_q_t)];

    for(int i = 0; i < ntohs(r->recv_hdr->ancount); i++) {
        int j;
        int name_length;

        r->rr[i].name = malloc(256 * sizeof(char));
        
        j = 0;
        name_length = 0;

        while(recv_an[j]) {
            if(recv_an[j] < (0B11 << 6)) {
                unsigned char *name;
                
                name = message_decompression(&recv_an[j], r->recv_buffer);
                strcat(r->rr[i].name, name);
                name_length += strlen(name);
                j += recv_an[j] + 1;
            }
            else {
                int oct_sum;
                unsigned char *name;

                oct_sum = (recv_an[j] << 8) + recv_an[j + 1];
                oct_sum &= (0B1 << 14) - 1;

                name = message_decompression(&r->recv_buffer[oct_sum], r->recv_buffer);
                strcat(r->rr[i].name, name);
                name_length += strlen(name);
                j += 2;
                break;
            }
        }
        
        // from NAME jump to TYPE
        recv_an += j;

        r->rr[i].resource = (ns_rr_data_t *)recv_an;
        recv_an += sizeof(ns_rr_data_t);

        // move data to rdata
        r->rr[i].rdata = (unsigned char *)malloc(ntohs(r->rr[i].resource->rdlength));
        for(int j = 0; j < ntohs(r->rr[i].resource->rdlength); j++) {
            r->rr[i].rdata[j] = recv_an[j];
        }

        r->rr[i].rdata[ntohs(r->rr[i].resource->rdlength)] = 0;
        recv_an += ntohs(r->rr[i].resource->rdlength);
    }
    r->rr_length = ntohs(r->recv_hdr->ancount);
    return 0;
}

int resolver_print(struct resolver r) {
    printf("%d\n", r.rr_length);
    for(int i = 0; i < r.rr_length; i++) {
        unsigned int type;
        type = ntohs(r.rr[i].resource->type);
        
        printf("%s %s ", r.rr[i].name, resolver_ttoa(type));
        
        if(type == TYPE_A) {
            printf("%s", resolver_rrtoa(r.rr[i].rdata));
        }
        else if(type == TYPE_NS) {
            printf("%s", resolver_rrtons(r.rr[i].rdata, ntohs(r.rr[i].resource->rdlength), r.recv_buffer));
        }
        else if(type == TYPE_SOA) {
            printf("%s", resolver_rrtosoa(r.rr[i].rdata, ntohs(r.rr[i].resource->rdlength), r.recv_buffer));
        }
        else {
            printf("UNDEFINED");
        }
        
        printf("\n");
    }
    return 0;
}

unsigned char *resolver_rrtoa(unsigned char *rdata) {
    unsigned char *addr = malloc(256 * sizeof(unsigned char));
    sprintf(addr, "%d.%d.%d.%d", rdata[0], rdata[1], rdata[2], rdata[3]);
    return addr;
}

unsigned char *resolver_rrtons(unsigned char *rdata, uint16_t rdlength, unsigned char *buffer) {
    unsigned char *addr = malloc(256 * sizeof(unsigned char));
    int addr_length = 0;
    int i = 0;

    while(rdata[i]) {
        if(rdata[i] < (0B11 << 6)) {
            for(int j = 0; j < rdata[i]; j++) {
                addr[addr_length++] = rdata[i + j + 1];
            }
            i += rdata[i] + 1;

            strcat(addr, ".");
            addr_length += 1;
        }
        else {
            int oct_sum;
            unsigned char *name;

            oct_sum = (rdata[i] << 8) + rdata[i + 1];
            oct_sum &= (0B1 << 14) - 1;
            
            name = message_decompression(&buffer[oct_sum], buffer);
            strcat(addr, name);
            addr_length += strlen(name);
            i += 2;
            break;
        }
    }
    return addr;
}

unsigned char *resolver_rrtosoa(unsigned char *rdata, uint16_t rdlength, unsigned char *buffer) {
    unsigned char *addr = malloc(256 * sizeof(unsigned char));
    int addr_length = 0;
    unsigned char *temp = malloc(256 * sizeof(unsigned char));
    int i;
    
    /* MNAME */
    i = 0;
    while(rdata[i]) {
        if(rdata[i] < (0B11 << 6)) {
            for(int j = 0; j < rdata[i]; j++) {
                addr[addr_length++] = rdata[i + j + 1];
            }
            i += rdata[i] + 1;

            strcat(addr, ".");
            addr_length += 1;
        }
        else {
            int oct_sum;
            unsigned char *name;

            oct_sum = (rdata[i] << 8) + rdata[i + 1];
            oct_sum &= (0B1 << 14) - 1;

            name = message_decompression(&buffer[oct_sum], buffer);
            strcat(addr, name);
            addr_length += strlen(name);
            i += 2;
            break;
        }
    }

    if(rdata[i] == 0) {
        i += 1;
    }
    strcat(addr, " ");
    addr_length += 1;

    /* RNAME */
    while(rdata[i]) {
        if(rdata[i] < (0B11 << 6)) {
            for(int j = 0; j < rdata[i]; j++) {
                addr[addr_length++] = rdata[i + j + 1];
            }
            i += rdata[i] + 1;

            strcat(addr, ".");
            addr_length += 1;
        }
        else {
            int oct_sum;
            unsigned char *name;

            oct_sum = (rdata[i] << 8) + rdata[i + 1];
            oct_sum &= (0B1 << 14) - 1;
            
            name = message_decompression(&buffer[oct_sum], buffer);
            strcat(addr, name);
            addr_length += strlen(name);
            i += 2;
            break;
        }
    }

    if(rdata[i] == 0) {
        i += 1;
    }
    strcat(addr, " ");
    addr_length += 1;

    /* SERIAL */
    sprintf(temp, "%d ", (rdata[i] << 24) + (rdata[i + 1] << 16) + (rdata[i + 2] << 8) + rdata[i + 3]);
    strcat(addr, temp);
    addr_length += strlen(temp);
    i += 4;
    /* REFRESH */
    sprintf(temp, "%d ", (rdata[i] << 24) + (rdata[i + 1] << 16) + (rdata[i + 2] << 8) + rdata[i + 3]);
    strcat(addr, temp);
    addr_length += strlen(temp);
    i += 4;
    /* RETRY */
    sprintf(temp, "%d ", (rdata[i] << 24) + (rdata[i + 1] << 16) + (rdata[i + 2] << 8) + rdata[i + 3]);
    strcat(addr, temp);
    addr_length += strlen(temp);
    i += 4;
    /* EXPIRE */
    sprintf(temp, "%d ", (rdata[i] << 24) + (rdata[i + 1] << 16) + (rdata[i + 2] << 8) + rdata[i + 3]);
    strcat(addr, temp);
    addr_length += strlen(temp);
    i += 4;
    /* MINIMUM */
    sprintf(temp, "%d", (rdata[i] << 24) + (rdata[i + 1] << 16) + (rdata[i + 2] << 8) + rdata[i + 3]);
    strcat(addr, temp);
    addr_length += strlen(temp);
    i += 4;
    
    return addr;
}