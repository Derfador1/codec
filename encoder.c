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

struct mass_frame {
	union gps_header info;
	union bytes byte;
	union stat_payload packet;
	union com_payload command;
	struct message_payload message;
};

int fill(char * fake_buffer, FILE *writer, int *start, int *counter);
int get_value(char * x, struct mass_frame *frames, unsigned int *type_pt, unsigned int *len);
int get_gps(char * x, union gps_header *gps, unsigned int *len);
int get_statpayload(char * x, union stat_payload *pack, unsigned int *len);
int command_payload(char * x, struct mass_frame *frames, unsigned int *len, int *even);
int write_func(char * x, char * y, unsigned int *type_pt, unsigned int *max_byte);
int fwrite_func(int length, unsigned int *type_pt, int * even, struct mass_frame *frames, FILE *writer);
int get_messagepayload(char * x, struct mass_frame *frames, unsigned int *len);

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

			free(type_pt);

			free(total_len);

			fclose(reader);

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


int get_value(char * x, struct mass_frame *frames, unsigned int *type_pt, unsigned int *len)
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

	frames->byte.medi.version = *version;
	frames->byte.medi.seq_id = *seq_id;
	frames->byte.medi.type = *type;
	frames->byte.medi.source_device_id = *source_device_id;
	frames->byte.medi.dest_device_id = *dest_device_id;

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
			free(power);
			free(glucose);
			free(capsaicin);
			free(omorfine);
			free(str);

			fclose(reader);
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
		else if(sscanf(str, "Altitude : %lf", &value[i]))
		{
			*alt = value[i];
			i++;
		}
		else
		{
			free(tude);
			free(lon);
			free(alt);
			free(str);

			fclose(reader);
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


int command_payload(char * x, struct mass_frame *frames, unsigned int *len, int *even)
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

				frames->command.payloader.command = *com;

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

	frames->command.payloader.command = *com;

	frames->command.payloader.parameter = *par;

	free(com);
	free(par);
	free(str);

	fclose(reader);

	return 1;
}

int get_messagepayload(char * x, struct mass_frame *frames, unsigned int *len)
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
			free(check_buf);

			free(str);

			fclose(reader);

			return 0;
		}

		*len = ftell(reader);
	}

	frames->message.length = buffer;

	free(check_buf);

	free(str);

	fclose(reader);

	return 1;
}

int write_func(char * x, char * y, unsigned int *type_pt, unsigned int *max_byte) //change name
{
	*type_pt = 5;

	struct mass_frame frames;

	memset(&frames, '\0', sizeof(frames));

	memset(&frames.byte, '\0', sizeof(frames.byte));

	unsigned int *len = malloc(sizeof(len));

	*len = 0;

	int excess_headers = 58;

	int global_headers = 24;

	int *counter = malloc(sizeof(int));

	int *start = malloc(sizeof(int));

	char fake_buffer[1];

	fake_buffer[0] = 0;

	FILE *writer;

	writer = fopen(y, "w+");

	*start = excess_headers + global_headers;

	*counter = 0;

	int *even = malloc(sizeof(int));

	while (*len <= *max_byte)
	{
		int status_size = 0;
		int command_size = 0;
		int gps_size = 0;
		int message_size = 0;
		int length = 0;

		*even = 0;

		fill(fake_buffer, writer, start, counter);

		if (get_value(x, &frames, type_pt, len) != 1)
		{
			fprintf(stderr, "WOOOOOOOA get value error , non-encodable\n");
			exit(1);
		}

		*start = *start + global_headers;

		*counter = *start;

		*start = *start + excess_headers;

		if (*type_pt == 0)
		{
			if (get_statpayload(x, &frames.packet, len) != 1)
			{
				fprintf(stderr, "WOOOOOOOA get stat payload error , non-encodable\n");
				break;
			}

			status_size = sizeof(frames.packet.printer); //12
			length = status_size;
		}
		else if (*type_pt == 1)
		{

			if (command_payload(x, &frames, len, even) != 1)
			{
				fprintf(stderr, "WOOOOOOOA shhhhhh command payload error , non-encodable\n");
				break;
			}

			if (*even == 1)
			{	
				command_size = sizeof(frames.command.fields)/2; //12
				length = command_size;
			}
			else
			{
				command_size = sizeof(frames.command.fields); //12
				length = command_size;
			}

		}
		else if (*type_pt == 2)
		{
			if (get_gps(x, &frames.info, len) != 1)
			{
				fprintf(stderr, "WOOOOOOOA gps error , non-encodable\n");
				break;
			}

			gps_size = sizeof(frames.info.degrees); //12
			length = gps_size;
		}
		else if (*type_pt == 3)
		{	
			if (get_messagepayload(x, &frames, len) != 1)
			{
				fprintf(stderr, "WOOOOOOOA message error , non-encodable\n");
				break;
			}

			message_size = strlen(frames.message.length) - 1; //12 , and minus one for the null byte
			length = message_size;

		}

		fwrite_func(length, type_pt, even, &frames, writer);
	}

	free(even);

	free(len);

	free(start);

	free(counter);

	free(frames.message.length);

	fclose(writer);

	return 1;
}

int fwrite_func(int length, unsigned int *type_pt, int * even, struct mass_frame *frames, FILE *writer)
{

	frames->byte.data[0] = htons(frames->byte.data[0]);
	frames->byte.data[1] = htons(frames->byte.data[1]);

	frames->byte.data2[1] = htonl(frames->byte.data2[1]);
	frames->byte.data2[2] = htonl(frames->byte.data2[2]);

	int meditrik_size = 0;

	meditrik_size = sizeof(frames->byte.data); //12

	length = meditrik_size + length;

	frames->byte.medi.total_length = length;

	frames->byte.data[1] = htons(frames->byte.data[1]);
	
	fwrite((frames)->byte.data, 2, 6, writer);

	if(*type_pt == 0)
	{
		frames->packet.printer[4] = htons(frames->packet.printer[4]);

		frames->packet.printer[5] = htons(frames->packet.printer[5]);

		frames->packet.printer[6] = htons(frames->packet.printer[6]);

		fwrite(frames->packet.printer, 2, sizeof(frames->packet.printer)/2, writer);
	}
	else if (*type_pt == 1)
	{
		frames->command.fields[0] = htons(frames->command.fields[0]);

		frames->command.fields[1] = htons(frames->command.fields[1]);

		if  (*even == 1)
		{
			fwrite(&frames->command.fields, 2, 1, writer);
		}
		else
		{
			fwrite(frames->command.fields, 2, (sizeof(frames->command.fields)/2), writer);
		}
	}
	else if (*type_pt == 2)
	{
		fwrite(&frames->info.degrees, 20, 1, writer);
	}
	else if (*type_pt == 3)
	{
		size_t mes_len = strlen(frames->message.length);

		fwrite(frames->message.length, (mes_len - 1), 1, writer);
	}

	return 1;
}
