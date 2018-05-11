//gcc -o hw RyanD_NetComArm.c -L/usr/lib/x86_64-linux-gnu -lcurl -lpthread
/*********************************************************************
 *
 *      Network Communication on ARM
 *
 *********************************************************************
 * FileName:        RyanD_NetComArm.c
 * 
 *********************************************************************
 * This program is using libcurl to communicate via HTTP.
 * The program takes command line arguments, and executes various HTTP 
 * Request Methods.
 * Options:
 * 		-u/--url 	<URL>		URL to work with
 *		-o/--post 	<string>	Send HTTP POST with string
 *		-g/--get 				Send HTTP GET to retrieve data
 *		-p/--put	<string>	Send HTTP PUT with string
 *		-d/--delete	<string>	Send HTTP Delete with string
 *		-h/--help				Help text
 *
 * Compile:
 *		x86_64:
 *			gcc -o hw RyanD_NetComArm.c 
 *						-L/usr/lib/x86_64-linux-gnu -lcurl -lpthread
 *
 *		ARM:
 *			(BUILDROOT_HOME)/output/host/usr/bin/arm-linux-gcc 
 *					--sysroot=(BUILDROOT_HOME)/output/staging  
 *					-o test RyanD_NetComArm.o  -lcurl -uClibc -lc
 *
 * Usage:
 *		./hw [options...] <url> <string>
 
 * Usage: special character must be in single or double quotation marks
 
 * Examples:
 *	 	./hw --post --url http://localhost:8080 here is the message
 *		./hw --get --url http://www.cnn.com
 *		./hw --put --url http://localhost:8080 putting message!
 *		./hw --delete --url http://localhost:8080 78392
 *      ./hw --help
 *
 * Special Characters && Short Commands Examples:
 *	 	./hw -u http://localhost:8080 -o "I'm writing this!"
 *		./hw -g -u http://www.cnn.com
 *		./hw -u http://localhost:8080 -p "')(*@)($*)#(#'"!
 *		./hw -d -u http://localhost:8080 78392
 *      ./hw -h
 *
 ********************************************************************/
 
/*--------Libraries-------*/
#include <stdio.h>
#include <curl/curl.h>
#include <string.h>

/*--------Definitions-------*/
#define OK		0
#define INIT_ERR	1
#define REQ_ERR		2
#define ARG_ERR		3
#define URL_ERR		4
#define METHOD_ERR	5
#define MSG_ERR		6
#define MAX_STRING_LENS 50

/*--------Constant Chars For Validating URL--------*/
static const char *nospecial="0123456789"
                "abcdefghijklmnopqrstuvwxyz"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                ".:/";
				
static const char *portNums="0123456789";

/*--------CURL Global Variables------*/
CURL *curl;
CURLcode res;

/*--------Function Declarations--------*/
int postMethod(char *messageStr);
int getMethod();
int putMethod(char *messageStr);
int deleteMethod(char *messageStr);
void helpMessage();
int errMessage(int code);

/* ----------------------------------- */
/*				Main				   */
/* ----------------------------------- */
int main(int argc, char *argv[])
{
    int status;
	char URL[100]="";
	char messageStr[100] = "";
   	int cmd=0;
	int c=0;
	int portFlag = 0;
	int urlReady = 0;
	char method = ' ';
	
	curl = curl_easy_init();
	
	// At least 1 argument in cmd-line for --help 
    if (argc < 1)
	{
		return errMessage(ARG_ERR);
    }
	
	// Loop through all the arguments to check for options
	for (cmd = 0; cmd < argc; cmd++)
	{
		// --url/-u option 
		if ((!strcmp(argv[cmd], "-u") || !strcmp(argv[cmd], "--url")) && !urlReady)
		{
			// url should be defined after -u/--url option is found
			if (cmd+1 < argc)
			{
				urlReady = 1;
				cmd++;
				strcpy(URL, argv[cmd]); //copy the URL
			}
			else
				return errMessage(ARG_ERR);
			
			// Check for http:// or https:// in the URL
			if (strncmp(URL,"http://",7) != 0 && strncmp(URL,"https://",8) != 0)
				return errMessage(URL_ERR);
			
			// Check for special characters in the URL and port numbers should be all numbers
			while(URL[c])
			{
				if(!strchr(nospecial, URL[c]) || (portFlag && !strchr(portNums, URL[c])))
					return errMessage(URL_ERR);
				else if (strchr(":", URL[c]) && !strchr("/", URL[c+1]))
					portFlag = 1;
				c++;					
			}
			curl_easy_setopt(curl, CURLOPT_URL, URL);
		}
		
		// --post/-o option 
		else if (!strcmp(argv[cmd], "-o") || !strcmp(argv[cmd], "--post") || method == 'O')
		{
			// set method to POST
			if (method == ' ')
				method = 'O';
			
			/* since the string should be at the end of arguments, wait until url is found,
			Then put the rest of arguments in messageStr*/
			else if(urlReady)
			{
				strcat(messageStr, argv[cmd]);
				if (cmd+1 < argc)
					strcat(messageStr, " ");
			}
		}
		
		// --get/-g option 
		else if (!strcmp(argv[cmd], "-g") || !strcmp(argv[cmd], "--get"))
		{
			// set method to GET
			if (method == ' ')
				method = 'G';
		}
		
		// --put/-p option 
		else if (!strcmp(argv[cmd], "-p") || !strcmp(argv[cmd], "--put") || method == 'P')
		{
			// set method to PUT
			if (method == ' ')
				method = 'P';
			
			/* since the string should be at the end of arguments, wait until url is found,
			Then put the rest of arguments in messageStr*/
			else if(urlReady)
			{
				strcat(messageStr, argv[cmd]);
				if (cmd+1 < argc)
					strcat(messageStr, " ");
			}
		}
		
		// --delete/-d option 
		else if (!strcmp(argv[cmd], "-d") || !strcmp(argv[cmd], "--delete") || method == 'D')
		{
			// set method to DELETE
			if (method == ' ')
				method = 'D';
			
			/* since the string should be at the end of arguments, wait until url is found,
			then put the rest of arguments in messageStr*/
			else if(urlReady)
			{
				strcat(messageStr, argv[cmd]);
				if (cmd+1 < argc)
					strcat(messageStr, " ");
			}
		}
		
		// --help/-h option 
		else if (!strcmp(argv[cmd], "-h") || !strcmp(argv[cmd], "--help"))
		{	
			// set method to HELP
			method = 'H';
			break;
		}
	}
	
	// Return error if no methods/url found
	if (method == ' ' && !urlReady)
		return errMessage(METHOD_ERR);
	else if (method == ' ')
		return errMessage(METHOD_ERR);
	else if (!urlReady && method != 'H')
		return errMessage(URL_ERR);
	
	// Return error if no strings found for POST/PUT/DELETE requests
	if ((method == 'O' || method == 'P' || method == 'D') && !strcmp(messageStr,""))
		return errMessage(MSG_ERR);
	
	// Now call the HTTP request based on the method found in cmd-line
	switch (method)
	{	
		case 'O':
			status = postMethod(messageStr);
			break;
		case 'G':
			status = getMethod();
			break;
		case 'P':
			status = putMethod(messageStr);
			break;
		case 'D':
			status = deleteMethod(messageStr);
			break;
		case 'H':
			helpMessage();
			break;
	}
	return status; 
}

