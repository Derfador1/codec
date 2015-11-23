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

struct status {
	double battery;
	short glucose;
	short capsaicin;
	short omorfine;
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

union stat_payload {
	struct status payload;
	unsigned short printer[7];
};

int fill(char * fake_buffer, size_t one, FILE *writer);
int get_value(char * x, union bytes *byte, union gps_header *info, union stat_payload *packet, unsigned int *type_pt);
//int get_gps(char * x, union gps_header *info);
//int get_statpayload(char *x, union stat_playload *packet);

int main(int argc, char * argv[])
{
	char * x;
	unsigned int *type_pt = malloc(sizeof(type_pt));
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
	union stat_payload packet;

	memset(byte.data2, '\0', sizeof(byte.data2));
	memset(byte.data, '\0', sizeof(byte.data));
	memset(info.degrees, '\0', sizeof(info.degrees));
	memset(packet.printer, '\0', sizeof(packet.printer));

	get_value(x, &byte, &info, &packet, type_pt);

	FILE *writer;

	writer = fopen(argv[2], "w+");

	char *fake_buffer = malloc(1);

	fake_buffer[0] = 0;
	byte.data[0] = htons(byte.data[0]);
	byte.data[1] = htons(byte.data[1]);

	byte.data2[1] = htonl(byte.data2[1]);
	byte.data2[2] = htonl(byte.data2[2]);

	//move to if statement
	packet.printer[4] = htons(packet.printer[4]);
	packet.printer[5] = htons(packet.printer[5]);
	packet.printer[6] = htons(packet.printer[6]);

	size_t one = 1;

	size_t six = 6;

	fill(fake_buffer, one, writer);

	fwrite(&byte.data, 2, six, writer);

	if (*type_pt == 0)
	{
		fwrite(&packet.printer, 2, 7, writer); //sizeof(packet.fields) instead of 14
	}
	else if (*type_pt == 2)
	{
		fwrite(&info.degrees, 20, one, writer);
	}

	free(fake_buffer);

	free(type_pt);

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


int get_value(char * x, union bytes *byte, union gps_header *info, union stat_payload *packet, unsigned int *type_pt)
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

	*type_pt = *type;

	if (*type == 0)
	{
		double *power = malloc(sizeof(double));
		short *glucose = malloc(sizeof(short));
		short *capsaicin = malloc(sizeof(short));
		short *omorfine = malloc(sizeof(short));

		fscanf(reader, "Battery power : %lf%%\n", power);
		fscanf(reader, "Glucose: %hd\n", glucose);
		fscanf(reader, "Capsaicin: %hd\n", capsaicin);
		fscanf(reader, "Omorfine: %hd\n", omorfine);

		(*packet).payload.battery = (*power/100);
		(*packet).payload.glucose = *glucose;
		(*packet).payload.capsaicin = *capsaicin;
		(*packet).payload.omorfine = *omorfine;

		printf("%.2f\n", *power);
		printf("%d\n", *glucose);
		printf("%d\n", *capsaicin);
		printf("%d\n", *omorfine);

		free(power);
		free(glucose);
		free(capsaicin);
		free(omorfine);
	}	
	else if (*type == 2)
	{
		double *tude = malloc(sizeof(double));
		double *lon = malloc(sizeof(double));
		float *alt = malloc(sizeof(float));

		fscanf(reader, "Latitude : %lf degree N\n", tude);
		fscanf(reader, "Longitude : %lf degree W\n", lon);
		fscanf(reader, "Altitude : %f ft\n", alt);

		(*info).fields.lat = *tude;
		(*info).fields.longs = *lon;
		(*info).fields.alt = (*alt/6);

		/*
		printf("%.9lf\n", *tude);
		printf("%.9lf\n", *lon);
		printf("%.1f\n", (*info).fields.alt);
		*/

		free(tude);
		free(lon);
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
