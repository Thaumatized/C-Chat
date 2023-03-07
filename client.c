#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <unistd.h>

#include <poll.h>
 
#define PORT             (22000)
#define MAXBUFF          (1024)
#define MAX_CONN         (16)
#define TIMEOUT          (1024 * 1024)
#define POLL_ERR         (-1)
#define POLL_EXPIRE      (0)

int main(int argc,char **argv)
{
	int i, j = 0;
    char buffer[MAXBUFF];
    memset(buffer, 0, MAXBUFF);
    
    struct pollfd pfds[2];
    
    pfds[0].fd = 0; // 0 = stdin
    pfds[0].events = POLLIN;
    pfds[1].events = POLLIN;
    
    struct sockaddr_in servaddr;
    pfds[1].fd = socket(AF_INET,SOCK_STREAM,0);
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
 
    connect(pfds[1].fd,(struct sockaddr *)&servaddr,sizeof(servaddr));
 	while(1)
 	{
 		j = poll(pfds, (unsigned int)MAX_CONN, TIMEOUT);
		switch( j )
		{
			case POLL_EXPIRE:
				printf("Timeout has expired !\n");
				break;                                                    

			case POLL_ERR:
				perror("Error on poll");

			default:  
			
				if(pfds[0].revents & POLLIN)
				{
					fgets(buffer,100,stdin); //stdin = 0
        			write(pfds[1].fd,buffer,strlen(buffer)+1);
				}
				
				if(pfds[1].revents & POLLIN)
				{
						read(pfds[1].fd, buffer, MAXBUFF);
						printf("%s", buffer);
				}
		} //Switch
	} //While
}
