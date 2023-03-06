#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <unistd.h>
 
int main(int argc,char **argv)
{
    int sockfd,n;
    char sendline[100];
    char recvline[100];
    struct sockaddr_in servaddr;
 
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&servaddr,sizeof servaddr);
 
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(22000);
 
 	char ServerAddressString[16] = "127.0.0.1";
 	if(argc > 1)
 	{
 		int VarLength = strlen(argv[1]);
 		for(int i = 0; i < 16; i++)
 		{
 			if(i < VarLength)
 			{
 				ServerAddressString[i] = argv[1][i];
 			}
 			else
 			{
 				ServerAddressString[i] = '\0';
 			}
 		}
 	}
   	inet_pton(AF_INET, ServerAddressString, &(servaddr.sin_addr));
    //inet_pton(AF_INET,"192.168.207.248",&(servaddr.sin_addr));
 
    connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
 
    while(1)
    {
        bzero( sendline, 100);
        bzero( recvline, 100);
        fgets(sendline,100,stdin); /*stdin = 0 , for standard input */
 
        write(sockfd,sendline,strlen(sendline)+1);
        read(sockfd,recvline,100);
        printf("%s",recvline);
    }
 
}
