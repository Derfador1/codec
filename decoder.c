#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define SIZE 255




//pull info from bits
//ver = >> 4
//type = >> 5
//seq_id = >> 3 , and by mask


//function to check values based on dec number


/* Use this site as a starting point to seperate the fields to lower numbers based on fields given one this site
https://wiki.wireshark.org/Development/LibpcapFileFormat
*/

struct global{
	unsigned int magic_number : 32;
	unsigned int version_major : 16; 
	unsigned int version_minor : 16; 
	unsigned int time_zone: 32; 
	unsigned int sig_flags : 32; 
	unsigned int snap_len : 32; 
	unsigned int network : 32;  

};

struct packet_header{
	unsigned int ts_sec : 32;
	unsigned int ts_usec : 32;
	unsigned int incl_len : 32;
	unsigned int orig_len : 32;
};

struct ethernet{
	//had to split, no room to hold 48 bits in one var
	unsigned int destmac : 32;
	unsigned int destmac_split : 16;
	unsigned int sourcemac : 32;
	unsigned int sourcemac_split : 16;
	unsigned int length : 16;
};

struct ipv4{
	unsigned int version : 4;
	unsigned int h_length : 4;
	unsigned int s_type : 8;
	unsigned int total_length : 16;
	unsigned int identification : 16;
	unsigned int flags : 4;
	unsigned int offset : 8;
	unsigned int ttl : 4;
	unsigned int protocol : 4;
	unsigned int checksum : 16;
	unsigned int source_ip : 32;
	unsigned int dest_ip : 32;
	unsigned int option : 12;
};

struct udp{
	unsigned int s_port : 16;
	unsigned int d_port : 16;
	unsigned int length : 16;
	unsigned int checksum : 16;
};

struct meditrik{
	unsigned int version : 4;
	unsigned int seq_id : 9;
	unsigned int type : 3;
	unsigned int total_length : 16;
	unsigned int source_device_id : 32;
	unsigned int dest_device_id : 32;

};

struct meditrik *make_meditrik(void)
{
	struct meditrik *meditrik = malloc(sizeof(struct meditrik));
	if(!meditrik) {
		return NULL;
	}

	return meditrik;
}


int hexDump(void *buf, int len);
int printtofile(void *buf, int len);
int bit_seperation(struct meditrik *medi, unsigned char * buf, unsigned int *type_pt);
int field_check(unsigned int *type_pt, unsigned char * buf);


int main(void)
{
	int count = 0;

	unsigned int *type_pt = malloc(sizeof(*type_pt));

	int descrip = open("command_glucose.pcap", O_RDONLY);

	if (descrip == -1)
	{
		fprintf(stderr, "Error could not open file\n");
		exit(1);
	}

	unsigned char *buf;

	buf = malloc(SIZE);

	count = read(descrip, buf, SIZE);

	printf("%d", count);

	//int c = 106; //change with count
	
	hexDump(buf, count);

	struct meditrik *stuff = make_meditrik();

	bit_seperation(stuff, buf, type_pt);

	//printf("%u\n", *type_pt);

	field_check(type_pt, buf);

	printtofile(buf, count);

	free(buf);

	free(stuff);
	
	free(type_pt);

	close(descrip);
}

int hexDump(void *buf, int len)
{
	int count = 0;
	int start = 82;

	unsigned char * buffer = buf;	

	for (count = 0; count < len; count++)
	{
		if (count % 16 == 0) {
			printf("\n");
		}
		else if (count % 8 == 0) { 
			printf(" ");
		}
		printf ("%02x ", buffer[count]);
	}
	printf("\n\n");


	for (count = start; count < len; count++)
	{
		if ((count % 16) == 0) {
			printf("\n");
		}
		printf ("%02x ", buffer[count]);
	}
	printf("\n\n");

	return 1;
}


int printtofile(void * buf, int len)
{
	FILE *fp;
	fp = fopen("decoded.txt", "w");

	int c = 0;

	unsigned char * buffer = buf;

	for (c = 0; c < len; c++)
	{
		if ((c % 16) == 0) {
			fprintf(fp, "\n");
		}
		fprintf(fp, "%02x ", buffer[c]);
	}

	fclose(fp);

	return 1;
}


int bit_seperation(struct meditrik *medi, unsigned char * buf, unsigned int *type_pt)
{
	//version bitmath
	unsigned int byte_start = buf[82];
	byte_start >>= 4;
	medi->version = byte_start;
	printf("Version: %d\n", medi->version);

	//sequence_id bitmath
	byte_start = buf[82];
	byte_start &= 15;
	byte_start <<= 5;
	unsigned int byte_start2 = buf[83];
	byte_start2 >>= 3;
	byte_start += byte_start2;
	medi->seq_id = byte_start;
	printf("Sequence: %d\n", medi->seq_id);

	//type bitmath
	unsigned char byte_starter = buf[83];
	byte_starter &= 7;
	medi->type = byte_starter;
	printf("Type: %d\n", medi->type);
	*type_pt = medi-> type;

	//source device id bitmath
	unsigned int byte_start_source = buf[86];
	byte_start_source <<= 8;
	byte_start_source += buf[87];
	byte_start_source <<= 8;
	byte_start_source += buf[88];
	byte_start_source <<= 8;
	byte_start_source += buf[89];
	medi->source_device_id = byte_start_source;
	printf("Source Device: %d\n", medi->source_device_id);

	//dest device id bitmath
	unsigned int byte_start_dest = buf[90];
	byte_start_dest <<= 8;
	byte_start_dest += buf[91];
	byte_start_dest <<= 8;
	byte_start_dest += buf[92];
	byte_start_dest <<= 8;
	byte_start_dest += buf[93];
	medi->dest_device_id = byte_start_dest;
	printf("Destination Device: %d\n", medi->dest_device_id);

	return 0;
}


int field_check(unsigned int *type_pt, unsigned char * buf)
{
	if (*type_pt == 0)
	{
		printf("GET STATUS(0)\n");
	}
	else if (*type_pt == 1)
	{
		unsigned int byte_start = buf[94];

		byte_start <<= 8;

		byte_start += buf[95];

		if (byte_start == 0)
		{
			printf("GET STATUS(0)\n");
		}
		else if (byte_start == 1)
		{
			unsigned int value = buf[96];
			value <<= 8;
			value += buf[97];
			printf("Glucose set to: %d\n", value);
		}
	}
	else if (*type_pt == 2)
	{
		printf("%c\n", buf[84]);
	}
	else if (*type_pt == 3)
	{
		printf("%c\n", buf[84]);
	}

	return 0;
}
