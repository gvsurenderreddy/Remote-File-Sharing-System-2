

#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>
#include<signal.h>
#include<ifaddrs.h>
#define BUFF_SIZE 1000

/////////////////////variables//////////
fd_set readfds, testfds;
int PORTNO = 0; //defines the port number of the client/server
char myip[15]; //stores ip address
//////////////////////////////

void error(char *msg) {
	perror(msg);

}

/*This structure maintains all the information of
 * the clients connected to the SERVER
 */
struct connection {
	int id;
	char host_name[50];
	char ip_address[50];
	unsigned int portno;
	int clientfd;
} table[5];

/*This structure contains the information regarding the
 * peers connected to the client
 * */

struct client_con {
	int con_id;
	char hostn[50];
	char ip[50];
	int confd;
	int port;
} mycon[4];

/*This function is a default initialization for the
 * client which makes all the table id to -1
 * and all connection id to -1
 *
 * */
void client_initialize() {
	int i;
	for (i = 0; i < 5; i++) {
		table[i].id = -1;
		strcpy(table[i].host_name, "NO_HOST_YET");
		strcpy(table[i].ip_address, "NO_HOST_YET");
		table[i].portno = 1000;
	}

	for (i = 1; i < 4; i++) {
		mycon[i].con_id = -1;
		mycon[i].confd = -1;
		strcpy(mycon[i].hostn, "not_yet_added");
	}

}

/*This function is used to display the list sent by the server
 * */
void display_table() {

	int j;
	char dis_host[50];
	bzero(dis_host, 50);
	fprintf(stderr, "\nSERVER TABLE:\n");
	for (j = 0; j < 75; j++)
		fprintf(stderr, "-");

	fprintf(stderr, "\nID\tHOSTNAME\t\t\tIP ADDRESS\t\t PORTNO\n");
	for (j = 0; j < 75; j++)
		fprintf(stderr, "-");
	for (j = 0; j < 5; j++) {
		if (table[j].id != -1) {
			strcpy(dis_host, table[j].host_name);
			while (strlen(dis_host) < 27) {
				strcat(dis_host, " ");
			}
			fprintf(stderr, "\n%d)\t%s\t%s\t\t  %d\n", table[j].id, dis_host,
					table[j].ip_address, table[j].portno);
			memset(dis_host, '\0', 50);
		}
	}
	for (j = 0; j < 75; j++)
		fprintf(stderr, "-");
	fprintf(stderr, "\n\n");

}

/*This function is used by the server to send the list to registered client
 *
 * */

void send_table() {
	int i;
	char buffer[1024];
	char temp[10];
	char seperator[2] = " ";
	bzero(buffer, 1024);
	strcpy(buffer, "send table:");
	for (i = 0; i < 5; i++) {

		strcat(buffer, seperator);
		sprintf(temp, "%d", table[i].id);
		strcat(buffer, temp);
		bzero(temp, 10);
		strcat(buffer, seperator);
		strcat(buffer, table[i].host_name);
		strcat(buffer, seperator);
		strcat(buffer, table[i].ip_address);
		strcat(buffer, seperator);
		sprintf(temp, "%d", table[i].portno);
		strcat(buffer, temp);

	}

	////broadcasting to all connections
	for (i = 1; i < 5; i++) {
		if (table[i].id != -1)
			write(table[i].clientfd, buffer, strlen(buffer));

	}
}

/*reference:http://www.binarytides.com/get-local-ip-c-linux/
 * This function provides the IP address of the current running program */
void myipis() {

	FILE *f;
	char line[100], *p, *c;

	f = fopen("/proc/net/route", "r");

	while (fgets(line, 100, f)) {
		p = strtok(line, " \t");
		c = strtok(NULL, " \t");

		if (p != NULL && c != NULL) {
			if (strcmp(c, "00000000") == 0) {
				// printf("Default interface is : %s \n" , p);
				break;
			}
		}
	}

	//which family do we require , AF_INET or AF_INET6
	int fm = AF_INET;
	struct ifaddrs *ifaddr, *ifa;
	int family, s;
	char host[NI_MAXHOST];

	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}

	//Walk through linked list, maintaining head pointer so we can free list later
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL) {
			continue;
		}

		family = ifa->ifa_addr->sa_family;

		if (strcmp(ifa->ifa_name, p) == 0) {
			if (family == fm) {
				s = getnameinfo(ifa->ifa_addr,
						(family == AF_INET) ?
								sizeof(struct sockaddr_in) :
								sizeof(struct sockaddr_in6), host, NI_MAXHOST,
						NULL, 0, NI_NUMERICHOST);

				if (s != 0) {
					fprintf(stderr, "getnameinfo() failed: %s\n",
							gai_strerror(s));
					exit(EXIT_FAILURE);
				}
				strcpy(myip, host);

			}
		}
	}

	freeifaddrs(ifaddr);

}

/*This function is used by the server to initialize its LIST of connections
 * */
void initialize() {
	int i;
	table[0].id = 1;
	strcpy(table[0].host_name, "timberlake.cse.buffalo.edu");
	strcpy(table[0].ip_address, "128.205.36.8");
	table[0].portno = PORTNO;

	for (i = 1; i < 5; i++) {
		table[i].id = -1;
		strcpy(table[i].host_name, "NO_HOST_YET");
		strcpy(table[i].ip_address, "NO_HOST_YET");
		table[i].portno = 1000;
		table[i].clientfd = 1000;
	}

}

