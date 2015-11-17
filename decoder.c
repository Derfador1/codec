// fp = open()
//read()


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define SIZE 255


int hexDump(void *buf, int len);
int printtofile(void *buf, int len);


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

struct meditrik {
	unsigned int version : 4;
	unsigned int seq_id : 9;
	unsigned int type : 3;
	unsigned int total_length : 16;
	unsigned int source_device_id : 32;
	unsigned int dest_device_id : 32;

};



int main(void)
{

	int descrip = open("hello.pcap", O_RDONLY);

	if (descrip == -1)
	{
		fprintf(stderr, "Error could not open file\n");
		exit(1);
	}

	unsigned char *buf;

	buf = malloc(SIZE);

	read(descrip, buf, 255);

	int c = 106;
	
	hexDump(buf, c);

	printtofile(buf, c);

	free(buf);

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
