
// udp client driver program 
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdlib.h>

#define PORT 32124
#define MAXLINE 1000

struct dataPacket{
    uint16_t startPacketID;
    uint8_t clientID;
    uint16_t format;
    uint8_t segmentNo;
    uint8_t length;
    char payload[255];
    uint16_t endPacketID;
};

struct rejectPacket{
    uint16_t startPacketID;
    uint8_t clientID;
    uint16_t format;
    uint16_t subCode;
    uint8_t segmentNo;
    uint16_t endPacketID;
};

void printDetails(struct dataPacket data) {

    printf("Sending packet:\n");
    printf("Start of PacketID: %X\n",data.startPacketID);
    printf("Client ID: %X\n",data.clientID);
    printf("Data: %X\n",data.format);
    printf("Segment Number:%d\n",data.segmentNo);
    printf("Length:%d\n",data.length);
    printf("Payload:%s",data.payload);
    printf("End of PacketID:%X\n",data.endPacketID);
}

// Driver code 
int main()
{
    struct dataPacket data;
    struct rejectPacket returnPacket;
    char message[225];
    //char buffer[100];
    //char *message = "Hello Server";
    int sockfd, n;
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
    //setting socket timeout options
    //setsocketopt(socket, level, timeout_value(option_name),option_value, option_length )
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timer,sizeof(struct timeval));


    data.startPacketID = 0XFFFF;
    data.clientID = 0XFF;
    data.format = 0XFFF1;
    data.endPacketID = 0XFFFF;

    filepointer = fopen("/Users/ishwaryarajasekaran/CLionProjects/test_client/Input.txt","rt");
    if (filepointer == NULL)
    {printf("File not found");
     exit(0);
    }
    while(fgets(message, sizeof(message),filepointer)!= NULL) {
        attempt = 0;
        printf("\n");
        retryCounter = 0;
        data.segmentNo = segmentCounter;
        strcpy(data.payload,message);
        data.length = strlen(data.payload);
        data.endPacketID = 0XFFFF;
        //sequence error packet
        if(segmentCounter == 6) {
            data.segmentNo = data.segmentNo + 2;
        }
        // length mismatch error packet
        if( segmentCounter == 7){
            data.length += 5;
        }

        //End of packet missing
        if(segmentCounter == 8){
            data.endPacketID = 0xFFF0;
        }
        // duplicate packet
        if (segmentCounter == 9){
            data.segmentNo = 2;
        }
        printDetails(data);
        while (attempt<= 0 && retryCounter < 3){
        // request to send datagram
        // no need to specify server address in sendto
        // connect stores the peers IP and port
        sendto(sockfd, &data, sizeof(struct dataPacket), 0, (struct sockaddr *) NULL, sizeof(servaddr));

        // waiting for response
        attempt = recvfrom(sockfd, &returnPacket, sizeof(struct rejectPacket), 0, (struct sockaddr *) NULL, NULL);
        if (attempt <= 0){
            printf("No response from server, Sending packet again.\n");
            retryCounter += 1;
        }
        else if(returnPacket.format == 0XFFF2  ){
            printf("ACK packet recieved.\n");
            }
        else if(returnPacket.format == 0XFFF3){
            printf("REJECT packet recieved: ");
            if(returnPacket.subCode == 0XFFF4){
                printf("Packet out of sequence.\n");
            }
            else if(returnPacket.subCode == 0XFFF5){
                printf("Packet Length mismatch.\n");
            }
            else if(returnPacket.subCode == 0XFFF6){
                printf("End of packet missing.\n");
            }
            else if(returnPacket.subCode == 0XFFF7){
                printf("Duplicate Packet.\n");
            }
        }
        }
        if (retryCounter >= 3) {
            printf("Server doesn't respond.\n");
            exit(0);
        }
        segmentCounter += 1;
    }
    // close the descriptor 
    close(sockfd);
    fclose(filepointer);

} 
