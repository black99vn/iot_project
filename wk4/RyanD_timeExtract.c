//gcc -o RyanD_timeExtract RyanD_timeExtract.c
/*********************************************************************
 *
 *     Daemon Program: Time Extracting
 *
 *********************************************************************
 * FileName:        RyanD_timeExtract.c
 * 
 *********************************************************************
 * The daemon program must extract the time on one second intervals
 * and write that time to the syslog logging system. 

 * Compile:
 *			gcc -o RyanD_timeExtract RyanD_timeExtract.c
 *
 * Usage:
 *		./RyanD_timeExtract 
 *
 ********************************************************************/
 
 /*--------Libraries-------*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

/*--------Definitions-------*/
#define DAEMON_NAME "timeExtract"
#define OK 			0
#define ERR_FORK 	1
#define ERR_SETSID	2
#define ERR_CHDIR	3
#define ERR_WTF		4
#define ERROR_FORMAT "Error: %s\n"

/*--------Function Declarations--------*/
static void _signal_handler(const int signal);
static void _extract_time();

/* ----------------------------------- */
/*				Main				   */
/* ----------------------------------- */
int main(void)
{
	openlog(DAEMON_NAME, LOG_PID | LOG_NDELAY | LOG_NOWAIT, LOG_DAEMON);
	syslog(LOG_INFO, "extracting timed");
	
	pid_t pid = fork();
	
	if (pid < 0)
	{
		syslog(LOG_ERR, ERROR_FORMAT, strerror(errno));
		return ERR_FORK;
	}
	
	if (pid > 0)
		return OK;

	if (setsid() < 0)
	{
		syslog(LOG_ERR, ERROR_FORMAT, strerror(errno));
		return ERR_SETSID;
	}
	
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	
	umask(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	
	if (chdir("/") < 0)
	{
		syslog(LOG_ERR, ERROR_FORMAT, strerror(errno));
		return ERR_CHDIR;
	}
	
	signal(SIGTERM, _signal_handler);
	signal(SIGHUP, _signal_handler);
	
	_extract_time();
	
	return ERR_WTF;	
}

/*------------------------------------------------------------*/
/*	_signal_handler
/*
/*	Description:
/*		Handle signals and write msg at the syslog
/*------------------------------------------------------------*/
static void _signal_handler(const int signal)
{
	switch(signal)
	{
		case SIGHUP:
			break;
		case SIGTERM:
			syslog(LOG_INFO, "received SIGTERM, exiting.");
			closelog();
			exit(OK);
			break;
		default:
			syslog(LOG_INFO, "received unhandled signal");
	}
}
/*------------------------------------------------------------*/
/*	_extract_time
/*
/*	Description:
/*		Get time every second and write it at the syslog
/*------------------------------------------------------------*/
static void _extract_time()
{
	time_t now;
	struct tm *tm;
		
	while(1)
	{
		time(&now);
		tm = localtime(&now);
		syslog(LOG_INFO, "%02d:%02d:%02d\n",tm->tm_hour, tm->tm_min, tm->tm_sec);
		sleep(1);
	}
}