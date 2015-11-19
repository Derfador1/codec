#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#define SIZE 255


struct meditrik {
	unsigned int type : 3;
	unsigned int seq_id : 9;
	unsigned int version : 4;
	unsigned int total_length : 16;
	unsigned int source_device_id : 32;
	unsigned int dest_device_id : 32;
};

union bytes {
	struct meditrik medi;
	unsigned short data[6];
	unsigned int data2[3];
};


int main(int argc, char * argv[])
{
	if (argc == 1)
	{
		printf("whore\n");
	}

	union bytes byte;

	memset(byte.data2, '\0', sizeof(byte.data2));

	FILE *reader;
	int version = 0;
	int seq_id = 0;
	int type = 0;
	int total_length = 0;
	int source_device_id = 0;
	int dest_device_id = 0;

	reader = fopen("decoded.txt", "r");

	fscanf(reader, "Version: %d\n", &version);
	fscanf(reader, "Sequence: %d\n", &seq_id);
	fscanf(reader, "Type: %d\n", &type);
	fscanf(reader, "Total Length: %d\n", &total_length);
	fscanf(reader, "Source Device: %d\n", &source_device_id);
	fscanf(reader, "Destination Device: %d\n", &dest_device_id);

	byte.medi.version = version;	
	byte.medi.seq_id = seq_id;
	byte.medi.type = type;
	byte.medi.total_length = total_length;
	byte.medi.source_device_id = source_device_id;
	byte.medi.dest_device_id = dest_device_id;

	printf("%d\n", version);
	printf("%d\n", seq_id);
	printf("%d\n", type);
	printf("%d\n", total_length);
	printf("%d\n", source_device_id);
	printf("%d\n", dest_device_id);

	FILE *writer;

	writer = fopen(argv[1], "w+");


	char *fake_buffer = malloc(1);

	fake_buffer[0] = 0;

	byte.data[0] = htons(byte.data[0]);
	byte.data[1] = htons(byte.data[1]);

	byte.data2[1] = htonl(byte.data2[1]);
	byte.data2[2] = htonl(byte.data2[2]);


	size_t one = 1;

	size_t three = 3;

	for (int c = 0; c < 82; c++)
	{
		fwrite(fake_buffer, 1, one, writer);
	}

	fwrite(&byte.data2, 4, three, writer);

	free(fake_buffer);

	fclose(writer);

	fclose(reader);
	
}



//convert to string and print out string with for loop buf[i]
/*
void dec_to_hex(unsigned int value, char * buf)
{
	//long int remainder;
	long int quotient;
	int i = 0, temp;

	quotient = value;

	while (quotient != 0)
	{
		temp = quotient % 16;


		if(temp < 10)
			temp = temp + 48;
		else
			temp = temp + 55;


		buf[i++] = temp;

		quotient = quotient/16;
	}

	//return 0;
	
}
*/