/*this function is used by the server to delete a particular client*/
void remove_client(int sock) {
	int i = 0;
	for (i = 1; i < 5; i++) {
		if (table[i].clientfd == sock)
			break;

	}
	if (table[i].id != -1)
		fprintf(stderr,
				"\n DEREGISTERING CLIENT ID-%d HOSTNAME-%s IPADDRESS-%s PORTNO-%d\n\n ",
				table[i].id, table[i].host_name, table[i].ip_address,
				table[i].portno);

	table[i].id = -1;
	strcpy(table[i].host_name, "NO_HOST_YET");
	strcpy(table[i].ip_address, "NO_HOST_YET");
	table[i].portno = 1000;
	table[i].clientfd = 1000;

}

/*the server uses this function to register a particular client*/

void client_register(char msg[], int sockfd) {
	int i, portno;
	char* token, hostname;
	char ip[15];

	char delimiter[2] = " ";
	token = strtok(msg, ":");
	token = strtok(NULL, delimiter);

	strcpy(ip, token);
	token = strtok(NULL, delimiter);

	portno = atoi(token);

	/////checking for duplicate connection////
	for (i = 1; i < 5; i++) {

		if ((strcmp(table[i].ip_address, ip) == 0)
				&& (table[i].portno == portno)) {
			write(table[i].clientfd, "DUPLICATE CONNECTION", 18);
			return;

		}
	}

	/*****finding empty entry*******/
	for (i = 1; i < 5; i++) {
		if (table[i].id == -1)
			break;
	}
	/*******************************/

	/*******checking if server list full*****/

	if (i == 5) {
		///means table full
		write(sockfd, "SERVER FULL", 40); //if full send message full
		return;

	}

	/*****************************************/

	/*adding the new client to list*/
	table[i].id = i + 1;
	table[i].clientfd = sockfd;
	token = strtok(NULL, delimiter);
	strcpy(table[i].host_name, token);
	strcpy(table[i].ip_address, ip);
	table[i].portno = portno;
	/******************************/

	/*******displaying the updated table********/
	display_table();
	write(sockfd, "REGISTRATION COMPLETE", 40);
	send_table(); //sending the updated table to all existing clients
}

/*this function is used to tell download a file to a corresponding connection*/

int download_file(char fname[], int conid) {

	/////check whether we have that id and if yes then which is it////
	int i;
	for (i = 1; i < 4; i++) {
		if (mycon[i].con_id == conid)
			break;

	}

	if (i == 4) {
		fprintf(stderr, "\n NO SUCH EXISTING CONNECTION\n");
		return;
	}

	else {
		char con_msg[256];
		strcpy(con_msg, "download");
		strcat(con_msg, " ");
		strcat(con_msg, fname);
		strcat(con_msg, " ");
		char temp[16];
		strcat(con_msg, myip);
		strcat(con_msg, " ");
		bzero(temp, 16);
		sprintf(temp, "%d", PORTNO);
		strcat(con_msg, temp);
		strcat(con_msg, " ");

		write(mycon[i].confd, con_msg, strlen(con_msg)); //sending message to the opposite side
														 //to start the upload
	}

}

///*****download file***/
int file_transfer(int clientsock, char f_name[]) {
	char hostnm[50];
	int sock = clientsock;                               //the connection socket
	struct stat buf;
	FILE* fp1;                                                  //file pointer
	int fd;
	int ret, n;
	size_t read;
	struct stat filestat;
	double Tx;  //transmission rate
	struct timeval start, end; //variables to calculate start and end of time
	ret = stat((const char*) f_name, &filestat);
	long long fsize = filestat.st_size; //taking the file size
	long long fs = fsize;

	/**********************************************/
	/*informing the other side to receive the file*/
	/**********************************************/

	char msg[1024];
	bzero(msg, 1024);
	char temp[130];
	strcpy(msg, "upload ");
	strcat(msg, f_name);
	strcat(msg, " ");
	sprintf(temp, "%lld", fsize);
	strcat(msg, temp);
	write(sock, msg, strlen(msg));
	/************************************/
	/************************************/
	char* readBuf = NULL;
	readBuf = (char*) malloc(sizeof(char) * BUFF_SIZE);
	bzero(readBuf, BUFF_SIZE);
	fp1 = fopen(f_name, "rb");

	if (fp1 == NULL) {
		fprintf(stderr, "\nFile not found, or other problem\n");
		return -1;
	}

	gettimeofday(&start, NULL); //getting the time before sending the file

	while (fsize > 0) {
		if (fsize > BUFF_SIZE) {

			n = fread(readBuf, 1, BUFF_SIZE, fp1); //read file in chunks

			n = write(sock, readBuf, BUFF_SIZE); //send file in chunks

			if (n <= 0) {
				fprintf(stderr, "\nError in file tranfer\n");
				write(sock, "ERROR in file transfer", 30);
				return -1;
			}
			fsize = fsize - BUFF_SIZE;
		} else {

			n = fread(readBuf, 1, fsize, fp1);
			//readBuf[n]='\0';

			n = write(sock, readBuf, fsize);
			if (n <= 0) {
				fprintf(stderr, "\nError in file tranfer\n");
				return -1;
			}
			bzero(readBuf, BUFF_SIZE);
			fsize = 0;

		}

	}

	gettimeofday(&end, NULL); //getting the time after sending the file

	fclose(fp1);
	gethostname(hostnm, 50);
	/*finding senders id*/
	int j;
	for (j = 1; j < 4; j++) {
		if (mycon[j].confd == sock)
			break;
	}

	long ttime = (end.tv_sec - start.tv_sec); //getting the difference in start
											  //  and end time i.e. total time

	if (ttime == 0)
		ttime = 1;

	Tx = (fs * 8) + 0.0;
	Tx = Tx / ttime;//converting bytes to bits and dividing by total time to get transmission rate

	/*******************/

	fprintf(stderr,
			"\nTx(%s):%s ---> %s,\nFile Size:%lld Bytes,Time Taken:%ld seconds,Tx Rate:%f bits/second\n",
			hostnm, hostnm, mycon[j].hostn, fs, ttime, Tx);
	free(readBuf);
	fprintf(stderr, "\nFILE SENT\n");
	return 1;

}

