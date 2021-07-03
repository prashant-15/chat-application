#include "common.h"

#define SA struct sockaddr
#define SA_IN struct sockaddr_in

int sockfd, n;
struct sockaddr_in servaddr;
char chatServerAddress[MAXLINE];
std::string username;

void connectToCoordinator();
void connectToChatServer();
void err_n_die(const char* fmt, ...);
void* receiveMsg(void* arg);

int main() {
  // Connect to the coordinator.
  connectToCoordinator();
  // Connect to the chat server at the address received in chatServerAddress. 
  connectToChatServer();
  return 0;
}

//-----------------------------------------------------------------------------

void connectToCoordinator() {
  // Creating socket.
  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    err_n_die("Error while creating socket!");
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(COORDINATOR_PORT);

  if(inet_pton(AF_INET, COORDINATOR_IP, &servaddr.sin_addr) <= 0) {
    err_n_die("inet_pton error for %s", COORDINATOR_IP);
  }
  // Sending options to clients.
  std::cout << "************************************************************\n";
  std::cout << "*                   MESSAGE FORMATS :                      *\n";
  std::cout << "*              1. CREATE #<SESSION_NAME>                   *\n";
  std::cout << "*               2. JOIN #<SESSION_NAME>                    *\n";
  std::cout << "************************************************************\n";
  std::cout << "*         Type \":EXIT\" to leave a chat session.            *\n";
  std::cout << "************************************************************\n";
  while(true) {
    std::cout << ">> ";
    // Getting message from client.
    std::string msg;
    std::getline(std::cin, msg);

    // Sending message to coordinator.
    socklen_t len;
    sendto(sockfd,
           msg.c_str(),
           msg.length(),
           MSG_CONFIRM,
           (const SA*) &servaddr,
           sizeof(servaddr));
  
    // Receiving message from coordinator.
    n = recvfrom(sockfd,
                 (char*)chatServerAddress,
                 MAXLINE,
                 MSG_WAITALL,
                 (SA*)&servaddr,
                 &len);
    chatServerAddress[n] = '\0';
    if(std::string(chatServerAddress).substr(0,5) != "ERROR") {
      // Got a valid reply.
      break;
    }
    else {
      std::cout << chatServerAddress << std::endl;
    }
  }
  std::string response(chatServerAddress);
  if(response.substr(0,12) == "Can't create") {
    std::cout << response.substr(0, response.find('\n') + 1);
  }
  // Terminating.
  close(sockfd);
}

//-----------------------------------------------------------------------------

void connectToChatServer() {
  std::string serverIP(chatServerAddress);
  serverIP = serverIP.substr(serverIP.find('=') + 2, 
                             serverIP.find(':') - serverIP.find('=') - 1 );
  serverIP = serverIP.substr(0, serverIP.length() - 1);
  std::cout << "Server IP: " << serverIP << std::endl;
  std::string port(chatServerAddress);
  port = port.substr(port.find(':') + 1);
  std::cout << "Port address: " << port << std::endl;
  
  // Making the server struct.
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(std::stoi(port));
  struct hostent* host = gethostbyname(serverIP.c_str());
  servaddr.sin_addr.s_addr =
    inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  int status = -1;
  while(status < 0) {
    status = connect(sockfd,
                     (SA*)&servaddr,
                     sizeof(servaddr));
  }
  std::cout << "Connected to server!" << std::endl;
  char buffer[MAXLINE];
  // Receive first message from the server.
  memset(buffer, 0, MAXLINE);
  n = read(sockfd, buffer, MAXLINE - 1);
  buffer[n] = '\0';
  std::string messageReceived(buffer);
  std::cout << messageReceived;
  // Take the username as input.
  getline(std::cin, username);
  // Send the username to the server.
  memset(buffer, 0, sizeof(buffer));
  strcpy(buffer, username.c_str());
  send(sockfd, buffer, strlen(buffer), 0);
  
  // Thread for receiving messages.
  pthread_t receiveThread;
  pthread_create(&receiveThread, NULL, receiveMsg, NULL); 
  
  // Communicate with the server.
  while(true) {
    std::string data;
    getline(std::cin, data);
    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, data.c_str());
    send(sockfd, buffer, strlen(buffer), 0);
    if(data == ":EXIT") {
      std::cout << "Exiting the chat application ...\n";
      std::cout.flush();
      exit(0);
    }
  }
}

//-----------------------------------------------------------------------------

void* receiveMsg(void* arg) {
  char recvline[MAXLINE];
  while(true) {
    memset(recvline, 0, MAXLINE);
    n = read(sockfd, recvline, MAXLINE - 1);
    recvline[n] = '\0';
    std::string messageReceived(recvline);
    std::cout << messageReceived << std::endl;
    std::cout.flush();
  }
  return NULL;
}

//-----------------------------------------------------------------------------

void err_n_die(const char* fmt, ...) {
  int errno_save;
  va_list ap;

  errno_save = errno;

  va_start(ap, fmt);
  vfprintf(stdout, fmt, ap);
  fprintf(stdout, "\n");
  fflush(stdout);

  if(errno_save != 0) {
    fprintf(stdout, "(errno = %d) : %s\n", errno_save, strerror(errno_save));
    fprintf(stdout, "\n");
    fflush(stdout);
  }
  va_end(ap);
  
  // Terminating
  exit(1);
}

//----------------------------------------------------------------------------

/*
#include <sys/socket.h> //basic socket definitions
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>

//standard HTTP port
#define SERVER_PORT 80

#define MAXLINE 4096
#define SA struct sockaddr

void err_n_die(const char *fmt, ...);

int main(int argc, char **argv) {
  int sockfd, n;
  int sendbytes;
  struct sockaddr_in servaddr;
  char sendline[MAXLINE];
  char recvline[MAXLINE];

  //takes IP address as input
  if(argc != 2) {
    err_n_die("usage: %s <server address>", argv[0]);
  }

  //creating socket //re diff
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    err_n_die("Error while creating socket!");
  }

  memset(&servaddr, 0, sizeof(servaddr)); //re diff
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(SERVER_PORT); //chat server

  if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
    err_n_die("inet_pton error for %s", SERVER_PORT);
  }

  if(connect(sockfd, (SA *) & servaddr, sizeof(servaddr)) < 0) {
    err_n_die("connect failed!");
  }

  //connected to server
  sprintf(sendline, "GET / HTTP/1.1\r\n\r\n");
  sendbytes = strlen(sendline);

  //check if all bytes written
  if(write(sockfd, sendline, sendbytes) != sendbytes) {
    err_n_die("write error");
  }

  memset(recvline, 0, MAXLINE);

  while((n=read(sockfd, recvline, MAXLINE - 1)) > 0) {
    printf("%s", recvline);
  }

  if(n<0) {
    err_n_die("read error");
  }

  exit(0); //end successfully
}

void err_n_die(const char* fmt, ...) {
  int errno_save;
  va_list ap;

  //any system or library call can set errno, so saving it in case it changes later
  errno_save = errno;

  va_start(ap, fmt);
  vfprintf(stdout, fmt, ap);
  fprintf(stdout, "\n");
  fflush(stdout);

  if(errno_save != 0) {
    fprintf(stdout, "(errno = %d) : %s\n", errno_save, strerror(errno_save));
    fprintf(stdout, "\n");
    fflush(stdout);
  }
  va_end(ap);
  
  //terminate with error
  exit(1);
}
*/ //re check