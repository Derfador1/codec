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
	unsigned char *length;
};

int fill(char * fake_buffer, size_t one, FILE *writer, int *start);
int get_value(char * x, union bytes *byte, unsigned int *type_pt, unsigned int *total_len, unsigned int *len);
int get_gps(char * x, union gps_header *gps, unsigned int *len);
int get_statpayload(char * x, union stat_payload *pack, unsigned int *len);
int command_payload(char * x, union com_payload *command, unsigned int *len);
int write_func(char * x, char * y, unsigned int *total_len, unsigned int *type_pt, unsigned int *max_byte);
int get_messagepayload(char * x, struct message_payload *messages, unsigned int *total_len, unsigned int *len);

int main(int argc, char * argv[])
{
	char * x;
	char * y;

	unsigned int *type_pt = malloc(sizeof(type_pt));
	unsigned int *total_len = malloc(sizeof(total_len));

	unsigned int max_byte = 0;

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
	y = argv[2];

	FILE *reader;
	reader = fopen(x, "r");

	fseek(reader, -1, SEEK_END);

	max_byte = ftell(reader);

	rewind(reader);

	printf("%d\n", max_byte);

	write_func(x, y, total_len, type_pt, &max_byte);

	free(type_pt);

	free(total_len);
}

int fill(char * fake_buffer, size_t one, FILE *writer, int *start)
{

	//int c = *start - excess_headers;

	int c = 0;

	for (c = 0; c < *start; c++)
	{
		fwrite(fake_buffer, 1, one, writer);
	}

	return 1;
}


int get_value(char * x, union bytes *byte, unsigned int *type_pt, unsigned int *total_len, unsigned int *len)
{
	FILE *reader;
	reader = fopen(x, "r");

	char str[50];
	int value[5];
	unsigned int count = 0;

	int *version = malloc(sizeof(int));
	int *seq_id = malloc(sizeof(int));
	int *type = malloc(sizeof(int));
	int *total_length = malloc(sizeof(int));
	int *source_device_id = malloc(sizeof(int));
	int *dest_device_id = malloc(sizeof(int));

	fseek(reader, *len, SEEK_SET);

	while (fgets(str, 50, reader) != NULL)
	{
		if (sscanf(str, "Version: %d\n", &value[count]))
		{
			*version = value[count];
			printf("Version: %d\n", *version);
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
		else if (sscanf(str, "Total Length: %d\n", &value[count]))
		{
			*total_length = value[count];
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
		printf("%d\n", *len);
	}

	(*byte).medi.version = *version;
	(*byte).medi.seq_id = *seq_id;
	(*byte).medi.type = *type;
	(*byte).medi.total_length = *total_length;
	(*byte).medi.source_device_id = *source_device_id;
	(*byte).medi.dest_device_id = *dest_device_id;

	*type_pt = *type;
	*total_len = *total_length;	

	free(version);
	free(seq_id);
	free(type);
	free(total_length);
	free(source_device_id);
	free(dest_device_id);

	fclose(reader);

	return 1;
}


int get_statpayload(char *x, union stat_payload *pack, unsigned int *len)
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

	printf("Length in status payload before:%d\n", *len);

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
			break;
		}

		*len = ftell(reader);
		printf("Stat payload after %d\n", *len);
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


int get_gps(char * x, union gps_header *gps, unsigned int *len)
{
	FILE *reader;
	reader = fopen(x, "r");

	char str[50];
	double value[5];
	unsigned int i = 0;

	double *tude = malloc(sizeof(double));
	double *lon = malloc(sizeof(double));
	float *alt = malloc(sizeof(float));

	printf("Length in gps payload before:%d\n", *len);

	fseek(reader, *len, SEEK_SET);

	while(fgets(str, 50, reader) != NULL)
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
			break;
		}

		*len = ftell(reader);
		printf("Gps payload after %d\n", *len);
	}

	(*gps).fields.longs = *lon;
	(*gps).fields.lat = *tude;
	(*gps).fields.alt = (*alt/6);

	free(tude);
	free(lon);
	free(alt);

	fclose(reader);

	return 1;
}


