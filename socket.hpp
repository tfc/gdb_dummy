/* initial example code from
 * http://www.bogotobogo.com/cplusplus/sockets_server_client.php */
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static void error(const char *msg)
{
    perror(msg);
    exit(1);
}

template <typename F>
static void wait_for_connection(unsigned port, F callback)
{
     int sockfd, newsockfd;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     sockfd =  socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) {
        error("ERROR opening socket");
     }

     int option {1};
     setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

     bzero((char *)&serv_addr, sizeof(serv_addr));

     // server byte order
     serv_addr.sin_family = AF_INET;
     // automatically be filled with current host's IP address
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(port);

     if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
         error("ERROR on binding");
     }

     listen(sockfd, 5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
     if (newsockfd < 0) {
         error("ERROR on accept");
     }

     std::cout << "[Connection from " << inet_ntoa(cli_addr.sin_addr)
               << ":" << ntohs(cli_addr.sin_port) << "]\n";

     callback(newsockfd);

     close(newsockfd);
     close(sockfd);
}
