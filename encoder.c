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
int get_value(char * x, union bytes *byte, unsigned int *type_pt);
int get_gps(char * x, union gps_header *gps);
int get_statpayload(char * x, union stat_payload *pack);

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

	get_value(x, &byte, type_pt);

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

	if (*type_pt == 0)
	{
		get_statpayload(x, &packet);

		packet.printer[4] = htons(packet.printer[4]);

		packet.printer[5] = htons(packet.printer[5]);

		packet.printer[6] = htons(packet.printer[6]);

		fwrite(&packet.printer, 2, sizeof(packet.printer)/2, writer);
	}
	else if (*type_pt == 1)
	{
		//function to check for command
	}
	else if (*type_pt == 2)
	{
		get_gps(x, &info);

		fwrite(&info.degrees, 20, one, writer);
	}
	else if (*type_pt == 3)
	{

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


int get_value(char * x, union bytes *byte, unsigned int *type_pt)
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

	free(version);
	free(seq_id);
	free(type);
	free(total_length);
	free(source_device_id);
	free(dest_device_id);

	fclose(reader);

	return 1;
}


int get_statpayload(char *x, union stat_payload *pack)
{
	FILE *reader;
	reader = fopen(x, "r");

	char str[50];
	double value1[1];
	int value2[5];
	unsigned int count1 = 0;
	unsigned int count2 = 0;

	double *power = malloc(sizeof(double));
	short *glucose = malloc(sizeof(short));
	short *capsaicin = malloc(sizeof(short));
	short *omorfine = malloc(sizeof(short));

	while(fgets(str, 50, reader) != NULL)
	{
		if(sscanf(str, "Battery power : %lf\n", &value1[count1]))
		{
			*power = value1[count1];
		}
		else if (sscanf(str, "Glucose: %d\n", &value2[count2]))
		{
			*glucose = value2[count2];
			count2++;
		}
		else if(sscanf(str, "Capsaicin: %d\n", &value2[count2]))
		{
			*capsaicin = value2[count2];
			count2++;
		}
		else if(sscanf(str, "Omorfine: %d\n", &value2[count2]))
		{
			*omorfine = value2[count2];
			count2++;
		}
	}

	(*pack).payload.battery = (*power/100);
	(*pack).payload.glucose = *glucose;
	(*pack).payload.capsaicin = *capsaicin;
	(*pack).payload.omorfine = *omorfine;

	free(power);
	free(glucose);
	free(capsaicin);
	free(omorfine);

	fclose(reader);

	return 1;
}


int get_gps(char * x, union gps_header *gps)
{
	FILE *reader;
	reader = fopen(x, "r");

	char str[50];
	double value[5];
	unsigned int i = 0;

	double *tude = malloc(sizeof(double));
	double *lon = malloc(sizeof(double));
	float *alt = malloc(sizeof(float));

	while(fgets(str, 50, reader) != NULL)
	{
		if(sscanf(str, "Latitude : %lf\n", &value[i]))
		{
			*tude = value[i];
			i++;
		}
		else if (sscanf(str, "Longitude : %lf\n", &value[i]))
		{
			*lon = value[i];
			i++;
		}
		else if(sscanf(str, "Altitude : %lf\n", &value[i]))
		{
			*alt = value[i];
			i++;
		}
	}

	(*gps).fields.lat = *tude;
	(*gps).fields.longs = *lon;
	(*gps).fields.alt = (*alt/6);

	free(tude);
	free(lon);
	free(alt);

	fclose(reader);

	return 1;
}

/*

*/