int command_payload(char * x, union com_payload *command, unsigned int *len)
{
	FILE *reader;
	reader = fopen(x, "r");

	char str[50];
	
	size_t one = 1;

	short *com = malloc(sizeof(short));
	short *par = calloc(one, sizeof(short));

	memset(str, '\0', 50);
	memset(com, '\0', sizeof(short));
	memset(par, '\0', sizeof(short));

	printf("Length in command payload before:%d\n", *len);

	fseek(reader, *len, SEEK_SET);

	while (fgets(str, 50, reader) != NULL)
	{
		if (sscanf(str, "Command: %hd\n", com))
		{
			printf("Command: %hd\n", *com);
		}
		else if (sscanf(str, "Glucose: %hd\n", par))
		{
			//
		}
		else if (sscanf(str, "Capsaicin: %hd\n", par))
		{
			//
		}
		else if (sscanf(str, "Omorfine: %hd\n", par))
		{
			//
		}
		else if (sscanf(str, "Seq_param: %hd", par))
		{
			//
		}
		else
		{
			break;
		}

		*len = ftell(reader);
		printf("command payload after command %d\n", *len);


	}


	if (*com == 0)
	{
		printf("Get status\n");
	}
	else if (*com == 2)
	{
		printf("Get GPS data\n");
	}
	else if (*com == 4)
	{
		printf("Reserved(4)\n");
	}
	else if (*com == 6)
	{
		printf("Reserved(6)\n");
	}

	(*command).payloader.command = *com;

	(*command).payloader.parameter = *par;

	free(com);
	free(par);

	fclose(reader);

	return 1;
}

int get_messagepayload(char * x, struct message_payload *messages, unsigned int *total_len, unsigned int *len)
{
	FILE *reader;
	reader = fopen(x, "r");

	char *str = malloc(SIZE);
	unsigned char *buffer = malloc(SIZE);
	unsigned char *check_buf = calloc(1, SIZE);

	memset(check_buf, '\0', SIZE);
	memset(buffer, '\0', SIZE);
	memset(str, '\0', SIZE);

	printf("Length in message payload before:%d\n", *len);

	fseek(reader, *len, SEEK_SET);

	unsigned int length = 0;

	length = (*total_len - 12);

	while(fgets(str, SIZE, reader) != NULL)
	{
		if (sscanf(str, "Message: %s", check_buf))
		{
			for (unsigned int i = 0; i < length; i++)
			{
				buffer[i] = str[i + 9];
			}
		}
		else
		{
			break;
		}

		*len = ftell(reader);
		printf("Message payload after %d\n", *len);
	}

	messages->length = buffer;

	free(check_buf);

	free(str);

	fclose(reader);

	return 1;
}

int write_func(char * x, char * y, unsigned int *total_len, unsigned int *type_pt, unsigned int *max_byte)
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

	unsigned int *len = malloc(sizeof(len));

	*len = 0;

	//memset(len, 0, sizeof(len));

	int excess_headers = 58;
	int global_headers = 24;

	int *start = malloc(sizeof(*start));

	char *fake_buffer = malloc(1);

	FILE *writer;

	writer = fopen(y, "w+");

	*start = excess_headers + global_headers;

	//start loop here instead
	//change start value here so that fill can hav +58
	//actually might just need fill the fill function

	printf("Max_byte %d\n", *max_byte);
	printf("Len %d\n", *len);

	while (*len < *max_byte)
	{
		size_t one = 1;

		size_t six = 6;

		size_t length = (*total_len - 12);

		fake_buffer[0] = 0;

		fill(fake_buffer, one, writer, start);

		get_value(x, &byte, type_pt, total_len, len);

		byte.data[0] = htons(byte.data[0]);
		byte.data[1] = htons(byte.data[1]);

		byte.data2[1] = htonl(byte.data2[1]);
		byte.data2[2] = htonl(byte.data2[2]);


		fwrite(&byte.data, 2, six, writer);

		if (*type_pt == 0)
		{
			get_statpayload(x, &packet, len);

			packet.printer[4] = htons(packet.printer[4]);

			packet.printer[5] = htons(packet.printer[5]);

			packet.printer[6] = htons(packet.printer[6]);

			fwrite(&packet.printer, 2, sizeof(packet.printer)/2, writer);
		}
		else if (*type_pt == 1)
		{
			command_payload(x, &command, len);

			command.fields[0] = htons(command.fields[0]);

			command.fields[1] = htons(command.fields[1]);

			fwrite(&command.fields, 2, 2, writer);
		}
		else if (*type_pt == 2)
		{
			get_gps(x, &info, len);

			fwrite(&info.degrees, 20, one, writer);
		}
		else if (*type_pt == 3)
		{	
			get_messagepayload(x, &message, total_len, len);

			printf("%s\n", message.length);

			fwrite(message.length, length, one, writer);
		}


		printf("Count before next loop %d\n", *start);
		printf("Len %d\n", *len);

		printf("Count before next loop %d\n", *start);
	}

	free(fake_buffer);

	free(message.length);

	free(start);

	fclose(writer);

	return 1;
}
