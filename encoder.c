#include <stdio.h>
#include <unistd.h>
#include <string.h>
#define SIZE 255

int binary_conversion(unsigned int num);

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

	union bytes byte;

	byte.data = 0;

	byte.medi.version = 1;	
	byte.medi.seq_id = 81;
	byte.medi.type = 3;
	byte.medi.total_length = 24;
	byte.medi.source_device_id = 7890;
	byte.medi.dest_device_id = 1234;


	printf("id: %032u\n", binary_conversion(byte.medi.source_device_id));
	printf("dest id: %032u\n", binary_conversion(byte.medi.source_device_id));

	printf("%.4d%.9d%.3d%.16d", binary_conversion(byte.medi.version), binary_conversion(byte.medi.seq_id), binary_conversion(byte.medi.type), binary_conversion(byte.medi.total_length));
	printf("\n");
	
	/*
	if (argc >= 1)
	{

		char buff[SIZE];
		char *line;
		FILE *fp;
		fp = fopen(argv[1], "r");

		while (1)
		{
		   	memset(buff, '\0', sizeof(buff));
		    	line = fgets(buff , SIZE , fp);
			if (!line) {
				break;
			}
		
	
		fclose(fp);
		//use fgets
		char buf[255];
		size_t nbytes;
		size_t bytes_read;
		FILE *fp;

		fp = fopen(argv[1], "r");

		nbytes = 1;
		bytes_read = read(fp, buf, nbytes);

		printf("bytes_read: %zd\n", bytes_read);
		
	}
	*/
}

int binary_conversion (unsigned int num)
{
	if (num == 0)
	{
		return 0;
	}
	else
	{
		return (num % 2) + 10 * binary_conversion(num/2);
	}
}
