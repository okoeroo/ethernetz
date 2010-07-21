//---------------------------------------------------------------------
//	nicwatch.cpp	
//	
//	This program demonstrates the promiscuous reception of raw
//	ethernet packets using the Linux PF_PACKET protocol family.
//	Its screen-output may be redirected to a file.  A user hits
//	the <CONTROL-C> key-combination to terminate its execution. 
//	Alternatively the user may specify the number of packets to
//	accept using the '-c' command-line switch.  The name for an
//	interface must be supplied by the user on the command-line. 
//
//		usage:  $ ./nicwatch <ifname> [-c <packet-limit>]
//		e.g.:	$ ./nicwatch eth0 -c 4
//
//	This program normally requires 'superuser' privileges (but
//	it may be executed by students on USF's 'anchor' cluster).
//
//	REFERENCE: Gianluca Insolvibile, "The Linux Socket Filter:
//	Sniffing Bytes over the Network," article in Linux Journal
//	(June 2001), pp. 26-31.
//
//	programmer: ALLAN CRUSE
//	written on: 21 JAN 2008
//---------------------------------------------------------------------

#include <stdio.h>		// for printf(), perror()
#include <string.h>		// for strncpy()
#include <stdlib.h>		// for exit()
#include <signal.h>		// for signal()
#include <unistd.h>		// for gethostname()
#include <sys/socket.h>		// for socket(), recvfrom()
#include <sys/ioctl.h>		// for SIOCGIFFLAGS, SIOCSIFFLAGS
#include <netinet/in.h>		// for htons()
#include <net/if.h>		// for 'struct ifreq', 'struct ifconf'
#include <linux/if_ether.h>	// for ETH_P_ALL
#include <linux/if_packet.h>	// for 'struct sockaddr_ll'

#include <errno.h>
#include <arpa/inet.h>

#include <sys/ioctl.h>

#include <sys/time.h>


#define MAXPKT	0x2400		// Maximum packet-length (jumbo)
#define MAXDEV	16		// Maximum number of interfaces

struct ifconf	ifc;		// structure for ifconfig request
struct ifreq	ifr[ MAXDEV ];	// structures for interface config 
struct ifreq	ethreq;		// structure for 'ifflags' request
char devname[ IFNAMSIZ ];	// name for the network interface
int 	ifindex;		// interface-index for the device
int	packet_limit = 100;	// default limit on packet-count
const char *pktype[] = { "incoming", "broadcast", "multicast", "otherhost",
			"outgoing", "loopback", "fastroute", "??" };	


int get_netdev_index (int sock, int argc, char *argv[])
{
	int i = 0,n = 0;

	// get the list of current network interface structures
	ifc.ifc_buf = (char*)&ifr;
	ifc.ifc_len = sizeof( ifr );
	if ( ioctl( sock, SIOCGIFCONF, &ifc ) < 0 )
		{ perror( "SIOCGIFCONF" ); exit(1); }

	// obtain the interface-index for each detected device
	int	sz = sizeof( struct ifreq );
	int	dev_count = ifc.ifc_len / sz;
	for (n = 0; n < dev_count; n++)
		if ( ioctl( sock, SIOCGIFINDEX, &ifr[ n ] ) < 0 )
			{ perror( "SIOCGIFINDEX" ); exit(1); }

	// scan command-line arguments for user's desired interface
	for (n = 0; n < dev_count; n++)
		for (i = 1; i < argc; i++)
			if ( strcmp( argv[i], ifr[n].ifr_name ) == 0 )
				{
				strcpy( devname, ifr[n].ifr_name );
				ifindex = ifr[n].ifr_ifindex;
				break;
				}

	// cannot continue if user omitted interface-name argument
	if ( !ifindex ) 
		{
		fprintf( stderr, "You must specify an interface: " );
		for (n = 0; n < dev_count; n++)
			fprintf( stderr, " %s ", ifr[n].ifr_name );
		fprintf( stderr, "\n" );
		exit(1);
		}

	return ifindex;
}


