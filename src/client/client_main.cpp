#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <iostream>
#include <sstream>
const size_t MAXSIZE = 4096;
using namespace std;
int main()
{
    int totalPacket = 0, ackPacket = 0;
    int iSockfd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in *sSvrAddr = new sockaddr_in;
    sSvrAddr->sin_family = AF_INET;
    sSvrAddr->sin_addr.s_addr = htonl(INADDR_ANY);
    sSvrAddr->sin_port = htons(9877);
    int n;
    char data[] = {0x01, 0x02};
    char data2[65500];
    for (int i = 0; i < 10000; i++)
    {
        totalPacket++;
        n = sendto(iSockfd, data2, sizeof(data2), 0, (sockaddr *)sSvrAddr, sizeof(*sSvrAddr));
        if (n == -1)
            continue;
        char ackData[1];
        n = recvfrom(iSockfd, ackData, sizeof(ackData), 0, NULL, NULL);
        if (n == -1)
            continue;
        if (ackData[0] == 0x01)
            ackPacket++;
        cout << "ACK/TOTAL: " << ackPacket << "/" << totalPacket << " " << ackPacket * 1.0 / totalPacket << endl;
    }
}
