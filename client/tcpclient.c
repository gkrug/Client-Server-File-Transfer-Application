/*
 * Nick Pellegrino && George Krug
 * npelleg1 && gkrug
 * CSE 30264
 * Programming Assignment 4
 * tcpclient.c
 * This code is a TCP client that is part of an FTP application.
 * It can upload files to the server, download files from server, 
 * list directory contents of the server, delete files from the server,
 * and has an exit feature that closes the client 
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
#include <sys/stat.h>
#define MAX_LINE 256

int main(int argc, char * argv[])
{
	FILE *fp;
	FILE *fp2;
	struct hostent *hp;
	struct sockaddr_in sin;
	char *host; 
	char hashrcv[MAX_LINE];
	char command[10];
	char file_c[10000000];
	char buf[MAX_LINE];
	int length_s[1];
	int length;
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
	unsigned char *hash;
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
	
	while (1) 
	{
		bzero((char*)&buf, sizeof(buf));
		bzero((char*)&command, sizeof(command));
		printf("Please enter command--REQ, UPL, LIS, DEL, XIT: ");
		fgets(command, 50, stdin);
		command[strlen(command) - 1] = '\0';
		if (strcmp(command, "REQ") == 0) {
			if (send(s,command,strlen(command) + 1,0)==-1)
			{
				perror("client send error!");
				exit (1);
			} 
			printf("Enter name of file to download from server: ");
			fgets(buf, 50, stdin);
			
			// send name of file to server
			if (send(s, buf, strlen(buf) + 1, 0) == -1) 
			{
				perror("client send error!");
				exit(1);
			}
			buf[strlen(buf) - 1] = '\0';
	
			// Receive the size
			recv(s,length_s,sizeof(length_s), 0);
	
			// if -1, file doesn't exist and exit
			if (length_s[0] == -1)
			{
				perror("File does not exist\n");
				continue;
			}
	
			// File exists=>Receive file hash from server
			else
			{
				recv(s, hashrcv, 33, 0);			
				fp = fopen(buf, "w");			// open requested file to write
      				if (fp == NULL)
      				{
      					exit(1);
      				}
		
				// prepare to receive blocks from the file
				// set remain_data = size of the file
		   		int remain_data = length_s[0];
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
				if ( strcmp(send_hash, hashrcv) != 0) {
					perror("The hash Received does not match the computed hash!\n");
					exit(1);
				}

				// Compute Round trip time
				rtt = ((ta.tv_sec - tb.tv_sec)*1000000L +ta.tv_usec) -tb.tv_usec; 
				rtt /= 1000000;
			}
	
			fsize = (double) length_s[0]/1000000;		// Size in Mb
			throughput = fsize/rtt;						// Throughput 
			printf("%d bytes transferred in %lf seconds.\nThroughput: %lf Megabytes/sec.\nFile MD5sum: %s\n", length_s[0], rtt, throughput, hashrcv);
		} else if (strcmp(command, "DEL") == 0) {
			int fexists;
			if (send(s,command,strlen(command) + 1,0)==-1)
			{
				perror("client send error!");
				exit (1);
			}
			printf("Enter name of file to delete: ");
			fgets(buf, 40, stdin);
		
			// send name of file to server
			if (send(s, buf, strlen(buf) + 1, 0) == -1) {
				perror("client send error!");
				exit(1);
			}
			buf[strlen(buf) - 1] = '\0';
			//printf("buf to delete is %s\n", buf);
			recv(s, &fexists, 4, 0);
			if (fexists) {
				while (1) {
					printf("'Yes' to delete, 'No' to ignore. ");
					bzero((char*)&buf, sizeof(buf));
					fgets(buf, sizeof(buf), stdin);
					buf[strlen(buf) - 1] = '\0';
					if (!strcmp(buf, "Yes")) {
						send(s, buf, strlen(buf) + 1, 0);
						break;
					} else if (!strcmp(buf, "No")) {
						send(s, buf, strlen(buf) + 1, 0);
						break;
					} else {
						continue;
					}
				}
			} else {
				printf("The file does not exist on the server\n");
				continue;
			}
		} else if (!strcmp(command, "UPL")) {
		// ********* UPlOAD ****************
			int ack;
			if (send(s,command,strlen(command) + 1,0)==-1)
			{
				perror("client send error!");
				exit (1);
			}
			while (1) {
				printf("Enter name of file to upload to server: ");
				fgets(buf, 40, stdin);
				buf[strlen(buf) - 1] = '\0';
			
				fp = fopen(buf, "r");
				if (fp != NULL) {
					break;
				}
			}

	
			// send name of file to server
			if (send(s, buf, strlen(buf) + 1, 0) == -1) {
				perror("client send error!");
				exit(1);
			}
			// receive acknowledgement
			recv(s, &ack, 4, 0);
	
			// break if acknowledge is 0
			if (!ack) {
				printf("Server acknowledges 0 because it already has the file\n");
				continue;
			}
		
			// COMPUTE AND SEND FILE SIZE 
			int ex[1];
			//fseek(fp, 0L, SEEK_END);
			//ex[0] = ftell(fp);	
			struct stat st;
			stat(buf, &st);
			int sz = st.st_size;
			ex[0] = sz;
			char *file_c = malloc( ex[0] + 4096 );
			send(s,ex,sizeof(ex),0);
				
			// CALCULATE AND SEND MD5 HASH 
			fp2 = fopen(buf, "r");
			if (fp2 == NULL ) {
				printf("fp2 is NULL: \n", buf);
			}
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
			//printf("SENDING HASH: %s\n", send_hash);
			send(s, send_hash, strlen(send_hash)+1, 0);
		
			// READ CONTENTS OF FILE //		
			rewind(fp);	
			fread(file_c, 1, ex[0], fp);		
			//printf("size is %d\n", ex[0]);	

			// SEND FILE TO SERVER /
	 		int offset = 0;
		  	int sent_bytes = 5000;
        		int remain_data = ex[0];
			if (remain_data < sent_bytes) {		// send as one packet
				sent_bytes = remain_data;
			}
        		while (((send(s, file_c + offset,sent_bytes,0)) > 0) && (remain_data > 0))
        		{
                		remain_data -= sent_bytes;
                		offset += sent_bytes;	// keeping track of sent and remaining data
					if (remain_data < sent_bytes) {
						sent_bytes = remain_data;
					}	
        		}
			char results[200];
			recv(s, results, 200, 0);
			printf("%s\n", results);		
		} else if (strcmp(command, "LIS") == 0)
		{
			char files[30];
                        if (send(s,command,strlen(command) + 1,0)==-1)
                        {
                                perror("client send error!");
                                exit (1);
                        }
			while (recv(s, files, sizeof(files), 0) > 0)
			{
				if (strcmp(files, "end") != 0)
				{
					printf("%s\n",files);
		//			bzero((char*)files, sizeof(files));
				}
				else
					break;
			}	
		} else if (strcmp(command, "XIT") == 0) {
			break;
		} else {
			printf("Not a valid command!\n");
			continue;
		}
	} // end while
	printf("Session closed\n");
	close (s);
}

