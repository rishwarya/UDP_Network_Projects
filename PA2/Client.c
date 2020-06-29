
// udp client driver program
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdlib.h>

#define PORT 32126
#define MAXLINE 1000

struct callerPacket {
    uint16_t startPacketID;
    uint8_t clientID;
    uint16_t acc_Per;
    uint8_t segmentNo;
    uint8_t length;
    uint8_t technology;
    unsigned int sourceSubno;
    uint16_t endPacketID;
};

struct messagePacket {
    uint16_t startPacketID;
    uint8_t clientID;
    uint16_t format;
    uint8_t segmentNo;
    uint8_t length;
    uint8_t technology;
    unsigned int sourceSubno;
    uint16_t endPacketID;
};

// Driver code
int main()
{
    struct callerPacket caller;
    struct messagePacket message;
    char filedata[225];
    //char buffer[100];
    //char *message = "Hello Server";
    int sockfd;
    struct sockaddr_in servaddr;
    int retryCounter = 0;
    int segmentCounter = 1;
    int attempt = 0;
    FILE *filepointer;


    // clear servaddr
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);
    servaddr.sin_family = AF_INET;

    // create datagram socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // connect to server
    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("\nConnection failed ...\n");
        exit(0);
    }
    //socket timeout
    struct timeval timer;
    timer.tv_sec = 3;
    timer.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timer,sizeof(struct timeval));


    caller.startPacketID = 0XFFFF;
    caller.clientID = 0XFF;
    caller.acc_Per = 0XFFF8;
    caller.endPacketID = 0XFFFF;

    filepointer = fopen("/Users/ishwaryarajasekaran/CLionProjects/PA2_client/Input.txt","rt");
    if (filepointer == NULL)
    {printf("File not found");
        exit(0);
    }
    while(fgets(filedata, sizeof(message),filepointer)!= NULL) {
        attempt = 0;

        printf("\n");
        retryCounter = 0;
        char *data;
        data = strtok(filedata," ");
        caller.sourceSubno = atoi(data);
        data = strtok(NULL," ");
        caller.technology = atoi(data);
        data = strtok(NULL," ");
       // caller.length = strlen(caller.sourceSubno) + strlen(caller.technology);
        caller.segmentNo = segmentCounter;

        printf("Packet Details:\n");
        printf("Start of PacketID: %X\n",caller.startPacketID);
        printf("Client ID: %X\n",caller.clientID);
        printf("Access Permissin: %X\n",caller.acc_Per);
        printf("Segment Number:%d\n",caller.segmentNo);
        printf("Length:%d\n",caller.length);
        printf("Technology:%d\n",caller.technology);
        printf("Source Subscriber Number:%u\n",caller.sourceSubno);
        printf("End of PacketID:%X\n",caller.endPacketID);

        while (attempt<= 0 && retryCounter < 3){
            // request to send datagram
            // no need to specify server address in sendto
            // connect stores the peers IP and port
            sendto(sockfd, &caller, sizeof(struct callerPacket), 0, (struct sockaddr *) NULL, sizeof(servaddr));

            // waiting for response
            attempt = recvfrom(sockfd, &message, sizeof(struct messagePacket), 0, (struct sockaddr *) NULL, NULL);
            if (attempt <= 0){
                printf("No response from server,sending packet again...\n");
                retryCounter += 1;
            }
            else if(message.format == 0XFFFB ){
                printf("Caller permitted access to the network.\n");
            }
            else if(message.format == 0XFFF9){
                printf("Caller not completed the payment.\n");}
            else if(message.format == 0XFFFA){
                    printf("Caller does not exist on the database.\n");
                }
        }
        if (retryCounter >= 3) {
            printf("Server doesn't respond...\n");
            exit(0);
        }
        segmentCounter += 1;
    }
    // close the descriptor
    close(sockfd);
    fclose(filepointer);

}