/*function to receive the file which is being sent by a connection******/

int file_receive(int sockfd, long long fsize, char* sendfile) {

	char hostnm[50];
	FILE* fp2;

	struct timeval start, end; //variables to calculate start and end of time
	int n;
	double Rx;
	char* token1;
	char* filenm;
	token1 = strtok(sendfile, "/");
	while (token1 != NULL) {
		filenm = token1;
		token1 = strtok(NULL, "/");
	}
	int readf;
	fp2 = fopen(filenm, "wb+");
	char buffer[BUFF_SIZE];
	bzero(buffer, BUFF_SIZE);
	long long remaining = fsize;

	gettimeofday(&start, NULL);
	while (remaining > 0) {

		if (remaining > BUFF_SIZE) {
			readf = (read(sockfd, buffer, BUFF_SIZE));
			if (readf <= 0) {
				fprintf(stderr, "\nError in file tranfer\n");
				return -1;

			}
			n = fwrite(buffer, 1, BUFF_SIZE, fp2);
			remaining = remaining - BUFF_SIZE;
		} else {
			readf = (read(sockfd, buffer, remaining));
			if (readf <= 0) {
				fprintf(stderr, "\nError in file transfer\n");
				return -1;
			}
			n = fwrite(buffer, 1, remaining, fp2);
			remaining = 0;
		}
	}
	gettimeofday(&end, NULL);
	fclose(fp2);
	gethostname(hostnm, 50);
	/*finding senders id*/
	int j;
	for (j = 1; j < 4; j++) {
		if (mycon[j].confd == sockfd)
			break;
	}

	long ttime = (end.tv_sec - start.tv_sec); //getting the difference in start
											  //  and end time i.e. total time

	if (ttime == 0)
		ttime = 1;

	Rx = (fsize * 8) + 0.0;	//converting bytes to bits and dividing by total time to get transmission rate
	Rx = Rx / ttime;

	/*******************/
	//printf("\nFILE SIZE:%lld BYTES,TIME TAKEN:%ld SECS,TRANSFER RATE Tx:%f",fsize,ttime,Rx);
	fprintf(stderr,
			"\nRx(%s):%s ---> %s\nFile Size:%lld Bytes,Time Taken:%ld seconds"
					",Rx Rate:%f bits/second\n", hostnm, mycon[j].hostn, hostnm,
			fsize, ttime, Rx);

	fprintf(stderr, "\nFILE RECIEVED\n");
	return 1;

}

/*help function*/
void help() {
	fprintf(stderr,
			"\n1)MYIP Display the IP address of this process\n"
					"\n2)MYPORT Display the port on which this "
					"process is listening for incoming connections"
					"\n\n3) REGISTER <server IP> <port no>: This command is used by the client to register itself with the server "
					"  and get theIP and listening port numbers of all the peers currently registered with the server"
					"\n\n4)CONNECT <destination> <port no>:This command establishes a new TCP connection to the"
					"specified <destination> at the specified port no.The destination can be ip or hostname"
					"\n\n5)LIST:Displays a numbered list of all connections this process is a part of."
					"\n\n6)Terminate<connection id>:This will terminate the connection listed under"
					"the specified number when LIST is used to display all connections"
					"\n\n7)EXIT:Close all connections and terminate this process"
					"\n\n8)UPLOAD <connection id> <port no>:This will upload a file to the host "
					"on the connection listed under by designated number when LIST is displayed"
					"\n\n9)DOWNLOAD <connection id1> <file1> <connection id2> <file2> <connection id3> <file3>:"
					"This will download a file from each host specified in the command.There can be 1 to 3 hosts"
					"\n\n10)CREATOR:MY NAME,UBIT NAME,UB EMAIL ADDRESS\n\n ");
}

/*function to display a clients existing connections*/

