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

int fill(char * fake_buffer, size_t one, FILE *writer);
int get_value(char * x, union bytes *byte, unsigned int *type_pt, unsigned int *total_len);
int get_gps(char * x, union gps_header *gps);
int get_statpayload(char * x, union stat_payload *pack);
int command_payload(char * x, union com_payload *command);
int write_func(char * x, char * y, unsigned int *total_len, unsigned int *type_pt);
int get_messagepayload(char * x, struct message_payload *messages, unsigned int *total_len);

int main(int argc, char * argv[])
{
	char * x;
	char * y;
	unsigned int *type_pt = malloc(sizeof(type_pt));
	unsigned int *total_len = malloc(sizeof(total_len));

	if (argc == 1)
	{
		printf("Please retry with a valid file to open.\n");
		exit(1);
	}
	else if (argc >= 2)
	{
		//add error checking to make sure its a valid file
		printf("You have successfully chosen to read from %s\n", argv[1]);
	}

	x = argv[1];
	y = argv[2];

	write_func(x, y, total_len, type_pt);

	free(type_pt);

	free(total_len);
}

int fill(char * fake_buffer, size_t one, FILE *writer)
{
	for (int c = 0; c < 82; c++)
	{
		fwrite(fake_buffer, 1, one, writer);
	}

	return 1;
}


int get_value(char * x, union bytes *byte, unsigned int *type_pt, unsigned int *total_len)
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


int command_payload(char * x, union com_payload *command)
{
	FILE *reader;
	reader = fopen(x, "r");

	char str[50];
	
	size_t one = 1;

	short *com = malloc(sizeof(short));
	short *par = calloc(one, sizeof(short));

	memset(str, '\0', 50);

	while (fgets(str, 50, reader) != NULL)
	{
		if (sscanf(str, "Command: %hd\n", com))
		{
			printf("Command: %hd\n", *com);
		}


		if (*com == 1)
		{
			sscanf(str, "Glucose: %hd\n", par);
		}
		else if (*com == 3)
		{
			sscanf(str, "Capsaicin: %hd\n", par);
		}
		else if (*com == 5)
		{
			sscanf(str, "Omorfine: %hd\n", par);
		}
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
	else if (*com == 7)
	{
		//ummm possible change to set param as seq 
		printf("Repeat packet(7)\n");	
	}

	(*command).payloader.command = *com;

	(*command).payloader.parameter = *par;

	free(com);
	free(par);

	return 1;
}

int get_messagepayload(char * x, struct message_payload *messages, unsigned int *total_len)
{
	FILE *reader;
	reader = fopen(x, "r");

	char str[50];
	//char *buffer = malloc(SIZE);
	unsigned char *buffer = malloc(SIZE);
	unsigned char *check_buf = calloc(1, SIZE);

	memset(check_buf, '\0', SIZE);
	memset(buffer, '\0', SIZE);
	memset(str, '\0', 50);

	unsigned int length = 0;

	length = (*total_len - 12);

	while(fgets(str, 50, reader) != NULL)
	{
		if (sscanf(str, "Message: %s", check_buf))
		{
			for (unsigned int i = 0; i < length; i++)
			{
				buffer[i] = str[i + 9];
			}
		}
	}

	messages->length = buffer;

	free(check_buf);

	fclose(reader);

	return 1;
}

int write_func(char * x, char * y, unsigned int *total_len, unsigned int *type_pt)
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

	get_value(x, &byte, type_pt, total_len);

	FILE *writer;

	writer = fopen(y, "w+");

	char *fake_buffer = malloc(1);

	fake_buffer[0] = 0;

	byte.data[0] = htons(byte.data[0]);
	byte.data[1] = htons(byte.data[1]);

	byte.data2[1] = htonl(byte.data2[1]);
	byte.data2[2] = htonl(byte.data2[2]);

	size_t one = 1;

	size_t six = 6;

	size_t length = (*total_len - 12);

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
		command_payload(x, &command);

		command.fields[0] = htons(command.fields[0]);

		command.fields[1] = htons(command.fields[1]);

		fwrite(&command.fields, 2, 2, writer);
	}
	else if (*type_pt == 2)
	{
		get_gps(x, &info);

		fwrite(&info.degrees, 20, one, writer);
	}
	else if (*type_pt == 3)
	{	
		get_messagepayload(x, &message, total_len);

		printf("%s\n", message.length);

		fwrite(message.length, length, one, writer);
	}

	free(fake_buffer);

	free(message.length);

	fclose(writer);

	return 1;
}
