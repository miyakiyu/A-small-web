#include "resolver.c"

int main(int argc, char *argv[]){
    //TYPE A, show Address
    struct resolver A;
    resolver_setup(&A, argv[1], atoi(argv[2]), argv[3], CLASS_IN, TYPE_A);
    resolver_UDP_send_recv(&A);
    resolver_build_rr(&A);
    resolver_print(A);

    //TYPE NS, show Name Server
    struct resolver NS;
    resolver_setup(&NS, argv[1], atoi(argv[2]),argv[3], CLASS_IN, TYPE_NS);
    resolver_UDP_send_recv(&NS);
    resolver_build_rr(&NS);
    resolver_print(NS);

    //TODO
    //TYPE SOA, acturaly we need the server version
    struct resolver SOA;
    resolver_setup(&SOA, argv[1], atoi(argv[2]), argv[3], CLASS_IN, TYPE_SOA);
    resolver_UDP_send_recv(&SOA);
    resolver_build_rr(&SOA);
    resolver_print(SOA);
}