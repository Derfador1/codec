#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define SIZE 254

//darn you commit

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

struct gps {
	double longs;
	double lat;
	float alt;
};

union battery {
	unsigned char tempbuf[8];
	double percent;
};

union gps_header{
	struct gps fields;
	unsigned char degrees[20];
};

int hexDump(void *buf, int len);

int bit_seperation(FILE *write, struct meditrik *medi, unsigned char *buf, unsigned int *type_pt, unsigned int *total_length, int *start);

int field_check(FILE *write, unsigned int *type_pt, unsigned char *buf, int *start, unsigned int *total_length);


int main(int argc, char * argv[])
{
	int descrip = open(argv[1], O_RDONLY);

	if (argc == 1)
	{
		printf("Please retry with a valid file to open.\n");
		exit(1);
	}
	else if (argc >= 2)
	{

		if (descrip == -1)
		{
			fprintf(stderr, "Error could not open file\n");
			exit(1);
		}
		else
		{
			printf("You successfully opened %s\n", argv[1]);
		}
	}

	int count = 0;

	int excess_headers = 58;

	int global_header = 24;

	unsigned int *type_pt = malloc(sizeof(*type_pt));

	unsigned int *total_length = malloc(sizeof(*total_length));

	int *start = malloc(sizeof(*start));

	unsigned char *buf = malloc(SIZE);

	memset(buf, '\0', SIZE);

	count = read(descrip, buf, SIZE);

	printf("Count of bytes: %d", count);
	
	if (hexDump(buf, count) != 1)
	{
		fprintf(stderr, "Something wrong happened in hexdump function\n");
	}

	struct meditrik *stuff = make_meditrik();
	
	FILE *write;
	write = fopen("decoded.txt", "w");

	*start = global_header + excess_headers;

	while(*start < count)
	{
		bit_seperation(write, stuff, buf, type_pt, total_length, start);

		field_check(write, type_pt, buf, start, total_length);
	}

	free(buf);

	free(stuff);
	
	free(type_pt);

	free(total_length);

	free(start);

	close(descrip);

	fclose(write);
}

int hexDump(void *buf, int len)
{
	int counter = 0;
	int start = 82;

	unsigned char * buffer = buf;	

	if (start >= len)
	{
		return 0;
	}


	for (counter = 0; counter < len; counter++)
	{
		if (counter % 16 == 0) {
			printf("\n");
		}
		else if (counter % 8 == 0) { 
			printf(" ");
		}
		printf ("%02x ", buffer[counter]);
	}
	printf("\n\n");


	for (counter = start; counter < len; counter++)
	{
		if ((counter % 16) == 0) {
			printf("\n");
		}
		printf ("%02x ", buffer[counter]);
	}
	printf("\n\n");

	return 1;
}

int bit_seperation(FILE *write, struct meditrik *medi, unsigned char *buf, unsigned int *type_pt, unsigned int *total_length, int *start)
{
	//version bitmath
	unsigned int byte_start = buf[*start];
	byte_start >>= 4;
	medi->version = byte_start;
	fprintf(stdout, "Version: %d\n", medi->version);
	fprintf(write, "Version: %d\n", medi->version);

	//sequence_id bitmath
	byte_start = buf[*start];
	byte_start &= 15;
	byte_start <<= 5;
	unsigned int byte_start2 = buf[++(*start)];
	byte_start2 >>= 3;
	byte_start += byte_start2;
	medi->seq_id = byte_start;
	fprintf(stdout, "Seq _ Id: %d\n", medi->seq_id);
	fprintf(write, "Sequence: %d\n", medi->seq_id);

	//type bitmath
	unsigned char byte_starter = buf[*start];
	byte_starter &= 7;
	medi->type = byte_starter;
	fprintf(stdout, "Type: %d\n", medi->type);
	fprintf(write, "Type: %d\n", medi->type);
	*type_pt = medi-> type;

	//total length
	unsigned char byte_length_starter = buf[++(*start)];
	byte_length_starter <<= 8;
	byte_length_starter += buf[++(*start)];
	medi->total_length = byte_length_starter;
	fprintf(stdout, "Total Length: %d\n", medi->total_length);
	fprintf(write, "Total Length: %d\n", medi->total_length);
	*total_length = medi-> total_length;

	//source device id bitmath
	unsigned int byte_start_source = buf[++(*start)];
	byte_start_source <<= 8;
	byte_start_source += buf[++(*start)];
	byte_start_source <<= 8;
	byte_start_source += buf[++(*start)];
	byte_start_source <<= 8;
	byte_start_source += buf[++(*start)];
	medi->source_device_id = byte_start_source;
	fprintf(stdout, "S Device Id: %d\n", medi->source_device_id);
	fprintf(write, "Source Device: %d\n", medi->source_device_id);

	//dest device id bitmath
	unsigned int byte_start_dest = buf[++(*start)];
	byte_start_dest <<= 8;
	byte_start_dest += buf[++(*start)];
	byte_start_dest <<= 8;
	byte_start_dest += buf[++(*start)];
	byte_start_dest <<= 8;
	byte_start_dest += buf[++(*start)];
	medi->dest_device_id = byte_start_dest;
	fprintf(stdout, "D Device Id: %d\n", medi->dest_device_id);
	fprintf(write, "Destination Device: %d\n", medi->dest_device_id);

	(*start)++;

	return 0;
}


