/*
 * Nick Pellegrino && George Krug
 * npelleg1 && gkrug
 * CSE 30264
 * Programming Assignment 3
 * tcpclient.c
 * This code is a TCP client that takes the host name, port number, and file name
 * as arguments. The client sends the filename to the server, gets the file's size
 * (if it exists at the server), gets the MD5 and file from the server, computes 
 * its own MD5 once the file transfer is complete, and if the two MD5's match,
 * the client displays file size, transfer time, throughput, and displays the MD5 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <sys/time.h>
#include <mhash.h>
#define MAX_LINE 256

int main(int argc, char * argv[])
{
	FILE *fp;
	FILE *fp2;
	struct hostent *hp;
	struct sockaddr_in sin;
	char *host; 
	char hash[MAX_LINE];
	char command[10];
	char file_c[10000000];
	char buf[MAX_LINE];
	int length[1];
	int s;
	int port;
	int len;
	int r_val;
	double rtt;
	double throughput;
	double fsize;
	struct tm * timeinfo;
	struct timeval tb, ta;
	clock_t start_t, end_t, total_t;
	int fd;
	unsigned char buffer;
	unsigned char *computedHash;
	unsigned char send_hash[4096];
	int leng = 0;
	int i;
	MHASH td;
	
	// Check if the arguments are appropriate
	if (argc==3)
	{
		host = argv[1];
		port = atoi(argv[2]);
	}
	else
	{
		fprintf(stderr, "usage: simplex-talk host\n");
		exit(1);
	}

	// convert to IP
	hp = gethostbyname(host);
	if (!hp)
	{
		fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
		exit(1);
	}
	
	// Fill in Socket
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	bcopy(hp->h_addr, (char*)&sin.sin_addr, hp->h_length);
	sin.sin_port = htons(port);
	
	// Exit if socket can't be created
	if ((s=socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("simplex-talk: socket");
		exit(1);
	}
	
	// Exit if Connection refused
	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		perror("simplex-talk: connect");
		close(s);
		exit(1);
	}
	
	// Set len to the length of the *file name* and send
	//len = strlen(argv[3]) + 1;
	while (1) {
		printf("Please enter command--REQ, UPL, LIS, DEL, XIT: ");
		fgets(command, 50, stdin);
		if (strcmp, "REQ") {
		if (send(s,command,strlen(command) + 1,0)==-1)
		{
			perror("client send error!");
			exit (1);
		}
		//char *t;
		//recv(s, t, 3, 0); 
		bzero((char*)&buf, sizeof(buf));
		printf("Enter name of file to send to server: ");
		fgets(buf, 50, stdin);
		// send name of file to server
		if (send(s, buf, strlen(buf) + 1, 0) == -1) {
			perror("client send error!");
			exit(1);
		}
						buf[strlen(buf) - 1] = '\0';
				printf("buf is %s\n",buf);
		// Receive the size
		recv(s,length,sizeof(length), 0);
	
		// if -1, file doesn't exist and exit
		if (length[0] == -1)
		{
			perror("File does not exist\n");
			exit(1);
		}
	
		// File exists=>Receive file hash from server
		else
		{
			recv(s, hash, 33, 0);			
			fp = fopen(buf, "w");			// open requested file to write
      			if (fp == NULL)
      			{
      				exit(1);
      			}
		
		// prepare to receive blocks from the file
		// set remain_data = size of the file
	   	int remain_data = length[0];
			int datarcv = 5000;
		
			// if file size is less than default receiving block size
			// set equal to size
			if (remain_data < datarcv) {
				datarcv = remain_data;
			}

			// get time of day before file is received
			gettimeofday(&tb, NULL);

			// receive file from server
				bzero((char*)&file_c, sizeof(file_c));
      		while (recv(s, file_c, datarcv, 0) > 0 && (remain_data > 0))
      			{
                	fwrite(file_c, sizeof(char), datarcv, fp);
						bzero((char*)&file_c, sizeof(file_c));
                	remain_data -= datarcv;
			if (remain_data < datarcv) {
				datarcv = remain_data;
			}
			if (remain_data <= 0) break;
      		}
		gettimeofday(&ta, NULL); // time of day after

		int fileSize;
		rewind(fp);
		fclose(fp);
		
		// open file received	
		fp2 = fopen(buf, "r");
		
		// Compute hash
				bzero((char*)&buffer, sizeof(buffer));
				bzero((char*)&computedHash, sizeof(computedHash));
				bzero((char*)&send_hash, sizeof(send_hash));
      		td = mhash_init(MHASH_MD5);
      		if (td == MHASH_FAILED) exit(1);
     	 	while (fread(&buffer, 1, 1, fp2) == 1) {
      			mhash(td, &buffer, 1);
      		}

      		computedHash = mhash_end(td);
				leng = 0;
      		// Fill in computed hash into send_hash
      		for (i = 0; i < mhash_get_block_size(MHASH_MD5); i++) {
            		leng += sprintf(send_hash+leng,"%.2x", computedHash[i]);
      		}
					
		// If the hashes do not match exit
		if ( strcmp(send_hash, hash) != 0) {
			perror("The hash Received does not match the computed hash!\n");
			exit(1);
		}

		// Compute Round trip time
		rtt = ((ta.tv_sec - tb.tv_sec)*1000000L +ta.tv_usec) -tb.tv_usec; 
		rtt /= 1000000;
	}
	
	fsize = (double) length[0]/1000000;		// Size in Mb
	throughput = fsize/rtt;						// Throughput 
	printf("%d bytes transferred in %lf seconds.\nThroughput: %lf Megabytes/sec.\nFile MD5sum: %s\n", length[0], rtt, throughput, hash);
	} else {
		printf("Not a valid command!\n");
		continue;
	}// end REQ if
	} // end while
	printf("Session closed\n");
	close (s);
}

