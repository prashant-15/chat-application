#ifndef _COMMON_H_
#define _COMMON_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <iostream>
#include <regex>

#define COORDINATOR_IP "127.0.0.1"
#define COORDINATOR_PORT 12345
#define BUFFSIZE 4096
#define MAXLINE 4096
#define SOCKETERROR (-1)
#define SERVER_BACKLOG 100
#define MSG_CONFIRM 0

#define createRegex std::regex("([ ]*)(CREATE)([ ]*)(#)([ -~]+)([ ]*)")
#define joinRegex std::regex("([ ]*)(JOIN)([ ]*)(#)([ -~]+)([ ]*)")

#endif