void display_con() {
	int j;
	char dis_host[50];
	fprintf(stderr, "\n\n\n MY CONNECTIONS:\n");
	for (j = 0; j < 80; j++)
		fprintf(stderr, "-");
	fprintf(stderr, "\nID\tHOSTNAME\t\t\t\tIP ADDRESS\t\t PORTNO\n");
	for (j = 0; j < 80; j++)
		fprintf(stderr, "-");
	for (j = 0; j < 4; j++) {
		strcpy(dis_host, mycon[j].hostn);
		while (strlen(dis_host) < 27) {
			strcat(dis_host, " ");
		}
		if (mycon[j].con_id != -1)
			fprintf(stderr, "\n%d)\t%s\t\t%s\t\t%d\n", mycon[j].con_id,
					dis_host, mycon[j].ip, mycon[j].port);
		memset(dis_host, '\0', 50);
	}
	for (j = 0; j < 80; j++)
		fprintf(stderr, "-");
	fprintf(stderr, "\n\n");
}

/*function to read the table sent by server*/

void read_table(int sock, char buffer[]) {
	char temp[10];
	char *token;
	int n, i, first = 1;
	char delimiter[2] = " ";
	token = strtok(buffer, delimiter);
	token = strtok(NULL, delimiter);
	for (i = 0; i < 5; i++) {

		token = strtok(NULL, delimiter);
		if (token != NULL) {
			strcpy(temp, token);
			table[i].id = atoi(temp);
			token = strtok(NULL, delimiter);
			strcpy(table[i].host_name, token);
			token = strtok(NULL, delimiter);
			strcpy(table[i].ip_address, token);
			token = strtok(NULL, delimiter);
			strcpy(temp, token);
			table[i].portno = atoi(temp);

		}
	}
	display_table();
	write(sock, "RECEIVED UPDATED LIST", 50);		         //notify the server

}

/*/*reference:http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
 *function that help to register at the server*/
int register_serv(char ip[], int serverport) {
	/***************server registration****************/
	int sockfd, portno, n;
	struct in_addr ipv4addr;
	// char ip[]="128.205.36.8";
	inet_pton(AF_INET, ip, &ipv4addr);
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char* token;
	char buffer[256];

	/*******adding it to the cleints connection************/
	mycon[0].con_id = 1;
	strcpy(mycon[0].ip, ip);
	strcpy(mycon[0].hostn, "timberlake.cse.buffalo.edu");
	mycon[0].port = serverport;

	/*****************************************************/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	server = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		//exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr,
			server->h_length);
	serv_addr.sin_port = htons(serverport);

	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting");
	myipis();		                                  //caling the myip function
	/********sending message to server*******/
	char send_msg[200];
	strcpy(send_msg, "Register:");
	token = strtok(myip, "\n");
	strcat(send_msg, token);
	strcat(send_msg, " ");
	char temp[10];
	sprintf(temp, "%d", PORTNO);
	strcat(send_msg, temp);
	strcat(send_msg, " ");
	char hostname[50];
	gethostname(hostname, 50);
	strcat(send_msg, hostname);
	strcat(send_msg, " ");
	write(sockfd, send_msg, 100);

	/*******message sent**************************/
	return (sockfd);

	/*done registration*/

}

/////to update connections///
int update_conn(char msg[], int fd) {
	char*token;
	char delimiter[2] = " ";
	char host[30], ip[15];
	int portnum, i, j;
	token = strtok(msg, delimiter);
	token = strtok(NULL, delimiter);
	strcpy(host, token);
	token = strtok(NULL, delimiter);
	portnum = atoi(token);

	int con_flag = 1;
	///check if you are already connected to it
	for (i = 1; i < 4; i++) {
		if ((strcmp(host, mycon[i].hostn) == 0)
				|| (strcmp(host, mycon[i].ip) == 0)) {
			fprintf(stderr, "\nDUPLICATE CONNECTION\n");
			con_flag = 0;
			break;

		}
	}

	//finding the id in table
	if (con_flag != 0) {
		for (i = 0; i < 6; i++) {
			if (((strcmp(table[i].host_name, host) == 0)
					|| (strcmp(table[i].ip_address, host) == 0))
					&& (table[i].portno == portnum))
				break;

		}

		for (j = 1; j < 4; j++) {
			if (mycon[j].con_id == -1)
				break;
		}

		if (j == 4) {

			fprintf(stderr, "\nONLY 3 MAX CONNECTIONS ALLOWED\n");
			return -1;
		}

		strcpy(mycon[j].ip, table[i].ip_address);

		///adding to my connections of client///
		strcpy(mycon[j].hostn, host);
		mycon[j].con_id = j + 1;
		mycon[j].confd = fd;
		mycon[j].port = portnum;

		//////////done with adding in list///////
		fprintf(stderr,
				"\n\nCONNECTION REVCIEVED \nADDED TO CONNECTION \n%d)HOSTNAME %s\n->IPADDRESS:%s\n->PROTNO:%d",
				mycon[j].con_id, mycon[j].hostn, mycon[j].ip, mycon[j].port);
		display_con();
		return 1;

	}
}

void terminate_con(int connectid) {

	int i;
	for (i = 1; i < 4; i++) {

		if (mycon[i].con_id == connectid)
			break;
	}

	if (i == 4)
		fprintf(stderr, "\n NO SUCH CONNECTION");
	else {
		char hostnm[1024], con_msg[1024], temp[10];
		gethostname(hostnm, 1024);
		strcpy(con_msg, "terminate ");
		strcat(con_msg, hostnm);
		strcat(con_msg, " ");
		int myport = PORTNO;
		sprintf(temp, "%d", myport);
		strcat(con_msg, temp);
		strcat(con_msg, " ");
		write(mycon[i].confd, con_msg, strlen(con_msg));
		("\nTERMINATED CONNECTION:\n->HOSTNAME:%s \n->IPADDRESS:%s \n->PORTNO:%d", mycon[i].hostn, mycon[i].ip, mycon[i].port);
		mycon[i].con_id = -1;

		strcpy(mycon[i].hostn, "not_yet_added");

	}
	display_con();

}

