
// server program for udp connection 
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include<netinet/in.h>
#include <stdlib.h>

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

struct VerificationDetails{
    unsigned int Subno;
    uint8_t  Technology;
    int paid_notPaid;
};


struct messagePacket createMessagepacket(struct callerPacket caller) {
    struct messagePacket message;
    message.startPacketID = caller.startPacketID;
    message.clientID = caller.clientID;
    message.segmentNo = caller.segmentNo;
    message.length = caller.length;
    message.technology = caller.technology;
    message.sourceSubno = caller.sourceSubno;
    message.endPacketID = caller.endPacketID;
    return message;
}

// Driver code 
int main() {
    int verificationBuffer = 0;
    //char buffer[100];
    struct callerPacket caller;
    struct messagePacket message;
    struct VerificationDetails file1[10];
    int packetCount = 1;
    char line[30], *data;
    //int i = 0;
    int status = -1;

    FILE *filepointer;
    filepointer = fopen("/Users/ishwaryarajasekaran/CLionProjects/PA2_server/Verification_Database.txt", "rt");

    if(filepointer == NULL)
    {printf("File not found!\n");
    return 0;}
    while(fgets(line, sizeof(line),filepointer) != NULL)
    {   data = strtok(line," ");
        file1[verificationBuffer].Subno = atol(data);
        data = strtok(NULL," ");
        file1[verificationBuffer].Technology = atoi(data);
        data = strtok(NULL," ");
        file1[verificationBuffer].paid_notPaid = atoi(data);
        verificationBuffer += 1;
    }
    fclose(filepointer);

    //char *message = "Hello Ishwarya. Dont cry";
    int listenfd, len, n;
    struct sockaddr_in servaddr, cliaddr;
    bzero(&servaddr, sizeof(servaddr));

    // Create a UDP Socket 
    listenfd = socket(AF_INET, SOCK_DGRAM, 0);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    servaddr.sin_family = AF_INET;
    // bind server address to socket descriptor 
    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    len = sizeof(cliaddr);
    printf("Server started...");
    while (1) {
        n = recvfrom(listenfd, &caller, sizeof(caller),
                     0, (struct sockaddr *) &cliaddr, &len); //receive message from server
        printf("\n\nReceived Message Details:\n");
        printf("Start of PacketID: %X \n", caller.startPacketID);
        printf("Client ID : %X \n", caller.clientID);
        printf("Access Permission: %X \n", caller.acc_Per);
        printf("Segment Number: %d \n", caller.segmentNo);
        printf("Length %d \n", caller.length);
        printf("Technology:%d \n", caller.technology);
        printf("Subscriber Number:%u\n", caller.sourceSubno);
        printf("End of PacketID: %X \n", caller.endPacketID);


        //buffer[n] = '\0';
        //puts(buffer);
        if (n > 0 && caller.acc_Per == 0XFFF8) {
            for(int i= 0;i<=10;i++) {
                if (caller.sourceSubno == file1[i].Subno && caller.technology == file1[i].Technology) {
                    if (file1[i].paid_notPaid == 1) {
                        status = 1;
                        break;
                    } else {
                        status = 0;
                        break;
                    }
                }
            }

            if (status == 1) {
                message = createMessagepacket(caller);
                message.format = 0XFFF9;
                sendto(listenfd, &message, sizeof(struct messagePacket), 0, (struct sockaddr *) &cliaddr, len);
                printf("Caller has not paid.\n");
            }
            else if (status == 0) {

                message = createMessagepacket(caller);
                message.format = 0XFFFB;
                sendto(listenfd, &message, sizeof(struct messagePacket), 0, (struct sockaddr *) &cliaddr, len);
                printf("Caller permitted to access the network.\n");
            }
            else if (status == -1){
                message = createMessagepacket(caller);
                message.format = 0XFFFA;
                sendto(listenfd, &message, sizeof(struct messagePacket), 0, (struct sockaddr *) &cliaddr, len);
                printf("Caller does not exist on the database.\n");
            }

        }
        status = -1;

    }
} 
