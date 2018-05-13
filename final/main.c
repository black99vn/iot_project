#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>

#define SPoint				10
#define DEFAULT_TEMP		70
#define URL_CURRENTTEMP 	"http://18.188.240.240:1880/loadtemp"
#define URL_HEATERSTATUS	"http://18.188.240.240:1880/heaterstatus"
#define URL_GET				"http://18.188.240.240:1880/getpoints"
#define URL_SETPOINTS		"http://18.188.240.240:1880/setpoints"
#define URL_SETTEMP			"http://18.188.240.240:1880/settemp"

#define OK 			0
#define ERR_WTF		1
#define INIT_ERR	2
#define REQ_ERR		3

static const char*  TEMP_FILENAME   = "/tmp/temp";
static const char*  STATE_FILENAME  = "/tmp/status";
static const char*	CONFIG_FILENAME = "/tmp/config";
static const char*  WORKING_DIR     = "/";
 
static int read_temp();
static void write_status(int cmd);
static void temp_control(int setpoint, int current);
static int read_config(int argc, char *argv[]);
static void helpMessage();
static void _extract_time();
static void sort_setpoints(int count);
static int postMethod(char *timeStr, int type);
static int update_setpoints();

int set_temp;
int setpoints[SPoint][2];

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
   return size * nmemb;
}

int main(int argc, char *argv[])
{
	int cur_temp;
	int total_setpoints = 0;

	set_temp = DEFAULT_TEMP;
	
	if (argc > 2)
	{
		total_setpoints = read_config(argc, argv);
		update_setpoints();
	}
	else if (argc == 1)
		postMethod("DEFAULT", 2);
		
	while(1)
	{
		_extract_time(total_setpoints);
		cur_temp = read_temp();
		temp_control(set_temp, cur_temp);
		printf("Current %d Set Temp to: %d\n",cur_temp, set_temp);
		sleep(5);
	}
	
	return 0;
}



static int update_setpoints(int total_setpoints)
{
	FILE* file = fopen(CONFIG_FILENAME, "w"); // write mode
	
	if (file == NULL)
	{
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}
	
	fprintf(file, "%s", setpoints);
	postMethod(setpoints, 2);
	fclose (file);    
}

static int postMethod(char *message, int type)
{
	CURL *curl;
	CURLcode res;
	struct curl_slist *headers = NULL;
	
	curl = curl_easy_init();
	
	if (type == 0)
		curl_easy_setopt(curl, CURLOPT_URL, URL_CURRENTTEMP);
	else if (type == 1)
		curl_easy_setopt(curl, CURLOPT_URL, URL_HEATERSTATUS);
	else if (type == 2)
		curl_easy_setopt(curl, CURLOPT_URL, URL_SETPOINTS);
	else if (type == 3)
		curl_easy_setopt(curl, CURLOPT_URL, URL_SETTEMP); 
	
    if (curl)
    {
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		headers = curl_slist_append(headers, "Content-Type: text/plain");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, message);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
	    {
			// Describing error code from res
			fprintf(stderr, "Failed: %s\n", curl_easy_strerror(res));
			return REQ_ERR;
	    }
        curl_easy_cleanup(curl);
    }
    else
		return INIT_ERR;
}

static void _extract_time(int total_setpoints)
{
	time_t now;
	struct tm *tm;
	int current_time = 0;
	int i;
	char message[5] = "";
	
	time(&now);
	tm = localtime(&now);
	current_time = (tm->tm_hour * 100)+tm->tm_min;
	
	for (i = 0; i < total_setpoints; i++)
	{
		if (current_time >= setpoints[i][0])
			set_temp = setpoints[i][1];
	}
	
	sprintf(message,"%d",set_temp);
	postMethod(message, 3);
}

static int read_config(int argc, char *argv[])
{
	FILE* file;
	int hr, temp, i = 0;
	char *config_path = malloc(50);

	if (argc == 3 && (!strcmp(argv[1], "-c") || !strcmp(argv[1], "--config_file")))
	{
		strcpy(config_path, argv[2]);
		printf("%s\n",config_path);
	}
	
	else if (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")))
	{
		helpMessage();
		exit(0);
	}
	
	else if (argc == 1)
	{
		return 0;
	}
	
	file = fopen(config_path, "r"); // read mode
	
	if (file == NULL)
	{
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}
	
	while(fscanf (file, "%d %d", &hr, &temp) != EOF)
	{
		setpoints[i][0] = hr;
		setpoints[i][1] = temp;
		printf("%d\t%d\n",hr,temp);
		i++;
	}
	sort_setpoints(i);
	
	
	
	fclose (file);    
	return i;
}

static void sort_setpoints(int count)
{
	int temp, i, j, k;
	for (j = 0; j < count; ++j)
	{
		for (k = j + 1; k < count; ++k)
		{
			if (setpoints[j][0] > setpoints[k][0])
			{
				temp = setpoints[j][0];
				setpoints[j][0] = setpoints[k][0];
				setpoints[k][0] = temp;
			
				temp = setpoints[j][1];
				setpoints[j][1] = setpoints[k][1];
				setpoints[k][1] = temp;
         }
      }
   }
}

static void helpMessage()
{
    printf("Options:\n"
  		"\t-u/--url 	<URL>		URL to work with\n"
 		"\t-o/--post 	<string>	Send HTTP POST with string\n"
 		"\t-g/--get 			Send HTTP GET to retrieve data\n"
 		"\t-p/--put	<string>	Send HTTP PUT with string\n"
 		"\t-d/--delete	<string>	Send HTTP Delete with string\n"
 		"\t-h/--help			Help text\n");
	printf("Usage:\n"
 		"\t./prog [options...] <url> <string>\n");
		
	printf("Usage:\n\tSpecial characters must be in double or single quotation marks\n"
				"\tURL must contain http:// or https://\n");

	printf("Examples:\n"
 	 	"\t./prog --post --url http://localhost:8080 here is the message\n"
 		"\t./prog --get --url http://www.cnn.com\n"
 		"\t./prog --put --url http://localhost:8080 putting message!\n"
 		"\t./prog --delete --url http://localhost:8080 78392\n"
		"\t./prog --help\n");
 
}

static void temp_control(int setpoint, int current)
{
	if (setpoint+2 <= current)
		write_status(0);
	else if (setpoint-2 >= current)
		write_status(1);
}

static void write_status(int cmd)
{
	FILE* file = fopen(STATE_FILENAME, "w"); // write mode
	char message[2] = "";
	if (file == NULL)
	{
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}
	
	sprintf(message,"%s", cmd?"ON":"OFF");
	fprintf(file, "%s", message);
	postMethod(message, 1);
	fclose (file);    
}

static int read_temp()
{
	FILE* file = fopen(TEMP_FILENAME, "r"); // read mode
	int i = 0;
	char message[5] = "";
	
	if (file == NULL)
	{
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}
	
	fscanf (file, "%d", &i);
	
	sprintf(message, "%d",i);
	postMethod(message, 0);
	
	fclose (file);    
	return i;
}