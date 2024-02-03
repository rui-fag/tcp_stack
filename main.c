#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include "udp.h"
#include "ipv6.h"
#include "ethernetII.h"
#include "data.h"
#include "frame_check.h"

#define IPV4 0x0800
#define ARP  0x0806
#define IPV6 0x86DD

void build_structs(ethernetII *, ipv6 *, udp *, uint8_t *, size_t, frame_check *);
int tun_alloc(char *, int);
size_t build_datagram(ethernetII, ipv6, udp, uint8_t *, size_t, uint8_t *, size_t);

int main(void)
{

	/* tunclient.c */
	int tap_fd;
	char tun_name[IFNAMSIZ];
	int nwrite;

	/* Connect to the device */
	strcpy(tun_name, "tap0");
	tap_fd = tun_alloc(tun_name, IFF_TAP | IFF_NO_PI);//| IFF_MULTI_QUEUE | IFF_NAPI);

	if (tap_fd < 0) {
		perror("Allocating interface");
		exit(1);
	}

	uint8_t buffer[2048] = {0};
	size_t buf_sz = sizeof(buffer);

	ethernetII link_layer = {0};
	ipv6 ip_layer = {0};
	udp transport_layer = {0};

	uint8_t data[] = "Algumas coisas já estão a funcionar";
	size_t data_sz = sizeof(data);

	frame_check frame = {0};

	build_structs(&link_layer, &ip_layer, &transport_layer, data, data_sz, &frame);
	
	uint8_t *ethernet_ptr = (uint8_t*)build_ethernetII(&link_layer, &buffer[0], buf_sz);

	uint8_t *ip_ptr = (uint8_t*) build_ipv6(&ip_layer, ethernet_ptr, buf_sz);	

	uint8_t *transport_ptr = (uint8_t*)build_udp(&transport_layer, ip_ptr, buf_sz);

	uint8_t *data_ptr = (uint8_t*)build_data(&data, sizeof(data), transport_ptr, buf_sz);

	uint8_t *frame_check_ptr = (uint8_t*)build_frame_check(&frame, data_ptr, buf_sz);
	//size_t datagram_sz = build_datagram(link_layer, ip_layer, transport_layer, data, data_sz, buffer, buf_sz);

	size_t dataframe_sz = frame_check_ptr - &buffer[0];

	nwrite = write(tap_fd, &buffer[0], dataframe_sz);
	if (nwrite < 0)
	{
		perror("writing data");
	}
	printf("Write %d bytes\n", nwrite);
}

