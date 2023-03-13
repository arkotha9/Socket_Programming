#include <stdio.h> 
#include <iostream>
#include <fstream>
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <string>
#include <vector>
#include <sstream>

   
#define UDP_PORT 21242 
#define MAXLINE 10000 

using namespace std;
// Driver code 

int main() { 

    //read cred.txt file
    ifstream ifs;
    ifs.open("./cred.txt");
    //ifstream ifs = ifstream("cred.txt");
    string uname_pass;
    vector<string> unames;
    vector<string> passwds;
    if(ifs.is_open()){
        while(getline(ifs,uname_pass)){
             stringstream linestream(uname_pass);
             string uname, passwd;
             getline(linestream, uname, ',');
             linestream >> passwd;
             unames.push_back(uname);
             passwds.push_back(passwd);
        }
    }

    int sock_dp_C; 
    char buffer[MAXLINE]; 
    //char *hello = "Hello from server"; 
    struct sockaddr_in servCaddr, servMaddr; 
        
    // Creating socket file descriptor 
    if ((sock_dp_C = socket(AF_INET, SOCK_DGRAM, 0)) < 0){ 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    cout << "The ServerC is up and running using UDP on port " << UDP_PORT << endl;
        
    memset(&servCaddr, 0, sizeof(servCaddr)); 
    memset(&servMaddr, 0, sizeof(servMaddr)); 
        
    // Filling server information 
    servCaddr.sin_family    = AF_UNSPEC; // IPv4 or IPv6 
    servCaddr.sin_addr.s_addr = INADDR_ANY; 
    servCaddr.sin_port = htons(UDP_PORT); 
        
    // Bind the socket with the server address 
    if (bind(sock_dp_C, (const struct sockaddr *)&servCaddr, sizeof(servCaddr)) < 0){ 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
        
    unsigned int len, n; 
    //socklen_t length;
    while(1){
        len = sizeof(servMaddr);  //len is value/result 
        //receive username 
        n = recvfrom(sock_dp_C, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servMaddr, &len); 
        if(n<0){            
            perror("No request received by serverC from Main server"); 
            exit(EXIT_FAILURE); 
        }
        cout << "The ServerC received an authentication request from the Main Server." << endl;
        buffer[n] = '\0'; 
        string u_name = buffer;
        bool found_uname = false;
        for(int i = 0; i < unames.size(); i++){
            if(unames[i] == u_name){
                found_uname = true;
                break;
            }
        }
        if(!found_uname){
            sendto(sock_dp_C, "0", 1, MSG_CONFIRM, (const struct sockaddr *) &servMaddr, len); 
            //printf(".\n"); 
        }
        else{
            //receive password
            n = recvfrom(sock_dp_C, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servMaddr, &len); 
            //if(n > 0){
 
                buffer[n] = '\0'; 
                string password = buffer;
                bool found_pwd = false;
                for(int i = 0; i < passwds.size(); i++){
                    if(passwds[i] == password){
                        found_pwd = true;
                        break;
                    }
                }
                if(!found_pwd){
                    sendto(sock_dp_C, "1", 1, MSG_CONFIRM, (const struct sockaddr *) &servMaddr, len); //no password
                }
                else{
                    sendto(sock_dp_C, "2", 1, MSG_CONFIRM, (const struct sockaddr *) &servMaddr, len); //succesful authenctication
                }
           // }

        }  

        //printf("Client : %s\n", buffer); 
        //sendto(sockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len); 
        printf("The ServerC finished sending the response to the Main Server.\n");   

    }

}
