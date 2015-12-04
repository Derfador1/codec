#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#define SIZE 1500


struct meditrik {
	unsigned int type : 3;
	unsigned int seq_id : 9;
	unsigned int version : 4;
	unsigned int total_length : 16;
	unsigned int source_device_id : 32;
	unsigned int dest_device_id : 32;
};

struct gps {
	double longs;
	double lat;
	float alt;
};

struct status {
	double battery;
	short glucose;
	short capsaicin;
	short omorfine;
};

struct c_payload {
	short command;
	short parameter;
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

union com_payload {
	struct c_payload payloader;
	unsigned short fields[2];
};

struct message_payload {
	char *length;
};

int fill(char * fake_buffer, FILE *writer, int *start, int *counter);
int get_value(char * x, union bytes *byte, unsigned int *type_pt, unsigned int *len);
int get_gps(char * x, union gps_header *gps, unsigned int *len);
int get_statpayload(char * x, union stat_payload *pack, unsigned int *len);
int command_payload(char * x, union com_payload *command, unsigned int *len, int *even);
int write_func(char * x, char * y, unsigned int *type_pt, unsigned int *max_byte);
int get_messagepayload(char * x, struct message_payload *messages, unsigned int *len);

int main(int argc, char * argv[])
{
	char * x;
	char * y;

	unsigned int *type_pt = malloc(sizeof(type_pt));
	unsigned int *total_len = malloc(sizeof(total_len));

	unsigned int max_byte = 0;

	x = argv[1];
	y = argv[2];

	FILE *reader;
	reader = fopen(x, "r");

	if (argc == 1)
	{
		printf("Please retry with a valid file to open.\n");
		exit(1);
	}
	else if (argc >= 2)
	{
		if (reader == NULL)
		{
			fprintf(stderr, "Error could not open file\n");
			exit(1);
		}
		else
		{
			printf("You have successfully chosen to read from %s\n", argv[1]);
		}
	}

	if (fseek(reader, -1, SEEK_END) != 0)
	{
		fprintf(stderr, "Error reading the file\n");
	}

	max_byte = ftell(reader);

	rewind(reader);

	printf("Max bytes: %d\n", max_byte);

	if(write_func(x, y, type_pt, &max_byte) != 1)
	{
		fprintf(stderr, "Error with write_func function\n");
	}

	free(type_pt);

	free(total_len);

	fclose(reader);
}

int fill(char * fake_buffer, FILE *writer, int *start, int *counter)
{
	int c = 0;

	for (c = *counter; c < *start; c++)
	{
		fwrite(fake_buffer, 1, 1, writer);
	}

	return 1;
}


int get_value(char * x, union bytes *byte, unsigned int *type_pt, unsigned int *len)
{
	FILE *reader;
	reader = fopen(x, "r");

	char *str = malloc(SIZE);
	int value[5];
	unsigned int count = 0;

	int *version = malloc(sizeof(int));
	int *seq_id = malloc(sizeof(int));
	int *type = malloc(sizeof(int));
	int *source_device_id = malloc(sizeof(int));
	int *dest_device_id = malloc(sizeof(int));

	memset(str, '\0', SIZE);

	fseek(reader, *len, SEEK_SET);

	while (fgets(str, 50, reader) != NULL)
	{
		if (sscanf(str, "Version: %d\n", &value[count]))
		{
			*version = value[count];
			count++;
		}
		else if (sscanf(str, "Sequence: %d\n", &value[count]))
		{
			*seq_id = value[count];
			count++;
		}
		else if (sscanf(str, "Type: %d\n", &value[count]))
		{
			*type = value[count];
			count++;
		}
		else if (sscanf(str, "Source Device: %d\n", &value[count]))
		{
			*source_device_id = value[count];
			count++;
		}
		else if (sscanf(str, "Destination Device: %d\n", &value[count]))
		{
			*dest_device_id = value[count];
			count++;
		}
		else
		{	
			break;
		}
		
		*len = ftell(reader);
	}

	(*byte).medi.version = *version;
	(*byte).medi.seq_id = *seq_id;
	(*byte).medi.type = *type;
	(*byte).medi.source_device_id = *source_device_id;
	(*byte).medi.dest_device_id = *dest_device_id;

	*type_pt = *type;	

	free(version);
	free(seq_id);
	free(type);
	free(source_device_id);
	free(dest_device_id);
	free(str);

	fclose(reader);

	return 1;
}


int get_statpayload(char *x, union stat_payload *pack, unsigned int *len)
{
	FILE *reader;
	reader = fopen(x, "r");

	char *str = malloc(SIZE);
	double value1[1];
	int value2[5];
	unsigned int count1 = 0;
	unsigned int count2 = 0;

	double *power = malloc(sizeof(double));
	short *glucose = malloc(sizeof(short));
	short *capsaicin = malloc(sizeof(short));
	short *omorfine = malloc(sizeof(short));

	memset(str, '\0', SIZE);

	fseek(reader, *len, SEEK_SET);

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
		else
		{
			return 0;
		}

		*len = ftell(reader);
	}

