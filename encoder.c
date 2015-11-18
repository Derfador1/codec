#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define SIZE 255

void dec_to_hex(unsigned int value, char * buf);

struct meditrik {
	//check mask
	unsigned int version : 4;
	unsigned int seq_id : 9;
	unsigned int type : 3;
	unsigned int total_length : 16;
	unsigned int source_device_id : 32;
	unsigned int dest_device_id : 32;

};

union bytes {
	struct meditrik medi;
	unsigned short data;	
};


int main(void)
{
	int j;

	char *buf;

	buf = malloc(13);

	memset(buf, '\0', 13);

	union bytes byte;

	byte.data = 0;

	byte.medi.version = 1;	
	byte.medi.seq_id = 81;
	byte.medi.type = 1;
	byte.medi.total_length = 18;
	byte.medi.source_device_id = 7890;
	byte.medi.dest_device_id = 1234;
	

	dec_to_hex(byte.medi.source_device_id, buf);

	FILE *fp;

	fp = fopen("test.pcap", "w+");

	for (j = 0; j < 12; j++)
	{
		fprintf(fp ,"%c", buf[j]);
	}

	/*
	for (j = 0; j < 12; j++)
	{
		printf("%c", buf[j]);
	}
	*/

	fclose(fp);

	free(buf);
	
}

//convert to string and print out string with for loop buf[i]
void dec_to_hex(unsigned int value, char * buf)
{
	//long int remainder;
	long int quotient;
	int i = 0, temp;

	quotient = value;

	while (quotient != 0)
	{
		temp = quotient % 16;

		/*
		if(temp < 10)
			temp = temp + 48;
		else
			temp = temp + 55;
		*/

		buf[i++] = temp;

		quotient = quotient/16;
	}

	//return 0;
	
}
