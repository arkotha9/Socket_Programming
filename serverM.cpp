#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>

#define TCP_PORT "25242"

#define UDP_PORT 24242
#define UDP_PORT_C 21242
#define UDP_PORT_CS 22242
#define UDP_PORT_EE 23242

#define BACKLOG 10
#define MAXSIZE 50

using namespace std;

void* get_in_addr(struct sockaddr);
char encrypt(char);

int main(){
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo, *res; // will point to the results
    struct sockaddr_storage their_addr; // connectors address information
    socklen_t addr_size;

    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE; // fill in my IP for me

    status = getaddrinfo(NULL, TCP_PORT, &hints, &servinfo);

    if ((status) != 0){
      fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
      exit(1);
    }
    //tcp socket

    // servinfo now points to a linked list of 1 or more struct addrinfos
    // initialize parent socket sockdp
    int sockdp = -1;
    for (res = servinfo; res!= NULL; res = servinfo->ai_next){
        sockdp = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockdp < 0) {
            continue;
        }
        break;  /* okay we got one */
    }


    freeaddrinfo(servinfo); // free the linked-list
       
    if (sockdp < 0) {
        perror("socket creation failed\n"); 
        exit(EXIT_FAILURE); 
        /*NOTREACHED*/
    }
    cout << "The main server is up and running." << endl;
    //bind the tcp socket
    int status_bind = bind(sockdp, res->ai_addr, res->ai_addrlen);
    if(status_bind < 0){
        perror("socket binding failed\n"); 
        exit(EXIT_FAILURE); 
    }

    // listen to the client
    int status_listen = listen(sockdp,BACKLOG);
    if(status_listen < 0){
        perror("socket listening failed\n"); 
        exit(EXIT_FAILURE); 
    }
    // accept the request TCP and create child socket
    addr_size = sizeof(their_addr);
    int sockdp_ch = accept(sockdp, (struct sockaddr *)&their_addr, &addr_size);
    if(sockdp_ch < 0){
        perror("connection request denied\n"); 
        exit(EXIT_FAILURE); 
    } 

    //udp socket for serverC, serverCS, and serverEE
    //udp for serverC
    int serversocketC;

	char auth_result[2];        //change names
	struct sockaddr_in servCaddr;

	int n;
	unsigned int len;
	if ((serversocketC = socket(AF_UNSPEC, SOCK_DGRAM, 0)) < 0 ){
	    printf("\nsocket C creation failed\n");
	    exit(0);
	}

	memset(&servCaddr, 0, sizeof(servCaddr));
	memset(auth_result, '\0', sizeof(auth_result));

	// Filling serverC information
	servCaddr.sin_family = AF_UNSPEC;
    servCaddr.sin_port = htons(UDP_PORT_C);
    servCaddr.sin_addr.s_addr = INADDR_ANY;

    //UDP socket for serverCS
    int serversocketCS;


	struct sockaddr_in servCSaddr;

	if ((serversocketCS = socket(AF_UNSPEC, SOCK_DGRAM, 0)) < 0 ){
	    printf("\nsocket CS creation failed\n");
	    exit(0);
	}

	memset(&servCSaddr, 0, sizeof(servCSaddr));


	// Filling serverCS information
	servCSaddr.sin_family = AF_UNSPEC;
    servCSaddr.sin_port = htons(UDP_PORT_CS);
    servCSaddr.sin_addr.s_addr = INADDR_ANY;

    //UDP socket for serverEE
    int serversocketEE;


	struct sockaddr_in servEEaddr;

	if ((serversocketEE = socket(AF_UNSPEC, SOCK_DGRAM, 0)) < 0 ){
	    printf("\nsocket  EE creation failed\n");
	    exit(0);
	}

	memset(&servEEaddr, 0, sizeof(servEEaddr));


	// Filling serverEE information
	servEEaddr.sin_family = AF_UNSPEC;
    servEEaddr.sin_port = htons(UDP_PORT_EE);
    servEEaddr.sin_addr.s_addr = INADDR_ANY;


    while(1){
 
        //receive username and password from client and encrypt it
        char buf_un[MAXSIZE];
        char buf_pwd[MAXSIZE];
        int bytes_recv = recv(sockdp_ch, buf_un, MAXSIZE-1,0);
        buf_un[bytes_recv] = '\0';
        char en_buf_un[bytes_recv]; //encrypted username
        for(int i = 0; i < bytes_recv; i++){
            en_buf_un[i] = encrypt(buf_un[i]);
        }
        bytes_recv = recv(sockdp_ch, buf_pwd, MAXSIZE-1,0);
        buf_pwd[bytes_recv] = '\0';
        char en_buf_pwd[bytes_recv]; //encrypted password
        for(int i = 0; i < bytes_recv; i++){
            en_buf_pwd[i] = encrypt(buf_pwd[i]);
        }
        cout << "The main server received the authentication for " << string(buf_un) << " using TCP over port " << TCP_PORT << "." << endl;

        //forward encrypted username and password to serverC
        //UDP client need not bind a connection
        char *send_uname = en_buf_un; //change names
        char *send_pwd = en_buf_pwd;

        //sedn uname to serverc
	    if((sendto(serversocketC, (const char *)send_uname, strlen(send_uname),MSG_CONFIRM, (const struct sockaddr *) &servCaddr, sizeof(servCaddr))) < 0){
	    	printf("\nSend username  to server Cfailed\n");
	    	exit(0);
	    }
        //send pwd to serverC
        if((sendto(serversocketC, (const char *)send_pwd, strlen(send_pwd),MSG_CONFIRM, (const struct sockaddr *) &servCaddr, sizeof(servCaddr))) < 0){
	    	printf("\nSend password  to server Cfailed\n");
	    	exit(0);
	    }
        cout << "The main server sent an authentication request to serverC" << endl;

        //receive message from the server C
        len = sizeof (servCaddr);
        
	    if((n = recvfrom(serversocketC, (char *)auth_result, sizeof (auth_result),MSG_WAITALL, (struct sockaddr *) &servCaddr, &len)) < 0){
	    	printf("\nRecieve from server C failed");
	    	exit(0);
	    }
        cout << "The main server received the result of the authentication request from ServerC using UDP over port " << TCP_PORT << "." << endl;

        // size of auth_rsult 1 or 2
        char * msg;
	    strcpy(msg,auth_result);

        //send auth result to the client over TCP
        int bytes_sent;
        len = strlen(msg);
        bytes_sent = send(sockdp_ch, msg, len, 0);
        if(bytes_sent < len){
            perror("auth not sent to client\n"); 
            exit(EXIT_FAILURE); 
        }
        cout << "The main server sent the authentication result to the client.\n";
        //receive next query from client via TCP
        char buf_course_code[MAXSIZE];
        int bytes_recv_coursecode = recv(sockdp_ch, buf_course_code, MAXSIZE-1,0);
        buf_course_code[bytes_recv_coursecode] = '\0';
        //cout << "The main server received from " << str(buf_un) << "to query course " << str(buf_course_code) << " about <category> using TCP over port <port number>." << endl;

        if(buf_course_code[0] == 'E' && buf_course_code[1] == 'E'){
            if((sendto(serversocketEE, (const char *)buf_course_code, strlen(buf_course_code),MSG_CONFIRM, (const struct sockaddr *) &servEEaddr, sizeof(servEEaddr))) < 0){
                printf("\nSend coursecode   to server EE failed\n");
                exit(0);
            } 
            cout << "The main server sent a request to serverEE.\n";
            //recieve result from server EE
            len = sizeof (servEEaddr);
            char query_res[MAXSIZE];
	        if((n = recvfrom(serversocketEE, (char *)query_res, sizeof (query_res),MSG_WAITALL, (struct sockaddr *) &servEEaddr, &len)) < 0){
	    	    printf("\nRecieve query result from server EE failed");
	    	    exit(0);
	        }
            // send of query result as TCP
            char *query_result;
	        strcpy(query_result,query_res);
            int bytes_query_sent;
            len = strlen(query_result);
            //send query to client
            bytes_query_sent = send(sockdp_ch, query_result, len, 0);
            if(bytes_query_sent < len){
                perror("query result  not send to client\n"); 
                exit(EXIT_FAILURE); 
            }
        }
        else if(buf_course_code[0] == 'C' && buf_course_code[1] == 'S' ){
            if((sendto(serversocketCS, (const char *)buf_course_code, strlen(buf_course_code),MSG_CONFIRM, (const struct sockaddr *) &servCSaddr, sizeof(servCSaddr))) < 0){
                printf("\nSend coursecode   to server CS failed\n");
                exit(0);
            } 
            cout << "The main server sent a request to serverCS.\n";
            //receive result from server CS
            len = sizeof (servCSaddr);
            char query_res[MAXSIZE];
	        if((n = recvfrom(serversocketCS, (char *)query_res, sizeof (query_res),MSG_WAITALL, (struct sockaddr *) &servCSaddr, &len)) < 0){
	    	    printf("\nRecieve query result from server CS failed");
	    	    exit(0);
	        }
            // size of query result as TCP
            char *query_result;
	        strcpy(query_result,query_res);
            int bytes_query_sent;
            len = strlen(query_result);
            //send query to client
            bytes_query_sent = send(sockdp_ch, query_result, len, 0);
            if(bytes_query_sent < len){
                perror("query result  not send to client\n"); 
                exit(EXIT_FAILURE);                 
            }

        }
        else{
            //sedning code 2 if any wrong course entered
            char code[] = {'2'};
            char* msg;
            strcpy(msg,code);
            int bytes_sent_cc;
            len = strlen(msg);
            bytes_sent_cc = send(sockdp_ch, msg, len, 0);
            if(bytes_sent < len){
                perror("auth result  not send to client\n"); 
                exit(EXIT_FAILURE); 
            }     

        }
        cout << "The main server sent the query information to the client.\n";

        // close child socket to prevent more connections on the same port but parent socket runs
        int sock = close(sockdp_ch);        
    }
}

// get sockaddr, IPv4 or IPv6:
void* get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

char encrypt(char a){
    char ch;
    if((a >= 'A' && a <= 'Z')){
        ch = (char)(((int)a + 4)%90);
    }
    else if(a >= 'a' && a<='z'){
        ch = (char)(((int)a + 4)%122);
    }
    else if(a >= '0' && a <='9'){
        ch = (char)(((int)a + 4)%57);
    }
    else{
        ch = a;
    }
    return ch;
}