	(*pack).payload.battery = (*power/100);
	(*pack).payload.glucose = *glucose;
	(*pack).payload.capsaicin = *capsaicin;
	(*pack).payload.omorfine = *omorfine;

	free(power);
	free(glucose);
	free(capsaicin);
	free(omorfine);
	free(str);

	fclose(reader);

	return 1;
}


int get_gps(char * x, union gps_header *gps, unsigned int *len)
{
	FILE *reader;
	reader = fopen(x, "r");

	char *str = malloc(SIZE);
	double value[5];
	unsigned int i = 0;

	double *tude = malloc(sizeof(double));
	double *lon = malloc(sizeof(double));
	float *alt = malloc(sizeof(float));

	memset(str, '\0', SIZE);

	fseek(reader, *len, SEEK_SET);

	while(fgets(str, SIZE, reader) != NULL)
	{
		if (sscanf(str, "Longitude : %lf\n", &value[i]))
		{
			*lon = value[i];
			i++;
		}
		else if(sscanf(str, "Latitude : %lf\n", &value[i]))
		{
			*tude = value[i];
			i++;
		}
		else if(sscanf(str, "Altitude : %lf\n", &value[i]))
		{
			*alt = value[i];
			i++;
		}
		else
		{
			return 0;
		}


		*len = ftell(reader);
	}

	(*gps).fields.longs = *lon;
	(*gps).fields.lat = *tude;
	(*gps).fields.alt = (*alt/6);

	free(tude);
	free(lon);
	free(alt);
	free(str);

	fclose(reader);

	return 1;
}


int command_payload(char * x, union com_payload *command, unsigned int *len, int *even)
{
	FILE *reader;
	reader = fopen(x, "r");

	char *str = malloc(SIZE);
	short *com = malloc(sizeof(short));
	int *par = calloc(1, sizeof(int));


	memset(str, '\0', SIZE);
	memset(com, '\0', sizeof(short));
	memset(par, '\0', sizeof(short));

	fseek(reader, *len, SEEK_SET);

	while (fgets(str, SIZE, reader) != NULL)
	{
		*par = 0;

		if (sscanf(str, "Command: %hd\n", com))
		{
			if (*com % 2 == 0)
			{
				*par = 0;
				*even = 1;
				*len = ftell(reader);

				free(com);
				free(par);
				free(str);

				fclose(reader);

				return 1;
			}
		}
		else if (sscanf(str, "Glucose: %d\n", par))
		{
			printf("par %d\n", *par);
		}
		else if (sscanf(str, "Capsaicin: %d\n", par))
		{
			printf("par cap %d\n", *par);
		}
		else if (sscanf(str, "Omorfine: %d\n", par))
		{

		}
		else if (sscanf(str, "Seq_param: %d", par))
		{

		}

		if (*par > 32767 || *par < 0) //might need to bee 65535
		{
			*len = ftell(reader);
			printf("Parameter field is to high for a short\n");
			break;
		}

		*len = ftell(reader);
	}

	(*command).payloader.command = *com;

	(*command).payloader.parameter = *par;

	free(com);
	free(par);
	free(str);

	fclose(reader);

	return 1;
}