int main( int argc, char *argv[] )
{
    /*target address*/
    struct sockaddr_ll socket_address;

    /*buffer for ethernet frame*/
    void* buffer = (void*)malloc(ETH_FRAME_LEN);
     
    /*pointer to ethenet header*/
    unsigned char* etherhead = buffer;
        
    /*userdata in ethernet frame*/
    unsigned char* data = buffer + 14;
        
    /*another pointer to ethernet header*/
    struct ethhdr *eh = (struct ethhdr *)etherhead;
     
    int send_result = 0;

    /*our MAC address*/ /* eth0: 00:1B:21:5D:35:14 */
    unsigned char src_mac[6] = {0x00, 0x1B, 0x21, 0x5D, 0x35, 0x14};

    /*other host MAC address*/
    unsigned char dest_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


	int i, j, n;
	int	sock, pkt_num;		// for socket-ID and packet-number 

	// allow user to modify the default number of packets to accept
	for (i = 1; i+1 < argc; i++)
		if ( strncmp( argv[ i ], "-c", 3 ) == 0 )
			packet_limit = atoi( argv[ i+1 ] );

	// create an unnamed socket for reception of ethernet packets 
	sock = socket( PF_PACKET, SOCK_RAW, htons( ETH_P_ALL ) ); 
	if ( sock < 0 ) { perror( "socket" ); exit( 1 ); }


	printf ("Foo! Index is: %d\n", get_netdev_index (sock, argc, argv));



    /*prepare sockaddr_ll*/

    /*RAW communication*/
    socket_address.sll_family   = PF_PACKET;    
    /*we don't use a protocoll above ethernet layer
      ->just use anything here*/
    socket_address.sll_protocol = htons(ETH_P_ALL);  

    /*index of the network device
    see full code later how to retrieve it*/
    socket_address.sll_ifindex = get_netdev_index (sock, argc, argv);
    printf ("Selected device index is: %d\n", get_netdev_index (sock, argc, argv));

    /*ARP hardware identifier is ethernet*/
/*
    socket_address.sll_hatype   = ARPHRD_ETHER;
*/
        
    /*target is another host*/
    socket_address.sll_pkttype  = PACKET_OTHERHOST;

    /*address length*/
    socket_address.sll_halen    = ETH_ALEN;     
    /*MAC - begin*/
    socket_address.sll_addr[0]  = 0xFF;     
    socket_address.sll_addr[1]  = 0xFF;     
    socket_address.sll_addr[2]  = 0xFF;
    socket_address.sll_addr[3]  = 0xFF;
    socket_address.sll_addr[4]  = 0xFF;
    socket_address.sll_addr[5]  = 0xFF;
    /*MAC - end*/
    socket_address.sll_addr[6]  = 0x00;/*not used*/
    socket_address.sll_addr[7]  = 0x00;/*not used*/


    /*set the frame header*/
    memcpy((void*)buffer, (void*)dest_mac, ETH_ALEN);
    memcpy((void*)(buffer+ETH_ALEN), (void*)src_mac, ETH_ALEN);
    eh->h_proto = 0x00;
    /*fill the frame with some data*/
    for (j = 0; j < 1500; j++) {
        data[j] = (unsigned char)((int) (255.0*rand()/(RAND_MAX+1.0)));
    }


    /*send the packet*/

    while (1)
    {
	    send_result = sendto(sock, buffer, ETH_FRAME_LEN, 0, 
		      (struct sockaddr*)&socket_address, sizeof(socket_address));
	    if (send_result == -1) 
	    { 
		fprintf (stderr, "Error in sendto(): %s\n", strerror(errno));
		/* exit (1); */
	    }
    }



#if 0

	// main loop to intercept and display the ethernet packets
	char	hostname[ 64 ], packet[ MAXPKT ];
	gethostname( hostname, sizeof( hostname ) );
	printf( "\nMonitoring network packets for interface \'%s\'", devname );
	printf( " on station \'%s\' \n", hostname );
	do	{
		struct sockaddr_ll	saddr = { 0 };
		socklen_t		slen = sizeof( saddr );
		size_t			plen = sizeof( packet );
		int			nbytes;

		nbytes = recvfrom( sock, packet, MAXPKT, 0, 
					(sockaddr*)&saddr, &slen );

		if ( saddr.sll_ifindex == ifindex )
			display_packet( packet, nbytes, saddr.sll_pkttype );
		}
	while ( pkt_num < packet_limit );
#endif
}

