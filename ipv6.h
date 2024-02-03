#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#ifndef __IP_H__
#define __IP_H__

#define IPV6_HDR_SZ 40

typedef struct
{
	unsigned version:4;
	uint8_t traffic_class;
	unsigned flow_label:20;
	uint16_t payload_length;
	uint8_t next_header;
	uint8_t hop_limit;
	uint16_t source_address[8];
	uint32_t destination_address[8];
}ipv6;

void *build_ipv6(ipv6*, void *, size_t); 

#endif