/*reference:http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
 * */
int connect_client(char host[], int port) {
	int i, j;
	int sockfd, n;
	struct in_addr ipv4addr;
	char ip[15];
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char con_msg[256], temp[10];
	int con_flag = 1;

/////check for duplicate connections//////
	for (i = 1; i < 4; i++) {
		if (((strcmp(host, mycon[i].hostn) == 0)
				|| (strcmp(host, mycon[i].ip) == 0))
				&& (mycon[i].port == port)) {
			fprintf(stderr, "\nALREADY CONNNECTED\n");
			con_flag = 0;
			break;

		}
	}
	for (i = 0; i < 5; i++) {
		if (((strcmp(table[i].host_name, host) == 0)
				|| (strcmp(table[i].ip_address, host) == 0))
				&& (table[i].portno == port))
			break;

	}

	char hostnw[50];
	gethostname(hostnw, 50);
	/*check for self connection*/
	if ((strcmp(table[i].host_name, hostnw) == 0)
			&& table[i].portno == PORTNO) {
		fprintf(stderr, "\nSELF CONNECTION\n");
		con_flag = 0;

	}

//finding the id in table
	if (con_flag != 0) {

		for (j = 1; j < 4; j++) {
			if (mycon[j].con_id == -1)
				break;
		}

		if (j == 4) {

			fprintf(stderr, "\nONLY 3 MAX CONNECTIONS ALLOWED\n");
			return -1;
		}

		strcpy(ip, table[i].ip_address);

		///////////////////////////////////////

		inet_pton(AF_INET, ip, &ipv4addr);
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
			error("ERROR opening socket");
		server = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
		if (server == NULL) {
			fprintf(stderr, "ERROR, no such host\n");

			return -1;
		}
		bzero((char *) &serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr,
				server->h_length);
		serv_addr.sin_port = htons(port);

		if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))
				< 0)
			error("ERROR connecting");
		/*saving the socket fd*/
		mycon[j].confd = sockfd;

		char hostnm[50];
		gethostname(hostnm, 50);
		strcpy(con_msg, "connect ");
		strcat(con_msg, hostnm);
		strcat(con_msg, " ");
		int myport = PORTNO;
		sprintf(temp, "%d", myport);
		strcat(con_msg, temp);
		strcat(con_msg, " ");

		///adding to my connections of client///
		strcpy(mycon[j].hostn, table[i].host_name);
		strcpy(mycon[j].ip, table[i].ip_address);
		mycon[j].con_id = j + 1;
		mycon[j].port = port;

		//////////done with adding in list///////
		fprintf(stderr,
				"\nADDED TO CONNECTION \n%d)HOSTNAME %s\n->IPADDRESS:%s\n->PROTNO:%d",
				mycon[j].con_id, mycon[j].hostn, mycon[j].ip, mycon[j].port);
		//////////done with adding in list///////

		write(sockfd, con_msg, strlen(con_msg));

		display_con();
		return (sockfd);
	}

	else {

		return -1;
	}
}

void sparse_command(char option[]) {
	char *token;
	const char s[2] = " ";
	//parsing work

	token = strtok(option, s);
	if ((strcmp(token, "REGISTER") == 0) || (strcmp(token, "register") == 0)) {

		fprintf(stderr, "\nOPTION NOT ALLOWED ON SERVER\n");
		return;
	}

	else if ((strcmp(token, "CONNECT") == 0)
			|| (strcmp(token, "connect") == 0)) {
		fprintf(stderr, "\nOPTION NOT ALLOWED ON SERVER\n");
		return;
	}

	else if ((strcmp(token, "UPLOAD") == 0) || (strcmp(token, "upload") == 0)) {
		fprintf(stderr, "\nOPTION NOT ALLOWED ON SERVER\n");
		return;
	}

	else if ((strcmp(token, "DOWNLOAD") == 0)
			|| (strcmp(token, "download") == 0)) {
		fprintf(stderr, "\nOPTION NOT ALLOWED ON SERVER\n");
		return;
	}

	else if ((strcmp(token, "TERMINATE") == 0)
			|| (strcmp(token, "terminate") == 0)) {
		fprintf(stderr, "\nOPTION NOT ALLOWED ON SERVER\n");
		return;
	}

	else if ((strcmp(token, "EXIT") == 0) || (strcmp(token, "exit") == 0)) {

		fprintf(stderr, "\nEXITING.....\n");
		exit(1);

	}

	else if ((strcmp(token, "MYPORT") == 0) || (strcmp(token, "myport") == 0)) {

		fprintf(stderr, "\nMY PORT IS:%d\n\n", PORTNO);

	} else if ((strcmp(token, "LIST") == 0) || (strcmp(token, "list") == 0)) {

		display_con();

	}

	else if ((strcmp(token, "CREATOR") == 0)
			|| (strcmp(token, "creator") == 0)) {
		fprintf(stderr,
				"\nCREATOR:Pratik Gajanan Deshpande\npdeshpan\npdeshpan@buffalo.edu\n");

	}

	else if ((strcmp(token, "MYIP") == 0) || (strcmp(token, "myip") == 0)) {
		myipis();
		fprintf(stderr, "\nMY IP IS:%s\n", myip);

	} else if ((strcmp(token, "HELP") == 0) || (strcmp(token, "help") == 0)) {
		help();

	} else {
		fprintf(stderr, "\nINCORRECT COMMAND PLEASE TYPE 'HELP' IF REQUIRED\n");
	}

}

