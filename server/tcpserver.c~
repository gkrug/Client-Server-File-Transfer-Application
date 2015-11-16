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
#include <dirent.h>
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
	char hashrcv[40];
	char packet[100];
        int length = 0;
	time_t rawtime;
	FILE *fp2;
	unsigned char symlink[100];
	char fname[40];
	int length_s[2];
	char file_c[10000000];
	struct tm * timeinfo;
	struct timeval tb, ta;
	int leng = 0;
	double rtt, fsize, throughput;
	unsigned char *computedHash;
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
			if (strcmp(buf, "REQ") == 0) {
				//send(new_s, "go", 3, 0);
				bzero((char*)&buf, sizeof(buf));
//printf("size of buf is %d\n", strlen(buf));
				if ((len=recv(new_s, buf, sizeof(buf), 0)) == -1) {
					perror("Server Receiving Error!\n");
					exit(1);
				} 
				buf[strlen(buf) - 1] = '\0';
				// concat with path to test files
				bzero((char*)&symlink, sizeof(symlink));
				strcpy(symlink,"Prog3TestFiles/");
				strcat(symlink,buf);
				strcpy(buf, symlink);

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
				//printf("size is %d\n", ex[0]);	

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
				// delete command
				else if (strcmp(buf, "DEL") == 0) {
					int fexists;
					if ((len=recv(new_s, buf, sizeof(buf), 0)) == -1) {
					perror("Server Receiving Error!\n");
					exit(1);
				} 
				buf[strlen(buf) - 1] = '\0';
				strcpy(fname, buf);
				fp = fopen(buf, "r");
				// check if file exists
				if (fp == NULL) 
					 fexists = 0;
				else
					 fexists = 1;
				// send whether the file exists or not
				send(new_s, &fexists, 4, 0);
	
				// receive confirmation of delete
				bzero((char *)&buf, sizeof(buf));
				recv(new_s, buf, sizeof(buf), 0);
				
				if (!strcmp(buf, "Yes")) {
					remove(fname);
				} 

				} else if (!strcmp(buf, "UPL")) {
            // ************** UPLOAD ****************
				int ack;

				bzero((char*)&buf, sizeof(buf));
//printf("size of buf is %d\n", strlen(buf));
				if ((len=recv(new_s, buf, sizeof(buf), 0)) == -1) {
					perror("Server Receiving Error!\n");
					exit(1);
				} 
				//buf[strlen(buf) - 1] = '\0';
				//printf("ack check %s\n", buf);
				fp = fopen(buf, "r");
				if (fp != NULL) {
					printf("server already has file--ack 0 to upload request\n");
					ack = 0;
					break;
				} else {
					ack = 1;
				}
						
				// send ack
				send(new_s, &ack, 4, 0);
			
				// receive size of file
				recv(new_s, length_s, sizeof(length_s), 0);
				//printf("size of file is %d\n", length_s[0]);
				// receive hash
				recv(new_s, hashrcv, 33, 0);	
				//printf("hash is %s\n", hashrcv);		
			fp = fopen(buf, "w");			// open requested file to write
				//printf("OPENED: %s\n", buf);
      			if (fp == NULL)
      			{
						printf("fp is null\n");
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
				//printf("boutta receive file\n");
      		while (recv(new_s, file_c, datarcv, 0) > 0 && (remain_data > 0))
      			{
                	fwrite(file_c, sizeof(char), datarcv, fp);
						bzero((char*)&file_c, sizeof(file_c));
                	remain_data -= datarcv;
			if (remain_data < datarcv) {
				datarcv = remain_data;
			}
			if (remain_data <= 0) break;
      		}
			//printf("file received\n");
		gettimeofday(&ta, NULL); // time of day after

		int fileSize;
		rewind(fp);
		fclose(fp);
		//printf("fp2 botta open %s\n", buf);
		// open file received	
		fp2 = fopen(buf, "r");
		if (fp2 == NULL) {
			printf("fp2 is null\n");
			exit(1);	
		}
		
		// Compute hash
				//printf("compute hash\n");
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
				//printf("fill computed hash\n");
      		for (i = 0; i < mhash_get_block_size(MHASH_MD5); i++) {
            		leng += sprintf(send_hash+leng,"%.2x", computedHash[i]);
      		}
		//printf("comparing hashes\n");

		// If the hashes do not match exit
		if ( strcmp(send_hash, hashrcv) != 0) {
			perror("The hash Received does not match the computed hash!\n");
			exit(1);
		}
		//printf("computing rtt\n");
		// Compute Round trip time
		rtt = ((ta.tv_sec - tb.tv_sec)*1000000L +ta.tv_usec) -tb.tv_usec; 
		rtt /= 1000000;
	
	
	fsize = (double) length_s[0]/1000000;		// Size in Mb
	throughput = fsize/rtt;						// Throughput 
	printf("%d bytes transferred in %lf seconds.\nThroughput: %lf Megabytes/sec.\nFile MD5sum: %s\n", length_s[0], rtt, throughput, hashrcv);


				// ************* UPLOAD *****************
				} else if (strcmp(buf, "LIS") == 0)
			{
				DIR *dirp; 
				struct dirent *dp;
				char contents[30];
				dirp = opendir(".");
				while((dp = readdir(dirp)) != NULL) 
				{
                               		sprintf(contents, dp->d_name);
					printf("%s\n", contents);
			        	send(new_s, contents,30, 0);
           			}
				strcpy(contents, "end");
				send(new_s, contents, sizeof(contents), 0);
				closedir(dirp);
			}else if (strcmp(buf, "XIT") == 0) {
					break;
				}
				else {
					printf("not a valid command!\n");
				}
						
		}
		close(new_s);
	}
}
