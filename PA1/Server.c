
// server program for udp connection 
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define PORT 32124
#define MAXLINE 1000

struct dataPacket {
    uint16_t startPacketID;
    uint8_t clientID;
    uint16_t format;
    uint8_t segmentNo;
    uint8_t length;
    char payload[255];
    uint16_t endPacketID;
};

struct rejectPacket {
    uint16_t startPacketID;
    uint8_t clientID;
    uint16_t format;
    uint16_t subCode;
    uint8_t segmentNo;
    uint16_t endPacketID;
};

struct ackPacket {
    uint16_t startPacketID;
    uint8_t clientID;
    uint16_t format;
    uint8_t segmentNo;
    uint16_t endPacketID;
};


struct rejectPacket newRejectPacket(struct dataPacket data) {
    struct rejectPacket reject;
    reject.startPacketID = data.startPacketID;
    reject.clientID = data.clientID;
    reject.segmentNo = data.segmentNo;
    reject.format = 0XFFF3;
    reject.endPacketID = data.endPacketID;
    return reject;
}

// Driver code 
int main()
{   int packetBuffer[10] = {0};
    //char buffer[100];
    struct dataPacket data;
    struct ackPacket ack;
    struct rejectPacket reject;
    int packetCount = 1;

    //char *message = "Hello Ishwarya. Dont cry";
    int listenfd, len,n;
    struct sockaddr_in servaddr, cliaddr;
    bzero(&servaddr, sizeof(servaddr));

    // Create a UDP Socket 
    listenfd = socket(AF_INET, SOCK_DGRAM, 0);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    servaddr.sin_family = AF_INET;

    // bind server address to socket descriptor 
    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    //receive the datagram 
    len = sizeof(cliaddr);
    printf("Server started");
    while(1) {
        n = recvfrom(listenfd, &data, sizeof(data),
                     0, (struct sockaddr *) &cliaddr, &len); //receive message from server
        printf("\n\nReceived packet:\n");
        printf("Start of PacketID: %X \n",data.startPacketID);
        printf("Client ID : %X \n",data.clientID);
        printf("Data: %X \n",data.format);
        printf("Segment Number : %d \n",data.segmentNo);
        printf("Length %d \n",data.length);
        printf("Payload: %s \n",data.payload);
        printf("End of PacketID: %X \n",data.endPacketID);
        packetBuffer[data.segmentNo] += 1;
        int length = strlen(data.payload);
        //buffer[n] = '\0';
        //puts(buffer);
        if (data.segmentNo != packetCount) {
            //create reject packet
            reject = newRejectPacket(data);
            reject.subCode = 0XFFF4;
            sendto(listenfd, &reject, sizeof(struct rejectPacket), 0, (struct sockaddr *)&cliaddr,len);
            printf("Received packet at server is not in sequence with expected packet from client. \n");
        }
            // CASE 2
        else if (data.length != length) {
            //create reject packet
            reject = newRejectPacket(data);
            reject.subCode = 0XFFF5;
            sendto(listenfd, &reject, sizeof(struct rejectPacket), 0, (struct sockaddr *)&cliaddr,len);
            printf("Received packet at server where its length field does not match the length of data in its payload field. \n");
        }
            // CASE 3
        else if (data.endPacketID != 0XFFFF) {
            // create reject packet
            reject = newRejectPacket(data);
            reject.subCode = 0XFFF6;
            sendto(listenfd, &reject, sizeof(struct rejectPacket), 0, (struct sockaddr *)&cliaddr,len);
            printf("Received a packet which does not have the End of Packet Identifier. \n");
        }
            //CASE 4
        else if (packetBuffer[data.segmentNo] != 1) {
            // create reject packet
            reject = newRejectPacket(data);
            reject.subCode = 0XFFF7;
            sendto(listenfd, &reject, sizeof(struct rejectPacket), 0, (struct sockaddr *)&cliaddr,len);
            printf("Received a duplicate packet. \n");
        }
        else{
        // send the response
            ack.startPacketID = data.startPacketID;
            ack.clientID = data.clientID;
            ack.segmentNo = data.segmentNo;
            ack.format = 0XFFF2 ;
            ack.endPacketID = data.endPacketID;
            sendto(listenfd, &ack, sizeof(struct ackPacket), 0,
               (struct sockaddr *) &cliaddr, sizeof(cliaddr));
        }
        packetCount += 1;
        sleep(2);
    }
} 