void parse_command(char option[]) {
	int registerfd, consock;
	char server_ip[15], dest[100], filename[100], file1[100], file2[100],
			file3[100];

	char *token;
	char para1[50], para2[50], para3[50];
	int conid1 = 0, conid2 = 0, conid3 = 0, server_port, con_port, tid, uid;
	const char s[2] = " ";
	//parsing work

	token = strtok(option, s);
	if ((strcmp(token, "REGISTER") == 0) || (strcmp(token, "register") == 0)) {

		token = strtok(NULL, s);
		if (token == NULL) {
			fprintf(stderr,
					"\nINCORRECT INPUT TYPE 'HELP' FOR MORE INFORMATION\n");
			return;
		}
		strcpy(para1, token);
		token = strtok(NULL, s);
		if (token == NULL) {
			fprintf(stderr,
					"\nINCORRECT INPUT TYPE 'HELP' FOR MORE INFORMATION\n");
			return;
		}
		strcpy(para2, token);

		server_port = atoi(para2);

		strcpy(server_ip, para1);
		if (strcmp("128.205.36.8", server_ip) != 0) {
			fprintf(stderr, "\nSERVER IP INCORRECT\n");

		} else {

			registerfd = register_serv(server_ip, server_port);
			FD_SET(registerfd, &readfds);

		}
	} else if ((strcmp(token, "CONNECT") == 0)
			|| (strcmp(token, "connect") == 0)) {
		token = strtok(NULL, s);
		if (token == NULL) {
			fprintf(stderr,
					"\nINCORRECT INPUT TYPE 'HELP' FOR MORE INFORMATION\n");
			return;
		}
		strcpy(dest, token);
		token = strtok(NULL, s);
		if (token == NULL) {
			fprintf(stderr,
					"\nINCORRECT INPUT TYPE 'HELP' FOR MORE INFORMATION\n");
			return;
		}
		con_port = atoi(token);

		consock = connect_client(dest, con_port);
		if (consock == -1)
			fprintf(stderr, "\nIMPROPER CONNECTION\n");
		else
			FD_SET(consock, &readfds);

	}

	else if ((strcmp(token, "UPLOAD") == 0) || (strcmp(token, "upload") == 0)) {
		token = strtok(NULL, s);
		if (token == NULL) {
			fprintf(stderr,
					"\nINCORRECT INPUT TYPE 'HELP' FOR MORE INFORMATION\n");
			return;
		}
		uid = atoi(token);
		token = strtok(NULL, s);
		if (token == NULL) {
			fprintf(stderr,
					"\nINCORRECT INPUT TYPE 'HELP' FOR MORE INFORMATION\n");
			return;
		}
		strcpy(filename, token);
		fprintf(stderr, "\nUPLOADING FILE %s TO CONNECTION %d", filename, uid);

		if (uid == 1) {
			fprintf(stderr, "\nUPLOAD TO SERVER NOT ALLOWED\n");

		}

		else {
			int i;
			for (i = 1; i < 4; i++) {
				if (mycon[i].con_id == uid)
					break;

			}
			if (i == 4) {
				fprintf(stderr, "\nNO SUCH CONNECTION\n");
			}

			else {

				int val = file_transfer(mycon[i].confd, filename);
				if (val == -1)
					fprintf(stderr, "\n\nERROR IN FILE TRANSFER\n\n");

			}

		}
	}

	else if ((strcmp(token, "DOWNLOAD") == 0)
			|| (strcmp(token, "download") == 0)) {
		token = strtok(NULL, s);
		if (token == NULL) {
			fprintf(stderr,
					"\nINCORRECT INPUT TYPE 'HELP' FOR MORE INFORMATION\n");
			return;
		}
		conid1 = atoi(token);
		token = strtok(NULL, s);
		if (token == NULL) {
			fprintf(stderr,
					"\nINCORRECT INPUT TYPE 'HELP' FOR MORE INFORMATION\n");
			return;
		}
		strcpy(file1, token);
		token = strtok(NULL, s);
		if (token != NULL) {
			conid2 = atoi(token);
			token = strtok(NULL, s);
			if (token != NULL) {
				strcpy(file2, token);
				token = strtok(NULL, s);
				if (token != NULL) {
					conid3 = atoi(token);
					token = strtok(NULL, s);
					if (token != NULL) {
						strcpy(file3, token);
					} else {
						fprintf(stderr, "\nINCORRECT COMMAND\n");
					}
				}
			} else {
				fprintf(stderr, "\nINCORRECT COMMAND");
			}

		}

		if ((conid1 == 1) || (conid2 == 1) || (conid3 == 1)) {
			fprintf(stderr, "\nDOWNLOAD FROM SERVER NOT ALLOWED\n");
			if (conid1 == 1)
				conid1 = 0;
			else if (conid2 == 1)
				conid2 = 0;
			else
				conid3 = 0;
		}

		if (conid1 != 0) {
			fprintf(stderr, "\nDOWNLOADING FILE %s FROM CONNECTION %d\n", file1,
					conid1);
			int val = download_file(file1, conid1);
			if (val == -1)
				fprintf(stderr, "ERROR IN FILE TRNSFER %s", file1);
		}
		if (conid2 != 0) {
			fprintf(stderr, "\nDOWNLOADING FILE %s FROM CONNECTION %d\n", file2,
					conid2);
			int val = download_file(file2, conid2);
			if (val == -1)
				fprintf(stderr, "ERROR IN FILE TRNSFER %s", file1);
		}
		if (conid3 != 0) {
			fprintf(stderr, "\nDOWNLOADING FILE %s FROM CONNECTION %d\n", file3,
					conid3);
			int val = download_file(file3, conid3);
			if (val == -1)
				fprintf(stderr, "ERROR IN FILE TRNSFER %s", file1);
		}

	}

	else if ((strcmp(token, "TERMINATE") == 0)
			|| (strcmp(token, "terminate") == 0)) {
		token = strtok(NULL, s);
		if (token == NULL) {
			fprintf(stderr,
					"\nINCORRECT INPUT TYPE 'HELP' FOR MORE INFORMATION\n");
			return;
		}
		tid = atoi(token);
		fprintf(stderr, "\n TERMINATING CONNECTION ID %d\n", tid);
		terminate_con(tid);

	} else if ((strcmp(token, "EXIT") == 0) || (strcmp(token, "exit") == 0)) {

		fprintf(stderr, "\nEXITING.....\n");
		exit(1);

	}

	else if ((strcmp(token, "MYPORT") == 0) || (strcmp(token, "myport") == 0)) {

		fprintf(stderr, "\nMY PORT IS:%d\n", PORTNO);

	} else if ((strcmp(token, "LIST") == 0) || (strcmp(token, "list") == 0)) {

		display_con();

	}

	else if ((strcmp(token, "CREATOR") == 0)
			|| (strcmp(token, "creator") == 0)) {
		fprintf(stderr,
				"\nCREATOR:Pratik Gajanan Deshpande\npdeshpan\npdeshpan@buffalo.edu\n");

	}

	else if ((strcmp(token, "MYIP") == 0) || (strcmp(token, "myip") == 0)) {
		myipis();
		fprintf(stderr, "\nMY IP IS:%s\n", myip);

	} else if ((strcmp(token, "HELP") == 0) || (strcmp(token, "help") == 0)) {
		help();

	} else {
		fprintf(stderr, "\nINCORRECT COMMAND PLEASE TYPE 'HELP' IF REQUIRED\n");
	}

}

