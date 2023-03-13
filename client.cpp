#include <netdb.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <iostream>
#include <string>

#define SERVM_PORT "25242" // the port client will be connecting to
#define MAXDATASIZE 100  // max number of bytes we can get at once

using namespace std;

int dynamic_port_alloc();
void* get_in_addr(struct sockaddr);

int main(int argc, char *argv[]){

    int sock_dp, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    /*if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }*/

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], SERVM_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sock_dp = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
        perror("client: socket");
        continue;
    }
    cout << "The client is up and running.\n";
    if (connect(sock_dp, p->ai_addr, p->ai_addrlen) == -1) {
        close(sock_dp);
        perror("client: connect");
        continue;
    }

    break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    //inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)(p->ai_addr)), s, sizeof s);
    inet_ntop(p->ai_family,p->ai_addr,s,sizeof(s));

    printf("client: connecting to %s\n", s);


    freeaddrinfo(servinfo); // all done with this structure
    
    while(1){
        int count = 1;
        int CLIENT_PORT = dynamic_port_alloc();
        string username, password;
        while(count <= 3){
            count++;
            cout << "Please enter the username:";
            cin >> username;
            cout << endl << "Please enter the password:";
            cin >> password;
            cout << endl;

            //send username and password to main server over TCP
            //convert string to char*
            char *user = new char[username.size()+1];
            copy(username.begin(),username.end(),user);
            user[username.size()] = '\0';

            char *pass = new char[password.size()+1];
            copy(password.begin(),password.end(),pass);
            pass[password.size()] = '\0';

            int len_un, len_pwd, bytes_sent_un, bytes_sent_pwd;
            //send uname first
            len_un = strlen(user);
            bytes_sent_un = send(sock_dp, user, len_un, 0);
            if(bytes_sent_un < len_un){
                perror("full username not sent\n."); 
                exit(EXIT_FAILURE); 
            }
            cout << username <<" sent an authentication request to the main server." << endl;
            //send password
            len_pwd = strlen(pass);
            bytes_sent_pwd = send(sock_dp, pass, len_pwd, 0);
            if(bytes_sent_pwd < len_pwd){
                perror("full password not sent\n."); 
                exit(EXIT_FAILURE);
            }

            // receive auth result from main server
            int numbytes;
            char auth[MAXDATASIZE];
            numbytes = recv(sock_dp,auth,MAXDATASIZE-1,0); //receive code about authentication from serverM
            auth[numbytes] = '\0';
            if(auth[0] == '0'){
                cout << username << " received the result of authentication using TCP over port " << CLIENT_PORT << ".Authentication failed: Username Does not exist" << endl;
                cout << "Attempts remaining:" << (3 - count) << endl;
                count++;
            }
            else if(auth[0] == '1'){
                cout << username << " received the result of authentication using TCP over port " << CLIENT_PORT << ".Authentication is succesful." << endl;
                cout << "Attempts remaining:" << (3 - count) << endl;
                count++;
            }
            else{
                cout << username << " received the result of authentication using TCP over port " << CLIENT_PORT << ".Authentication failed: Username Does not exist" << endl;
                break;
            }
        }
        if(count > 3){
            cout << "Authentication Failed for 3 attempts. Client will shut down." << endl;
            close(sock_dp);
            return 0;
        }
        //if authentication passes.
        while(1){
            string c_code,category;
            cout << "Please enter the course code to query:";
            cin >> c_code;
            cout << endl << "Please enter the category (Credit / Professor / Days / CourseName):";
            cin >> category;
            cout << endl;
            //send the course code to serverM
            //char *code = c_code.c_str();
            char *code = new char[c_code.size()+1];
            copy(c_code.begin(),c_code.end(),code);
            code[c_code.size()] = '\0';

            int len_code, bytes_sent_code;
            len_code = strlen(code);
            bytes_sent_code = send(sock_dp, code, len_code, 0);
            if(bytes_sent_code < len_code){
                perror("full course code not sent\n."); 
                exit(EXIT_FAILURE);
            }
            cout << username << " sent a request to the main server." << endl;

            //get authentication about the course code from main server
            int numbytes_course;
            char auth_course[MAXDATASIZE];
            numbytes_course = recv(sock_dp,auth_course,MAXDATASIZE-1,0); //receive code about authentication of course from serverM
            cout << "The client received the response from the Main server using TCP over port " << CLIENT_PORT << endl;

            auth_course[numbytes_course] = '\0';            
            if(auth_course[0] == '1' || auth_course[0]=='0' ){ //EE or CS
                char *cat = new char[category.size()+1];
                copy(category.begin(),category.end(),cat);
                cat[category.size()] = '\0';

                int len_cat, bytes_sent_cat;
                len_cat = strlen(cat);
                bytes_sent_cat = send(sock_dp, cat, len_cat, 0); //sedning category to main server
                if(bytes_sent_cat < len_cat){
                    perror("full category not sent\n."); 
                    exit(EXIT_FAILURE);
                } 
                int numbytes;
                char cat_info[MAXDATASIZE];
                numbytes = recv(sock_dp, cat_info, MAXDATASIZE-1, 0); //receive info about category from serverM
                cat_info[numbytes] = '\0';
                cout << "The " << category << " of " << c_code << " is " << cat_info << "." << endl;
                cout << "-----Start a new request-----" << endl;

            }
            else{
                cout << "Didn’t find the course: " << c_code << ".\n";
                cout << "-----Start a new request-----" << endl;
                continue;
            }
        }

    }

    //close(sock_dp);

    return 0;
}



const void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int dynamic_port_alloc(){
    unsigned int clientPort;
    struct sockaddr_in clientAddress;

    bzero(&clientAddress, sizeof(clientAddress));
    socklen_t len = sizeof(clientAddress);
    getsockname(clientPort, (struct sockaddr *) &clientAddress, &len);
    clientPort = ntohs(clientAddress.sin_port);
    return clientPort;
}
