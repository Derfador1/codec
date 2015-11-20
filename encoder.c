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

struct gps {
	double lat;
	double longs;
	float alt;
};

union gps_header{
	struct gps fields;
	unsigned char degrees[20];
};

union bytes {
	struct meditrik medi;
	unsigned short data[6];
	unsigned int data2[3];
};

int fill(char * fake_buffer, size_t one, FILE *writer);
int get_value(char * x, union bytes *byte, union gps_header *info);

int main(int argc, char * argv[])
{
	char * x;
	if (argc == 1)
	{
		printf("Please retry with a valid file to open.\n");
		exit(1);
	}
	else if (argc >= 2)
	{
		printf("You have successfully chosen to read from %s\n", argv[1]);
	}

	x = argv[1];
	union gps_header info;
	union bytes byte;

	memset(byte.data2, '\0', sizeof(byte.data2));

	get_value(x, &byte, &info);

	FILE *writer;

	writer = fopen(argv[2], "w+");


	char *fake_buffer = malloc(1);

	fake_buffer[0] = 0;
	byte.data[0] = htons(byte.data[0]);
	byte.data[1] = htons(byte.data[1]);

	byte.data2[1] = htonl(byte.data2[1]);
	byte.data2[2] = htonl(byte.data2[2]);


	size_t one = 1;

	size_t six = 6;

	fill(fake_buffer, one, writer);

	fwrite(&byte.data, 2, six, writer);

	fwrite(&info.degrees, 20, one, writer);

	free(fake_buffer);

	fclose(writer);
}

int fill(char * fake_buffer, size_t one, FILE *writer)
{
	for (int c = 0; c < 82; c++)
	{
		fwrite(fake_buffer, 1, one, writer);
	}

	return 1;
}


int get_value(char * x, union bytes *byte, union gps_header *info)
{
	FILE *reader;
	reader = fopen(x, "r");

	int *version = malloc(sizeof(int));
	int *seq_id = malloc(sizeof(int));
	int *type = malloc(sizeof(int));
	int *total_length = malloc(sizeof(int));
	int *source_device_id = malloc(sizeof(int));
	int *dest_device_id = malloc(sizeof(int));

	fscanf(reader, "Version: %d\n", version);
	fscanf(reader, "Sequence: %d\n", seq_id);
	fscanf(reader, "Type: %d\n", type);
	fscanf(reader, "Total Length: %d\n", total_length);
	fscanf(reader, "Source Device: %d\n", source_device_id);
	fscanf(reader, "Destination Device: %d\n", dest_device_id);

	(*byte).medi.version = *version;
	(*byte).medi.seq_id = *seq_id;
	(*byte).medi.type = *type;
	(*byte).medi.total_length = *total_length;
	(*byte).medi.source_device_id = *source_device_id;
	(*byte).medi.dest_device_id = *dest_device_id;

	if (*type == 2)
	{
		double *tude = malloc(sizeof(double));
		double *longs = malloc(sizeof(double));
		double *alt = malloc(sizeof(double));

		fscanf(reader, "Latitude : %lf\n", tude);
		(*info).fields.lat = *tude;

		printf("%.9lf\n", *tude);

		free(tude);
		free(longs);
		free(alt);
	}

	printf("%d\n", *version);
	printf("%d\n", *seq_id);
	printf("%d\n", *type);
	printf("%d\n", *total_length);
	printf("%d\n", *source_device_id);
	printf("%d\n", *dest_device_id);

	free(version);
	free(seq_id);
	free(type);
	free(total_length);
	free(source_device_id);
	free(dest_device_id);

	fclose(reader);

	return 1;
}