int tun_alloc(char *dev, int flags) {

  struct ifreq ifr;
  int fd, err;
  char *clonedev = "/dev/net/tun";

  if( (fd = open(clonedev , O_RDWR)) < 0 ) {
    perror("Opening /dev/net/tun");
    return fd;
  }

  memset(&ifr, 0, sizeof(ifr));

  ifr.ifr_flags = flags;

  if (*dev) {
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
  }

  if( (err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0 ) {
    perror("ioctl(TUNSETIFF)");
    close(fd);
    return err;
  }

  strcpy(dev, ifr.ifr_name);

  return fd;
}

void build_structs(ethernetII *link_layer, ipv6 *ip_layer, udp *transport_layer, uint8_t *data, size_t data_sz, frame_check *frame)
{
//ETHERNETII
	for(size_t i = 0; i < 6;i++)
		link_layer->destination_mac_address[i] = i;
	
	for(size_t i = 0; i < 6;i++)
		link_layer->source_mac_address[i] = i + 20;

	link_layer->ethernet_type = IPV6;
//IPV6
	ip_layer->version = 0x6;
	ip_layer->traffic_class = 0;
	ip_layer->flow_label = 0;
	ip_layer->payload_length = sizeof(udp) + data_sz;
	ip_layer->next_header = 0x11;
	ip_layer->hop_limit = 0x40;

	for(size_t i = 0; i < 8; i++)
		ip_layer->source_address[i] = 0x0001 + i;
	
	for(size_t i = 0; i < 8; i++)
		ip_layer->destination_address[i] = 0x1847 + i;
//UDP
	transport_layer->source_port = 0x8081;
	transport_layer->destination_port = 0x1011;
	transport_layer->length = sizeof(udp) + data_sz;
//Checksum	
      	uint32_t ip_sum	= 0;

	for(size_t i = 0; i < 8; i++)
	{
		ip_sum += ip_layer->source_address[i];
		ip_sum += ip_layer->destination_address[i];
	}
	ip_sum += ip_layer->next_header;
	ip_sum += ip_layer->payload_length;

	uint32_t transport_sum = 0;

	transport_sum = transport_layer->source_port + transport_layer->destination_port + transport_layer->length;

	uint32_t data_sum = 0;

	for(size_t i = 0; i < data_sz; i += 2)
	{
		if(i + 1 >= data_sz)
		{
			uint16_t data_word = 0; 

			data_word = (uint16_t)(data[i] << 8);
			data_word |= 0;
			data_sum += data_word;
		}
		else
		{
			uint16_t data_word = 0; 

			data_word = (uint16_t)(data[i] << 8);
			data_word |= (uint16_t)(data[i + 1]);
			data_sum += data_word;
		}
	}

	uint32_t total_sum = data_sum + transport_sum + ip_sum;

	total_sum += (total_sum & 0x00FF0000) >> 16;

	transport_layer->checksum = compute_checksum_ipv6(ip_layer, transport_layer, data, data_sz); 

	frame->frame_check = 0x32435645;
}

size_t build_datagram(ethernetII ethernet_layer, ipv6 ip_layer, udp transport_layer, uint8_t *data, size_t data_sz, uint8_t buffer[], size_t buf_sz)
{
	(void)buf_sz;
//fill ethII	
	size_t mac_sz = sizeof(ethernet_layer.destination_mac_address);

	memcpy(&buffer[0], &ethernet_layer.destination_mac_address, mac_sz);
	memcpy(&buffer[mac_sz], &ethernet_layer.source_mac_address, mac_sz);
	buffer[2 * mac_sz] = (uint8_t)((ethernet_layer.ethernet_type & 0xFF00) >> 8);	
	buffer[2 * mac_sz + 1] = (uint8_t)(ethernet_layer.ethernet_type & 0x00FF);	
//fill ipv6
	size_t ethernet_layer_sz = sizeof(ethernet_layer);
	size_t ip_layer_sz = 4 * 10;

	uint8_t *ipv6_buffer_base = &buffer[ethernet_layer_sz];

	ipv6_buffer_base[0] = (uint8_t)ip_layer.version << 4;
	ipv6_buffer_base[0] |= 0x0F & (uint8_t)((ip_layer.traffic_class & 0xF0) >> 4);
	ipv6_buffer_base[1] = 0xF0 & (uint8_t)((ip_layer.traffic_class & 0x0F) << 4);
	ipv6_buffer_base[1] |= 0x0F & (uint8_t)((ip_layer.flow_label & 0xF0000) >> 16);
	ipv6_buffer_base[2] = (uint8_t)((ip_layer.flow_label & 0x0FF00) >> 8);
	ipv6_buffer_base[3] = (uint8_t)(ip_layer.flow_label & 0x000FF);
	ipv6_buffer_base[4] = (uint8_t)((ip_layer.payload_length & 0xFF00) >> 8);
	ipv6_buffer_base[5] = (uint8_t)(ip_layer.payload_length & 0x00FF);
	ipv6_buffer_base[6] = ip_layer.next_header;
	ipv6_buffer_base[7] = ip_layer.hop_limit;

	size_t index = 0;

	for(size_t i = 8; i < 8 + 4 * 4; i += 4)
	{
			ipv6_buffer_base[i] = (uint8_t)((ip_layer.source_address[index] & 0xFF00) >> 8);
			ipv6_buffer_base[i + 1] = (uint8_t)(ip_layer.source_address[index] & 0x00FF);
			index++;
			ipv6_buffer_base[i + 2] = (uint8_t)((ip_layer.source_address[index] & 0xFF00) >> 8);
			ipv6_buffer_base[i + 3] = (uint8_t)(ip_layer.source_address[index] & 0x00FF);
			index++;
	}
	
	index = 0;

	for(size_t i = 8 + 4 * 4; i < 8 + 4 * 4 + 4 * 4; i += 4)
	{
			ipv6_buffer_base[i] = (uint8_t)((ip_layer.destination_address[index] & 0xFF00) >> 8);
			ipv6_buffer_base[i + 1] = (uint8_t)(ip_layer.destination_address[index] & 0x00FF);
			index++;
			ipv6_buffer_base[i + 2] = (uint8_t)((ip_layer.destination_address[index] & 0xFF00) >> 8);
			ipv6_buffer_base[i + 3] = (uint8_t)(ip_layer.destination_address[index] & 0x00FF);
			index++;
	}
	
//fill udp
	uint8_t *udp_buffer_base = &buffer[ethernet_layer_sz + ip_layer_sz];
	size_t transport_layer_sz = sizeof(transport_layer);

	udp_buffer_base[0] = (uint8_t)((transport_layer.source_port & 0xFF00) >> 8);
	udp_buffer_base[1] = (uint8_t)(transport_layer.source_port & 0x00FF);
	udp_buffer_base[2] = (uint8_t)((transport_layer.destination_port & 0xFF00) >> 8);
	udp_buffer_base[3] = (uint8_t)(transport_layer.destination_port & 0x00FF);
	udp_buffer_base[4] = (uint8_t)((transport_layer.length & 0xFF00) >> 8);
	udp_buffer_base[5] = (uint8_t)(transport_layer.length & 0x00FF);
	udp_buffer_base[6] = (uint8_t)((transport_layer.checksum & 0xFF00) >> 8);
	udp_buffer_base[7] = (uint8_t)(transport_layer.checksum & 0x00FF);

	uint8_t *data_buffer_base = &buffer[ethernet_layer_sz + ip_layer_sz + transport_layer_sz];

	memcpy(data_buffer_base, data, data_sz);

	uint8_t *chksm_buffer_base = data_buffer_base + data_sz;

	chksm_buffer_base[0] = 0x24;
	chksm_buffer_base[1] = 0x76;
	chksm_buffer_base[2] = 0x98;
	chksm_buffer_base[3] = 0x15;
	
	return data_sz + ethernet_layer_sz + ip_layer_sz + transport_layer_sz + 4;
}