/* ------------------------------------------------------------ */
/**	errMessage
**
**	Description:
**		Provides Error Messages to Users
/* ------------------------------------------------------------ */
int errMessage(int code)
{
	if (code == ARG_ERR)
		fprintf(stderr,"ERROR[%d]: Missing Arguments.\n", code);
	else if (code == URL_ERR)
		fprintf(stderr,"ERROR[%d]: Invalid URL/Option\n", code);
	else if (code == METHOD_ERR)
		fprintf(stderr,"ERROR[%d]: No Method Found\n", code);
	else if (code == MSG_ERR)
		fprintf(stderr,"ERROR[%d]: No String Found\n", code);
	return code;
}
/* ------------------------------------------------------------ */
/**	postMethod
**
**	Description:
**		Perform HTTP POST with the URL set to receive POST
/* ------------------------------------------------------------ */
int postMethod(char *messageStr)
{
    if (curl)
    {
       curl_easy_setopt(curl, CURLOPT_POSTFIELDS, messageStr);
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

/* ------------------------------------------------------------ */
/**	getMethod
**
**	Description:
**		Perform HTTP GET to retrieve data
/* ------------------------------------------------------------ */
int getMethod()
{
    if (curl)
    {
       curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
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

/* ------------------------------------------------------------ */
/**	putMethod
**
**	Description:
**		Perform HTTP PUT using the custom request from curl
**		with URL to upload the message
/* ------------------------------------------------------------ */
int putMethod(char *messageStr)
{
    if (curl)
    {
       curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
	   curl_easy_setopt(curl, CURLOPT_POSTFIELDS, messageStr);
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

/* ------------------------------------------------------------ */
/**	deleteMethod
**
**	Description:
**		Perform HTTP DELETE using the custom request from curl
/* ------------------------------------------------------------ */
int deleteMethod(char *messageStr)
{
    if (curl)
    {
       curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
	   curl_easy_setopt(curl, CURLOPT_POSTFIELDS, messageStr);
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

/* ------------------------------------------------------------ */
/**	helpMessage
**
**	Description:
**		Provides Help for --help/-h
/* ------------------------------------------------------------ */
void helpMessage()
{
    printf("Options:\n"
  		"\t-u/--url 	<URL>		URL to work with\n"
 		"\t-o/--post 	<string>	Send HTTP POST with string\n"
 		"\t-g/--get 			Send HTTP GET to retrieve data\n"
 		"\t-p/--put	<string>	Send HTTP PUT with string\n"
 		"\t-d/--delete	<string>	Send HTTP Delete with string\n"
 		"\t-h/--help			Help text\n");
	printf("Usage:\n"
 		"\t./hw [options...] <url> <string>\n");
		
	printf("Usage:\n\tSpecial characters must be in double or single quotation marks\n"
				"\tURL must contain http:// or https://\n");

	printf("Examples:\n"
 	 	"\t./hw --post --url http://localhost:8080 here is the message\n"
 		"\t./hw --get --url http://www.cnn.com\n"
 		"\t./hw --put --url http://localhost:8080 putting message!\n"
 		"\t./hw --delete --url http://localhost:8080 78392\n"
		"\t./hw --help\n");
 
}