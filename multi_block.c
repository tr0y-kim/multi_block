#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/types.h>
#include <linux/netfilter.h>		/* for NF_ACCEPT */
#include <errno.h>
#include <string.h>

#include <netinet/ip.h>
// #include <netinet/ether.h>
#include <netinet/tcp.h>

#include <libnetfilter_queue/libnetfilter_queue.h>

#define IPTYPE 8
#define TCPTYPE 6

typedef u_int tcp_seq;

// Declare http_method & target (argv[1])
u_int8_t *target;
u_int8_t target_len;
u_int32_t id;
u_int8_t flag; // To identify ACCEPT, DROP


void iptables_F() {
	system("iptables -F");
}

void dump(unsigned char* buf, int size) {
    int i;
    for (i = 0; i < size; i++) {
        if (i % 16 == 0)
            printf("\n");
        printf("%02x ", buf[i]);
    }
	puts("");
}

// ---------------------------------------------------------------
static u_int32_t print_pkt (struct nfq_data *tb) {
	struct nfqnl_msg_packet_hdr *ph;
	struct nfqnl_msg_packet_hw *hwph;
	u_int32_t mark,ifi;
	int ret;
	unsigned char *data; // data for nfq_get_payload

	struct ip* ipHdr;
	struct tcphdr* tcpHdr;
	char * tcp_data_area;
	int ip_hdr_size;
	int tcp_hdr_size;
	int tcp_data_len;
	char * search_host;

	flag = 1;
	
	ph = nfq_get_msg_packet_hdr(tb);
	if(ph) id = ntohl(ph->packet_id); // id

	hwph = nfq_get_packet_hw(tb);
	if (hwph) {
		int i, hlen = ntohs(hwph->hw_addrlen);
	}

	mark = nfq_get_nfmark(tb);
	ifi = nfq_get_indev(tb);
	ifi = nfq_get_outdev(tb);
	ifi = nfq_get_physindev(tb);
	ifi = nfq_get_physoutdev(tb);
	ret = nfq_get_payload(tb, &data); // IP header의 시작 위치를 알아 낸다	
	if (ret >= 0) {
		printf("payload_len=%d ", ret);
		//dump(data, ret);
	} // finding IP header
	// Until this part, same as nfqtest!!!!!!!!
	int i =0;
	int size = ret;

	ipHdr = (struct ip *)data; //right
	ip_hdr_size = ipHdr->ip_hl * 4; //only ip header length
	//printf("[*] ipHdr size : %d\n", ip_hdr_size); // always 20 bytes
	//printf("[DUMP]ipHdr-----------");
	//dump((char *)ipHdr, ip_hdr_size);
	//printf("[ipHdr DUMP Finished]\n");

	// Now, let's find TCP Header & Data area! DUDE!!
	tcpHdr = (struct tcphdr *) (data + ip_hdr_size); // located at +20 
	tcp_hdr_size = (tcpHdr-> th_off * 4); // tcp header size
	tcp_data_area = data + ip_hdr_size + tcp_hdr_size;
	tcp_data_len = ret - ip_hdr_size - tcp_hdr_size; // ret is total length 

	if(ipHdr->ip_p == TCPTYPE && tcp_hdr_size > 0) {
		//printf("[*] tcpHdr size : %d\n",tcp_hdr_size);
		//printf("[DUMP]tcpHdr-----------");
		//dump((char *) tcpHdr, tcp_hdr_size);
		//printf("[tcpHdr DUMP Finished]\n");
		
		//printf("[*] tcp_data size : %d\n", tcp_data_len);
		//printf("[DUMP]tcp data area-----");
		//dump((char *) tcp_data_area, tcp_data_len);
		//printf("[tcp data area DUMP Finished]\n");
		//printf("%s\n", tcp_data_area);
		search_host = strstr(tcp_data_area, "Host: ");
		if(search_host != NULL) {
			// seg fault if it just print it!
			printf("****************ATTENTION***************\n");
			char *tmp;
			tmp = (char*)malloc(sizeof(char) * (6 + target_len)); // 6 for "Host: "
			strncpy(tmp, search_host, sizeof(char) * (6 + target_len));
			printf("%s\n", tmp);
			if (!strcmp(target,tmp+6)) {
				printf("[+] Same with the target DUDE!!!\n");
				flag = 0;
			}
			free(tmp);
		}
			
	}

	return id;
}


static int cb(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
	      struct nfq_data *nfa, void *data)
{
	u_int32_t id = print_pkt(nfa);
	printf("[*] entering callback\n");
	return nfq_set_verdict(qh, id, flag, 0, NULL); // flag: 1 ACCEPT, 0 DROP
}

int main(int argc, char *argv[]) {
    struct nfq_handle *h;			// struct nfnl_handle * 	nfnlh
									// struct nfq_q_handle * 	qh_list
									// struct nfnl_subsys_handle * 	nfnlssh
    struct nfq_q_handle *qh;
    struct nfnl_handle *nh;
    int fd;
    int rv;
    char buf[4096] __attribute__ ((aligned));

	if (argc != 2) {
		puts("You need argv[1]!");
		exit(1);
	}
	
	target = argv[1];
	target_len = strlen(target);

	// Initializing the iptables configuration
	system("iptables -F"); //iptables_F
	system("iptables -A OUTPUT -j NFQUEUE --queue-num 0");
	system("iptables -A INPUT -j NFQUEUE --queue-num 0");

    printf("opening library handle\n");
    h = nfq_open(); // handler of a netfilter queue
    
	if (!h) {
        fprintf(stderr, "error during nfq_open()\n"); iptables_F();
        exit(1);
    }

    printf("unbinding existing nf_queue handler for AF_INET (if any)\n");
    if (nfq_unbind_pf(h, AF_INET) < 0) {
        fprintf(stderr, "error during nfq_unbind_pf()\n"); iptables_F();
        exit(1);
    }

    printf("binding nfnetlink_queue as nf_queue handler for AF_INET\n");
    if (nfq_bind_pf(h, AF_INET) < 0) {
        fprintf(stderr, "error during nfq_bind_pf()\n"); iptables_F();
        exit(1);
    }

    printf("binding this socket to queue '0'\n");
    qh = nfq_create_queue(h,  0, &cb, NULL);
    if (!qh) {
        fprintf(stderr, "error during nfq_create_queue()\n"); iptables_F();
        exit(1);
    }

    printf("setting copy_packet mode\n");
    if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
        fprintf(stderr, "can't set packet_copy mode\n"); iptables_F();
        exit(1);
    }

    fd = nfq_fd(h);	// usage: nfq_fd(handler)

    for (;;) {
        if ((rv = recv(fd, buf, sizeof(buf), 0)) >= 0) {

            printf("[+] pkt received\n");
            nfq_handle_packet(h, buf, rv);
            continue;
        }
        if (rv < 0 && errno == ENOBUFS) {
            printf("[-] losing packets!\n");
            continue;
        }
        perror("[-] recv failed");
        break;
    }

    printf("unbinding from queue 0\n");
    nfq_destroy_queue(qh);

#ifdef INSANE
    /* normally, applications SHOULD NOT issue this command, since
     * it detaches other programs/sockets from AF_INET, too ! */
    printf("unbinding from AF_INET\n");
    nfq_unbind_pf(h, AF_INET);
#endif

    printf("closing library handle\n");
    nfq_close(h);
	iptables_F();
    exit(0);

}
