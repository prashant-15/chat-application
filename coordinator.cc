#include "common.h"
#include "server.h"

void createServer(std::string sessionName);
int check(int exp, const char* msg);

int lastPortNum = 12345;
// Map from chat session name to its port.
std::map<std::string, int> sessionToPortMap;

int main() {

  int server_socket;
  SA_IN server_addr, client_addr;

  //creating socket
  check((server_socket = socket(AF_INET, SOCK_DGRAM, 0)), 
        "Coordinator: Failed to create socket!");
  
  // Initializing address struct.
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(COORDINATOR_PORT);
  
  // Binding the coordinator to the given IP and port.
  check(bind(server_socket, (SA*)&server_addr, sizeof(server_addr)),
        "Bind failed!");
  
  // Buffer to receive the request in.
  char buffer[BUFFSIZE];
  // Length of client_addr.
  socklen_t len = sizeof(client_addr);
  
  std::cout << "Coordinator ready to receive requests..." << std::endl;

  while(true) {
    // Receiving request.
    int msgSize;
    check(msgSize = recvfrom(server_socket, 
                             (char*)buffer, 
                             BUFFSIZE, 
                             MSG_WAITALL,
                             (SA*)&client_addr,
                             &len), "request reception failed");
    std::cout << "Received a request!" << std::endl;
    buffer[msgSize] = '\0';
    std::cout << "Client: " << buffer << std::endl;
    // Converting to string. 
    std::string request(buffer);
    std::string sessionName = "";
    // Response to send to client.
    std::string response = "";
    
    // Checking the operation.
    if(regex_match(request, createRegex)) {
      sessionName = request.substr(request.find('#') + 1);
      std::cout << "Session name = " << sessionName << std::endl;
      // If the session already exists.
      if(sessionToPortMap.find(sessionName) != sessionToPortMap.end()) {
        response = "Can't create! Session exists. Joining the session ...\n";
      }
      else {
        ++lastPortNum;
        createServer(sessionName);
      }
      request = "JOIN #" + sessionName;
    }
    
    if(regex_match(request, joinRegex)) {
      sessionName = request.substr(request.find('#') + 1);
      if(sessionToPortMap.find(sessionName) == sessionToPortMap.end()) {
        response = "ERROR: Chat session with this name does not exist!";
      }
      else {
        // Getting the address of the session server.
        response += "Server address = 127.0.0.1:";
        response += std::to_string(sessionToPortMap[sessionName]);
      }
    }
    
    if(!regex_match(request, joinRegex) 
       && !regex_match(request, createRegex)) {
      response = "ERROR: Invalid Operation.\n";
      response += "Please look above for the correct format.\n";
      response += "For example to create a session named \"ROOM\" :";
      response += "CREATE #ROOM\nAnd to join a session named \"ROOM\" :";
      response += "JOIN #ROOM\n";
    }
    // Replying to client.
    sendto(server_socket, 
           response.c_str(), 
           response.length(), 
           MSG_CONFIRM,
           (const SA*)&client_addr,
           len);
    printf("Message sent!\n");

    // Resetting buffer
    memset(buffer, 0, sizeof(buffer));
  }
  return 0;
}

//-----------------------------------------------------------------------------

void createServer(std::string sessionName) {
  std::cout << "Port num = " << lastPortNum << std::endl;
  int session_socket;
  check((session_socket = socket(AF_INET, SOCK_STREAM, 0)), 
        "Failed to create socket");
  server* serverObj = new server(session_socket, lastPortNum);
  std::cout << "Server object created\n";
  sessionToPortMap[sessionName] = lastPortNum;

  pthread_t sessionServerThread;
  std::cout << "Thread init\n";
  pthread_create(&sessionServerThread, NULL, startServer, serverObj);
  std::cout << "Thread commenced\n";
}

//-----------------------------------------------------------------------------

int check(int exp, const char* msg) {
  if(exp == SOCKETERROR) {
    perror(msg);
    exit(1);
  }
  return exp;
}

//-----------------------------------------------------------------------------