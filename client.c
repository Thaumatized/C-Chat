#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <unistd.h>

#include <poll.h>
 
#define PORT			(22000)
#define MAXBUFF			(1024)
#define MAX_USER		(16)
#define MAX_NAME		(20)
#define TIMEOUT			(1024 * 1024)
#define POLL_ERR		(-1)
#define POLL_EXPIRE		(0)

//Message types
#define USERNAME		(1)
#define MESSAGE			(2)

int main(int argc,char **argv)
{
	int i, j = 0;
    char buffer[2 + MAXBUFF];
    memset(buffer, 0, MAXBUFF);
    
    char UserNames[MAX_USER][MAX_NAME+1]; // +1 = null terminator
    memset(UserNames, 0, MAX_USER*(MAX_NAME+1));
    
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
 
    connect(pfds[1].fd,(struct sockaddr *)&servaddr,sizeof(servaddr));
    
    printf("Set user name:\n");
	fgets((buffer + 2),MAX_NAME,stdin); //stdin = 0
	buffer[0] = USERNAME;
	buffer[1] = 1; // 1 = not null. This is here to make the server have a spot to mark userid.
	write(pfds[1].fd,buffer,strlen(buffer)+1);
    
 	while(1)
 	{
 		j = poll(pfds, 2u, TIMEOUT);
		switch( j )
		{
			case POLL_EXPIRE:
				printf("Timeout has expired !\n");
				break;                                                    

			case POLL_ERR:
				perror("Error on poll");

			default:  
			
				//Send message
				if(pfds[0].revents & POLLIN)
				{
					fgets((buffer + 2),MAXBUFF,stdin); //stdin = 0
					buffer[0] = MESSAGE;
					buffer[1] = 1; // 1 = not null. This is here to make the server have a spot to mark userid.
        			write(pfds[1].fd,buffer,strlen(buffer)+1);
				}
				
				//Receive message
				if(pfds[1].revents & POLLIN)
				{
						memset(buffer, 0, MAXBUFF+2);
						read(pfds[1].fd, buffer, MAXBUFF+2);
						
						switch(buffer[0])
						{
							case MESSAGE:
								printf("%s: %s", UserNames[buffer[1]-1],  &(buffer[2]));
								break;
							case USERNAME: ;
								int index = 0;
								while(index < MAXBUFF+2 && buffer[index] != 0)
								{
									printf("%i set username to %s", buffer[index + 1], &(buffer[index + 2]));
									//this leaves out the newline
									for(int i = 0; i < MAX_NAME && i < strlen(&buffer[index + 2])-1; i++)
									{
										UserNames[buffer[index + 1]-1][i] = buffer[index + 2 + i];
									}
									index += strlen(buffer + index)+1;
								}
								
								break;
							default:
								break;
						}
				}
		} //Switch
	} //While
}
