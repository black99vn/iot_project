//gcc -o project2v1 RyanD_timeUpdateDB.c -L/usr/lib/x86_64-linux-gnu -lcurl -lpthread
/*********************************************************************
 *
 *     Program: TimeUpdateDB
 *
 *********************************************************************
 * FileName:        RyanD_timeUpdateDB.c
 * 
 *********************************************************************
 * The program extracts the time on ten seconds intervals
 * and send that time to the database using POST. 
 *
 * Compile:
 *			gcc -o  project2v1 RyanD_timeUpdateDB.c
 *						-L/usr/lib/x86_64-linux-gnu -lcurl -lpthread
 *
 * Usage:
 *		./project2v1 
 *
 ********************************************************************/
 
 /*--------Libraries-------*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <curl/curl.h>

/*--------Definitions-------*/
#define DAEMON_NAME "timeExtract"
#define OK 			0
#define ERR_WTF		1
#define INIT_ERR	2
#define REQ_ERR		3
#define URL "http://18.188.240.240:1880/post"

/*--------Function Declarations--------*/
static void _extract_time();
static int postMethod(char *timeStr);

/* ----------------------------------- */
/*				Main				   */
/* ----------------------------------- */
int main(void)
{
	
	printf("=== Sending Time to Database Every 10s ===\n");
	
	_extract_time();
	
	return ERR_WTF;	
}

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
   return size * nmemb;
}

/*------------------------------------------------------------*/
/*	_extract_time
/*
/*	Description:
/*		Get time every 10 seconds and post it to the database
/*------------------------------------------------------------*/
static void _extract_time()
{
	time_t now;
	struct tm *tm;
	char timeStr[35] = "";
	
	while(1)
	{
		time(&now);
		tm = localtime(&now);
		sprintf(timeStr, "{\"time\":\"%02d:%02d:%02d\"}",tm->tm_hour, tm->tm_min, tm->tm_sec);
		printf("Sending %s to database\n",timeStr);	
		postMethod(timeStr);
		sleep(10);
	}
}

/*------------------------------------------------------------ */
/*	postMethod
/*
/*	Description:
/*		Perform HTTP POST with the URL
/*------------------------------------------------------------ */
static int postMethod(char *timeStr)
{
	CURL *curl;
	CURLcode res;
	struct curl_slist *headers = NULL;
	
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, URL);
	
    if (curl)
    {
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		headers = curl_slist_append(headers, "Content-Type: application/json");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, timeStr);
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