int field_check(FILE *write, unsigned int *type_pt, unsigned char *buf, int *start, unsigned int *total_length)
{
	
	//maybe they all need to be shorts
	short glucose = 0;
	int capsaicin = 0;
	int omorfine = 0;
	int counter = 0;

	int excess_headers = 58;
	int meditrik_header = 12;

	*total_length = *total_length - meditrik_header;


	if (*type_pt == 0)
	{
		//function //pass *start, buffer, move 4 values glucose etc.. to 
		printf("Status of Device\n");

		union battery power;

		for (counter = 0; counter < 8; counter++)
		{
			power.tempbuf[counter] = buf[*start + counter];
		}

		*start = *start + counter;

		fprintf(stdout, "Battery power : %.2f%%\n", power.percent * 100);
		fprintf(write, "Battery power : %.2f%%\n", power.percent * 100);

		unsigned int glucose_start = buf[*start]; //102
		glucose_start <<= 8;
		glucose_start += buf[++(*start)]; // 103
		glucose = glucose_start;
		fprintf(write, "Glucose: %d\n", glucose);
		fprintf(stdout, "Glucose: %d\n", glucose);

		unsigned int capsaicin_start = buf[++(*start)];
		capsaicin_start <<= 8;
		capsaicin_start += buf[++(*start)];
		capsaicin = capsaicin_start;
		fprintf(write, "Capsaicin: %d\n", capsaicin);
		fprintf(stdout, "Capsaicin: %d\n", capsaicin);

		unsigned int omorfine_start = buf[++(*start)];
		omorfine_start <<= 8;
		omorfine_start += buf[++(*start)];
		omorfine = omorfine_start;
		fprintf(write, "Omorfine: %d\n", omorfine);
		fprintf(stdout, "Omorfine: %d\n", omorfine);

		(*start)++;

		*start = *start + excess_headers;

		return 0;

	}
	else if (*type_pt == 1)
	{
		unsigned int byte_start = buf[*start];
		byte_start <<= 8;
		byte_start += buf[++(*start)];
		fprintf(write, "Command: %d\n", byte_start);
		if (byte_start == 0)
		{
			printf("GET STATUS(0)\n");
		}
		else if (byte_start == 1)
		{
			unsigned int glucose = buf[++(*start)];
			glucose <<= 8;
			glucose += buf[++(*start)];
			fprintf(write, "Glucose: %d\n", glucose);
			fprintf(stdout, "Glucose: %d\n", glucose);
		}
		else if (byte_start == 2)
		{
			printf("Request GPS packet\n");
		}
		else if (byte_start == 3)
		{
			unsigned int capsaicin = buf[*start];
			capsaicin <<= 8;
			capsaicin += buf[++(*start)];
			fprintf(write, "Capsaicin: %d\n", capsaicin);
			fprintf(stdout, "Capsaicin: %d\n", capsaicin);
		}
		else if (byte_start == 4)
		{
			printf("Reserved, GET OUT OF HERE\n");
		}
		else if (byte_start == 5)
		{
			unsigned int omorfine = buf[*start];
			omorfine <<= 8;
			omorfine += buf[++(*start)];
			fprintf(write, "Omorfine: %d\n", omorfine);
			fprintf(stdout, "Omorfine: %d\n", omorfine);
		}
		else if (byte_start == 6)
		{
			printf("Reserved, GET OUT OF HERE\n");
		}
		else if (byte_start == 7)
		{	
			unsigned int sequence_id = buf[*start];
			sequence_id <<= 8;
			sequence_id += buf[++(*start)];	
			fprintf(write, "Seq_param: %d\n", sequence_id);
			fprintf(stdout, "Seq_param: %d\n", sequence_id);
		}

		(*start)++;

		*start = *start + excess_headers;

		return 1;
	}
	else if (*type_pt == 2)
	{
		union gps_header gps;
		
		for (counter = 0; counter < 20; counter++)
		{
			gps.degrees[counter] = buf[*start + counter];
		}

		*start = *start + counter;

		fprintf(stdout, "Longitude: %.9f degree W\n", gps.fields.longs);
		fprintf(write, "Longitude: %.9f degree W\n", gps.fields.longs);

		fprintf(stdout, "Latitude: %.9f degree N\n", gps.fields.lat);
		fprintf(write, "Latitude: %.9f degree N\n", gps.fields.lat);

		fprintf(stdout, "Altitude: %.0f ft\n", gps.fields.alt * 6);
		fprintf(write, "Altitude: %.0f ft\n", gps.fields.alt * 6);
		
		(*start)++;

		*start = *start + excess_headers;

		return 2;

	}
	else if (*type_pt == 3)
	{
		int i = 0;

		int *counter = malloc(sizeof(*counter));

		*counter = *start + *total_length;

		printf("Counter : %d\n", *counter);

		fprintf(stdout, "Message: ");
		fprintf(write, "Message: ");

		for (i = *start; i < *counter; i++)
		{
			fprintf(stdout, "%c", buf[i]);
			fprintf(write, "%c", buf[i]);
		}
		printf("\n");

		*start = *start + *total_length;

		*start = *start + excess_headers - 2;

		printf("%d\n", *start);

		free(counter);

		return 3;
	}
	return 4;
}