/*reference:http://snap.nlc.dcccd.edu/learn/fuller3/chap13/chap13.html
 * main client code*/

void client() {

	int consock, registerfd, server_sockfd, client_sockfd;
	int server_len, client_len;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	int result;
	char option[1024];

	// consock=connect_client(2);
	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(PORTNO);
	server_len = sizeof(server_address);
	bind(server_sockfd, (struct sockaddr *) &server_address, server_len);
	listen(server_sockfd, 5);

	//initializing readfds to sockfd.
	FD_ZERO(&readfds);

	FD_SET(server_sockfd, &readfds);

	FD_SET(0, &readfds);

	while (1) // waiting for client requests
	{

		int fd;

		int nread;
		char ch[1024];

		testfds = readfds;

		result = select(FD_SETSIZE, &testfds, (fd_set *) 0, (fd_set *) 0,
				(struct timeval *) 0);
		if (result <= 0) {
			perror("Error");
			exit(1);
		}

		if (FD_ISSET(0, &testfds)) {
			memset(option, '\0', 1024);
			fgets(option, 1024, stdin);
			option[strlen(option) - 1] = '\0';
			parse_command(option);

		}

		else {

			for (fd = 0; fd < FD_SETSIZE; fd++) {
				if (FD_ISSET(fd, &testfds)) {
					if (fd == server_sockfd)

					{
						client_sockfd = accept(server_sockfd,
								(struct sockaddr *) &client_address,
								&client_len);

						FD_SET(client_sockfd, &readfds);

					} else {
						memset(ch, 0, 100);
						ioctl(fd, FIONREAD, &nread);
						if (nread == 0 && fd != 0)

						{
							close(fd);
							int j;
							////finding the fd in client connection////
							for (j = 1; j < 4; j++) {

								if (mycon[j].confd == fd) {
									break;
								}
							}

							////terminate the connection
							terminate_con(mycon[j].con_id);
							FD_CLR(fd, &readfds);

							display_con();

						} else {
							char temp1[1024];
							memset(ch, 0, 1024);
							memset(temp1, 0, 1024);
							read(fd, &ch, sizeof(ch));
							strcpy(temp1, ch);
							char *reader;
							reader = strtok(temp1, " ");

							if (reader != NULL) {

								if (strcmp(reader, "send") == 0) {
									read_table(fd, ch);
									memset(ch, 0, 1024);

								}

								else if (strcmp(reader, "connect") == 0) {

									update_conn(ch, fd);
									memset(ch, 0, 1024);

								} else if (strcmp(reader, "ERROR") == 0) {

									fprintf(stderr, "\n%s\n", ch);
									memset(ch, 0, 1024);

								} else if (strcmp(reader, "upload") == 0) {
									char filenm[256];
									bzero(filenm, 256);
									char* tk;
									long long files;
									tk = strtok(NULL, " ");

									strcpy(filenm, tk);

									tk = strtok(NULL, " ");
									files = atoll(tk);

									int val = file_receive(fd, files, filenm);
									if (val == -1)
										fprintf(stderr,
												"\n\nERROR IN FILE TRANSFER\n\n");
									memset(ch, 0, 1024);

								} else if (strcmp(reader, "terminate") == 0) {

									char* temp2;
									char ht[50];
									int pt, i;
									temp2 = strtok(NULL, " ");
									strcpy(ht, temp2);
									temp2 = strtok(NULL, " ");
									pt = atoi(temp2);
									for (i = 1; i < 4; i++) {
										if (((strcmp(mycon[i].hostn, ht) == 0))
												&& (mycon[i].port == pt))
											break;

									}

									mycon[i].con_id = -1;
									close(mycon[i].confd);
									FD_CLR(mycon[i].confd, &readfds);
									display_con();
									memset(ch, 0, 1024);

								} else if (strcmp(reader, "download") == 0) {
									char dfile[256], conip[16];
									bzero(dfile, 256);
									char* toknize;
									int dfd, port;
									toknize = strtok(NULL, " ");
									strcpy(dfile, toknize);
									toknize = strtok(NULL, " ");
									strcpy(conip, toknize);
									toknize = strtok(NULL, " ");
									port = atoi(toknize);
									int j;
									for (j = 1; j < 4; j++) {

										if ((strcmp(mycon[j].ip, conip) == 0)
												&& (mycon[j].port == port))
											break;
									}
									int val = file_transfer(mycon[j].confd,
											dfile);
									if (val == -1) {
										fprintf(stderr,
												"\n\nERROR IN FILE TRANSFER\n\n");
										write(mycon[j].confd, "ERROR IN FILE",
												30);
									}
									memset(ch, 0, 1024);

								}
							} else {

								memset(ch, '\0', 1024);
								memset(temp1, '\0', 1024);
								reader = NULL;

							}
						}
					}

				}
			}
		}
	}
}

