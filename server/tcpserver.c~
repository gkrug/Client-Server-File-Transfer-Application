/*
 * Nick Pellegrino && George Krug
 * npelleg1 && gkrug
 * CSE 30264
 * Programming Assignment 3
 * tcpserver.c
 * This code is a TCP server that uses a port number as input to run the server,
 * and then the server receives a file name from the client, sends back the size
 * of the file (if it exists), sends the MD5 hash of the file, and then sends
 * the file itself
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <mhash.h>
#define MAX_PENDING 5
#define MAX_LINE 256

int main(int argc, char* argv[])
{
	FILE *fp;
	struct sockaddr_in sin;
	char buf[MAX_LINE];
	int len;
	int port;
	int s, new_s,i;
	int opt = 1;
	MHASH td;
        unsigned char buffer;
        unsigned char *hash;
        unsigned char send_hash[256];
	char packet[100];
        int length = 0;
	time_t rawtime;
	FILE *fp2;
	unsigned char symlink[100];
	
	/* ENSURE PROPER ARGUMENT STRUCTURE */
	if (argc == 2)
	{
		port = atoi(argv[1]);
	}
	else
	{
		perror("Needs port number as argument\n");
		exit(1);
	}

	/* FILLING SOCKADDR_IN STRUCT */
	bzero((char*)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);

	/* SOCKET CREATION */
	if((s=socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("simplex-talk:socket");
		exit(1);
	}

	/* ENABLE OPTION TO REUSE SOCKET */
	if((setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)& opt, sizeof(int))) < 0)
	{
		perror("simplex-talk:setsocket\n");
		exit(1);
	}

	/* BIND SOCKET */
	if ((bind(s,(struct sockaddr *)&sin, sizeof(sin))) < 0)
	{
		perror("simplex-talk:bind\n");
		exit(1);
	}

	/* LISTEN TO SOCKET */
	if ((listen(s, MAX_PENDING))<0)
	{
		perror ("simplex-talk:listen");
		exit(1);
	}
	
	while(1)
	{
		if ((new_s = accept(s,(struct sockaddr *)&sin, &len)) < 0)
		{
			perror("simplex-talk:accept");
			exit(1);
		}
		printf("Waiting for operation from client...\n");
		while(1)
		{
			bzero((char*)&buf, sizeof(buf));
			/* ACCEPT AND MAKE NEW SOCKET */
			if ((len=recv(new_s, buf, sizeof(buf),0)) == -1)
			{
				perror("Server Receiving Error!\n");
				exit(1);	
			}
			if (len == 0)
			{
				break;
			}
			if (strcpy(buf, "REQ")) {
				//send(new_s, "go", 3, 0);
				bzero((char*)&buf, sizeof(buf));
//printf("size of buf is %d\n", strlen(buf));
				if ((len=recv(new_s, buf, sizeof(buf), 0)) == -1) {
					perror("Server Receiving Error!\n");
					exit(1);
				} 
				buf[strlen(buf) - 1] = '\0';
				// concat with path to test files
				//bzero((char*)&symlink, sizeof(symlink));
				//strcpy(symlink,"Prog3TestFiles/");
				//strcat(symlink,buf);
				//strcpy(buf, symlink);

				/* OPEN FILE FROM FILE NAME SENT TO SERVER */
				fp = fopen(buf, "r");
				if (fp == NULL)
				{
					int fne[1];
					fne[0] = -1;
					send(new_s,fne,4,0);
				}
				else
				{
					/* COMPUTE AND SEND FILE SIZE */
					int ex[1];
					//fseek(fp, 0L, SEEK_END);
					//ex[0] = ftell(fp);	
					struct stat st;
					stat(buf, &st);
					int sz = st.st_size;
					ex[0] = sz;
					char *file_c = malloc( ex[0] + 4096 );
					send(new_s,ex,sizeof(ex),0);

					/* CALCULATE AND SEND MD5 HASH */
					fp2 = fopen(buf, "r");
        			td = mhash_init(MHASH_MD5);
        			if (td == MHASH_FAILED) exit(1);
        			while (fread(&buffer, 1, 1, fp2) == 1) {
                			mhash(td, &buffer, 1);
        			}
        			hash = mhash_end(td);
				bzero((char*)&send_hash, sizeof(send_hash));
				length = 0;
        			for (i = 0; i < mhash_get_block_size(MHASH_MD5); i++) {
               				length += sprintf(send_hash+length,"%.2x", hash[i]);
        			}
				send(new_s, send_hash, strlen(send_hash)+1, 0);
		
				/* READ CONTENTS OF FILE */		
				rewind(fp);	
				fread(file_c, 1, ex[0], fp);		
				printf("size is %d\n", ex[0]);	

				/* SEND FILE TO CLIENT */
		  		int offset = 0;
		  		int sent_bytes = 5000;
        			int remain_data = ex[0];
				if (remain_data < sent_bytes) {		// send as one packet
					sent_bytes = remain_data;
				}
        			while (((send(new_s, file_c + offset,sent_bytes,0)) > 0) && (remain_data > 0))
        			{
                			remain_data -= sent_bytes;
                			offset += sent_bytes;	// keeping track of sent and remaining data
						if (remain_data < sent_bytes) {
							sent_bytes = remain_data;
						}	
        			}
				} 
				}// end REQ if
				else if (strcmp(command, "DEL")) {
					
				}
				else if (strcmp(command, "XIT")) {
					break;
				}
				else {
					printf("not a valid command!\n");
				}
						
		}
		close(new_s);
	}
}
