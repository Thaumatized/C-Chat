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

#define PORT			(22000)
//message type, sender, message, newline and a null character.
#define MAXBUFFER		(1+1+512+1+1)
#define MAX_USER		(16)
#define MAX_NAME		(20)
#define TIMEOUT			(1024 * 1024)
#define POLL_ERR		(-1)
#define POLL_EXPIRE		(0)


//Message types
#define USERNAME		(1)
#define MESSAGE			(2)
#define DISCONNECT		(3)

void SendMessage(int Sender, char* Buffer, int Len);

int accepted[MAX_USER];
struct pollfd pfds[MAX_USER + 1];

int main(int argc, char **argv)
{
	//Connections
	int i, j, responsefd = 0, sfds;
	unsigned int len;
	struct sockaddr_in sock;
	
	memset(accepted, 0, (MAX_USER+1) * sizeof(int));

	char Buffer[MAXBUFFER];
	memset(Buffer, 0, MAXBUFFER);
	
    char UserNames[MAX_USER][MAX_NAME+2]; //+2 = new line and null terminator)
    memset(UserNames, 0, MAX_USER*(MAX_NAME+1));
	 
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
	if(bind(sfds, (struct sockaddr *) &sock, sizeof(struct sockaddr_in)) < 0)
	{
		perror("Cannot bind to the socket");
		exit(1);
	}
	if(setsockopt(sfds, SOL_SOCKET, SO_REUSEADDR, &j, sizeof(int)) < 0)
	{
		perror("Cannot set socket options \n");
	}
	if(listen(sfds, 5) < 0)
	{
		perror("Failed to listen on the socket \n");
	}
	pfds[0].fd = sfds;
	pfds[0].events = POLLIN;
	
	for(int i = 1; i < MAX_USER +1; i++)
	{
		pfds[i].events = POLLIN;
	}

	while(1)
	{
		j = poll(pfds, (unsigned int)MAX_USER, TIMEOUT);
		switch(j)
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
					
					for(i = 0; i < MAX_USER; i++)
					{
						if(!accepted[i])
						{
							printf("Accepting connection into slot %i.\n", i);
							pfds[i+1].fd = accept(sfds, (struct sockaddr *)&sock, &len);
							if(pfds[i+1].fd != -1)
							{
								//send usernames
								Buffer[0] = USERNAME;
								for(int userid = 0; userid < MAX_USER; userid++)
								{
									if(accepted[userid])
									{
										Buffer[1] = userid + 1;
										Buffer[strlen(UserNames[userid])+2] = '\0';
										for(int index = 0; index < strlen(UserNames[userid]); index++)
										{
											Buffer[2 + index] = UserNames[userid][index];
										}
										write(pfds[i+1].fd, Buffer, strlen(UserNames[userid])+3);
									}
								}
							
								accepted[i] = 1;
								break;
							}
							else
							{
								printf("Failed.\n");
								break;
							}
						}
						
						if(i == MAX_USER)
						{
							int responsefd = accept(sfds, (struct sockaddr *)&sock, &len);
							write(responsefd, "Sorry, too many connections\0", 28);
							close(responsefd);
						}
					}
				}
				
				for(i = 0; i < MAX_USER; i++)
				{
                    //has data and is set.
					if(pfds[i+1].revents & POLLIN && pfds[i+1].fd != 0)
					{
						int Len = read(pfds[i+1].fd, Buffer, MAXBUFFER);
						
						if(Len == 0) // no message => Disconnected
						{
							//Unaccept
							accepted [i] = 0;
							pfds[i+1].fd = 0;
							
							//Inform others
							Buffer[0] = DISCONNECT;
							Buffer[1] = i+1; //Userid
							SendMessage(i, Buffer, 2);
							
							continue;
						}
						
						printf("Message from %i: %s", i, Buffer);
						Buffer[1] = i + 1; //Senderid, 1-MAX_USER becuse 0 = null
						SendMessage(i, Buffer, Len);
						printf("Message done\n");
						
						//if USERNAME, save it.
						if(Buffer[0] == USERNAME)
						{
							//If we don't cleanup we might leave a portion of the old name if the new one is shorter
							memset(UserNames[i], 0, MAX_NAME+1);
							//+1 because we want to include the newline, since the client parses it out!
							for(int index = 0; index < MAX_NAME + 1 && index < strlen(&Buffer[2]); index++)
							{
								UserNames[i][index] = Buffer[2 + index];
							}
						}
					}
				}
		} //Switch
	} //While
	return(0);
}

	
void SendMessage(int Sender, char* Buffer, int Len)
{
	switch(Buffer[0])
	{
		case MESSAGE:
			for(int i = 0; i < MAX_USER; i++)
			{
				if(i != Sender && accepted[i])
				{
					printf("Message to %i: %s\n", i, Buffer);
					write(pfds[i+1].fd, Buffer, Len);
				}
			}
			break;
		case USERNAME:
			//Share the new username
			printf("%i is setting their username to %s\n", Sender, Buffer);
			for(int i = 0; i < MAX_USER; i++)
			{
				if(i != Sender && accepted[i])
				{
					printf("Informing %i:\n", i);
					write(pfds[i+1].fd, Buffer, Len);
				}
			}
			break;
		case DISCONNECT:
			//Share the new username
			printf("%i disconnected\n", Sender);
			for(int i = 0; i < MAX_USER; i++)
			{
				if(accepted[i]) //The sender is already unaccepted at this point so we don't have to check for them
				{
					printf("Informing %i:\n", i);
					write(pfds[i+1].fd, Buffer, Len);
				}
			}
			break;
		default:
			break;
	} // Switch
}