int get_messagepayload(char * x, struct message_payload *messages, unsigned int *len)
{
	FILE *reader;
	reader = fopen(x, "r");

	char *str = malloc(SIZE);
	char *buffer = malloc(SIZE);
	char *check_buf = calloc(1, SIZE);

	memset(check_buf, '\0', SIZE);
	memset(buffer, '\0', SIZE);
	memset(str, '\0', SIZE);

	fseek(reader, *len, SEEK_SET);

	while(fgets(str, SIZE, reader) != NULL)
	{
		if (sscanf(str, "Message: %s", check_buf))
		{
			for (unsigned int i = 0; i < (strlen(str) - 9); i++)
			{
				buffer[i] = str[i + 9];
			}
		}
		else
		{
			return 0;
		}

		*len = ftell(reader);
	}

	messages->length = buffer;

	free(check_buf);

	free(str);

	fclose(reader);

	return 1;
}

int write_func(char * x, char * y, unsigned int *type_pt, unsigned int *max_byte)
{
	union gps_header info;
	union bytes byte;
	union stat_payload packet;
	union com_payload command;
	struct message_payload message;

	memset(&byte, '\0', sizeof(byte));
	memset(&info, '\0', sizeof(info));
	memset(&packet, '\0', sizeof(packet));
	memset(&command, '\0', sizeof(command));
	memset(&message, 0, sizeof(message));

	/*
	int meditrik_size = 0;
	int status_size = 0;
	int command_size = 0;
	int gps_size = 0;
	int message_size = 0;

	meditrik_size = sizeof(byte.data); //12
	printf("%d\n", meditrik_size);
	*/
	

	unsigned int *len = malloc(sizeof(len));

	*len = 0;

	int excess_headers = 58;

	int global_headers = 24;

	int *counter = malloc(sizeof(int));

	int *start = malloc(sizeof(int));

	char *fake_buffer = malloc(1);

	FILE *writer;

	writer = fopen(y, "w+");

	*start = excess_headers + global_headers;

	*counter = 0;


	while (*len <= *max_byte)
	{
		int *even = malloc(sizeof(int));

		*even = 0;

		fake_buffer[0] = 0;

		fill(fake_buffer, writer, start, counter);

		if (get_value(x, &byte, type_pt, len) != 1)
		{
			fprintf(stderr, "WOOOOOOOA getvalue\n");
			exit(1);
		}

		byte.data[0] = htons(byte.data[0]);
		byte.data[1] = htons(byte.data[1]);

		byte.data2[1] = htonl(byte.data2[1]);
		byte.data2[2] = htonl(byte.data2[2]);

		*start = *start + global_headers;

		*counter = *start;

		*start = *start + excess_headers;

		fwrite(&byte.data, 2, 6, writer);

		if (*type_pt == 0)
		{
			if (get_statpayload(x, &packet, len) != 1)
			{
				fprintf(stderr, "WOOOOOOOA get stat payload\n");
				break;
			}

			packet.printer[4] = htons(packet.printer[4]);

			packet.printer[5] = htons(packet.printer[5]);

			packet.printer[6] = htons(packet.printer[6]);

			fwrite(&packet.printer, 2, sizeof(packet.printer)/2, writer);
		}
		else if (*type_pt == 1)
		{
			if (command_payload(x, &command, len, even) != 1)
			{
				fprintf(stderr, "WOOOOOOOA shhhhhh command payload\n");
				break;
			}

			command.fields[0] = htons(command.fields[0]);

			command.fields[1] = htons(command.fields[1]);

			if  (*even == 1)
			{
				fwrite(&command.fields, 2, 1, writer);
			}
			else
			{
				fwrite(&command.fields, 2, (sizeof(command.fields)/2), writer);
			}
		}
		else if (*type_pt == 2)
		{
			if (get_gps(x, &info, len) != 1)
			{
				fprintf(stderr, "WOOOOOOOA gps\n");
				break;
			}

			fwrite(&info.degrees, 20, 1, writer);
		}
		else if (*type_pt == 3)
		{	
			if (get_messagepayload(x, &message, len) != 1)
			{
				fprintf(stderr, "WOOOOOOOA message\n");
				break;
			}

			size_t length = strlen(message.length);

			fwrite(message.length, length, 1, writer);
		}

		free(even);
	
	}

	free(len);

	free(fake_buffer);

	free(message.length);

	free(start);

	free(counter);

	fclose(writer);

	return 1;
}