/*reference:http://snap.nlc.dcccd.edu/learn/fuller3/chap13/chap13.html
 * main server code*/

void server() {
	initialize();
	int server_sockfd, client_sockfd;
	int server_len, client_len;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	int result;
	char option[1024];

	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(PORTNO);
	server_len = sizeof(server_address);
	bind(server_sockfd, (struct sockaddr *) &server_address, server_len);
	listen(server_sockfd, 5);
// Creating a queue of connections and initializing readfds to sockfd.
	FD_ZERO(&readfds);
	FD_SET(server_sockfd, &readfds);
	FD_SET(0, &readfds);

	while (1) // waiting for client requests
	{

		int fd;

		int nread;
		char ch[1024];

		testfds = readfds;

		result = select(FD_SETSIZE, &testfds, (fd_set *) 0, (fd_set *) 0,
				(struct timeval *) 0);
		if (result <= 0) {
			perror("Error");
			exit(1);
		}

		if (FD_ISSET(0, &testfds)) {

			fgets(option, 1024, stdin);
			option[strlen(option) - 1] = '\0';
			sparse_command(option);

		}

		else {

			for (fd = 0; fd < FD_SETSIZE; fd++) {
				if (FD_ISSET(fd, &testfds)) {
					if (fd == server_sockfd)

					{
						client_sockfd = accept(server_sockfd,
								(struct sockaddr *) &client_address,
								&client_len);

						FD_SET(client_sockfd, &readfds);

					}

					else {
						memset(ch, 0, 1024);
						ioctl(fd, FIONREAD, &nread);
						if (nread == 0)

						{
							remove_client(fd);
							close(fd);
							FD_CLR(fd, &readfds);
							send_table();

						} else {

							read(fd, &ch, sizeof(ch));
							char temp[1024];
							bzero(temp, 1024);
							strcpy(temp, ch);
							char* reader2;
							reader2 = strtok(temp, ":");

							/**registering the client***/
							if (strstr(ch, "Register") != NULL) {

								client_register(ch, fd);
								memset(ch, 0, 1024);

							}

							else {

								bzero(ch, 1024);
								bzero(temp, 1024);
							}
						}

					}

				}
			}

		}
	}
}

int main(int argc, char *argv[]) {
	// Variables Declaration

	int i = 0;
	int flag = 0;
	if (argc != 3) {
		fprintf(stderr, "\n Invalid input...Redirecting to help file.\n");
		help();
		exit(1);
	}

	PORTNO = atoi(argv[2]);

	//  Check if it is client or server
	if (strcmp(argv[1], "s") == 0) {
		fprintf(stderr, "\nSERVER...\n");
		server(); // This will call the main server function

	} else if (strcmp(argv[1], "c") == 0) {

		client_initialize();
		fprintf(stderr, "\nCLIENT...\n");
		client(); // This will call the client function

	} else {
		fprintf(stderr, "\n Invalid input...Redirecting to help file.\n");
		help();
		exit(1);
	}

}

