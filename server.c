//https://www.khmere.com/freebsd_book/src/06/poll_socket.c.html

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <poll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>


/* our defines */
#define PORT             (22000)
#define MAXBUFF          (1024)
#define MAX_CONN         (16)
#define TIMEOUT          (1024 * 1024)
#define MY_MAX(a,b)      (a = (a > b) ? a : b )
#define POLL_ERR         (-1)
#define POLL_EXPIRE      (0)

int main(int argc, char **argv)
{
	int i, j, responsefd = 0, sfds;
	unsigned int len;
	char buff[MAXBUFF];
	struct sockaddr_in sock;
	struct pollfd pfds[MAX_CONN + 1];
	int accepted[MAX_CONN + 1];
	
	memset(buff, 0, MAXBUFF);
	memset(accepted, 0, (MAX_CONN+1) * sizeof(int));
	accepted[0] = 1;

	/*
	 * We will loop through each file descriptor. First,
	 * we will create a socket bind to it and then call 
	 * listen. If we get and error we simply exit, 
	 * which is fine for demo code, but not good in the
	 * real world where errors should be handled properly. 
	 */
	 
	// Listener socket stuff
 	// check to see that we can create them
	if( (sfds = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		perror("Cannot create socket");
		exit(1);
	}
	memset(&sock, 0, sizeof(struct sockaddr_in));
	sock.sin_family = AF_INET;
	sock.sin_port = htons(PORT);
	len = INADDR_ANY;
	memset(&sock.sin_addr, len, sizeof(struct in_addr));
	if( bind(sfds, (struct sockaddr *) &sock, sizeof(struct sockaddr_in)) < 0 )
	{
		perror("Cannot bind to the socket");
		exit(1);
	}
	if( setsockopt(sfds, SOL_SOCKET, SO_REUSEADDR, &j, sizeof(int)) < 0 )
	{
		perror("Cannot set socket options \n");
	}
	if( listen(sfds, 5) < 0 )
	{
		perror("Failed to listen on the socket \n");
	}
	pfds[0].fd = sfds;
	pfds[0].events = POLLIN ;
	
	for(int i = 1; i < MAX_CONN +1; i++)
	{
		pfds[i].events = POLLIN;
	}


	/*
	 * Our main loop. Note, with the poll function we do 
	 * not need to modify our structure before we call 
	 * poll again. Also note that the overall function
	 * is much easier to implement over select.   
	 */
	 int a = 1;
	while(a)
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
					printf("We have a new connection!\n");
					len = sizeof(struct sockaddr_in);
					
					for(i = 1; i < MAX_CONN + 1; i++)
					{
						if(!accepted[i])
						{
							printf("Accepting connection into slot %i.\n", i);
							pfds[i].fd = accept(sfds, (struct sockaddr *)&sock, &len);
							if(pfds[i].fd != -1)
							{
								accepted[i] = 1;
								pfds[0].revents = pfds[0].revents &~POLLIN;
								break;
							}
							else
							{
								printf("Failed.\n");
								break;
							}
						}
						
						if(i == MAX_CONN)
						{
							int responsefd = accept(sfds, (struct sockaddr *)&sock, &len);
							write(responsefd, "Sorry, too many connections\0", 28);
							//pfds[0].revents = pfds[0].revents &~POLLIN;
							close(responsefd);
							a = 0;
						}
					}
				}
				
				for(i =1; i < MAX_CONN+1; i++)
				{
					if(pfds[i].revents & POLLIN && accepted[i])
					{
						len = read(pfds[i].fd, buff, MAXBUFF);
						write(pfds[i].fd, buff, len +1);
						printf("Echoing back:\n %s \n", buff);
						//close(afd);
					}
				}
		} //Switch
	} //While
	return(0);
}



