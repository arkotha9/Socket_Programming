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
    
#define UDP_PORT 22242 
#define MAXLINE 10000 

using namespace std;
// Driver code 


int main() { 

    //read cs.txt file
    ifstream ifs;
    ifs.open("./cs.txt");
    //vector<course_info*> courses;
    vector<string> courses;
    string details;
    if(ifs.is_open()){
        while(getline(ifs,details)){
           // stringstream linestream(details);
            //string course_code, credits, prof, days, course_name;
            //getline(linestream, course_code, ',');
            //linestream >> credits >> prof >> days >> course_name;
            //courses.push_back(add_courseinfo_to_list(course_name,credits,prof,days,course_name));
            courses.push_back(details);
        }
    }

    int sock_dp_CS; 
    char buffer[MAXLINE]; 
    //char *hello = "Hello from server"; 
    struct sockaddr_in servCSaddr, servMaddr; 
        
    // Creating socket file descriptor 
    if ((sock_dp_CS = socket(AF_INET, SOCK_DGRAM, 0)) < 0){ 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    cout << "The ServerCS is up and running using UDP on port " << UDP_PORT << endl; 

    memset(&servCSaddr, 0, sizeof(servCSaddr)); 
    memset(&servMaddr, 0, sizeof(servMaddr)); 
        
    // Filling server information 
    servCSaddr.sin_family    = AF_UNSPEC; // IPv4 or IPv6 
    servCSaddr.sin_addr.s_addr = INADDR_ANY; 
    servCSaddr.sin_port = htons(UDP_PORT); 
        
    // Bind the socket with the server address 
    if (bind(sock_dp_CS, (const struct sockaddr *)&servCSaddr, sizeof(servCSaddr)) < 0){ 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
        
    unsigned int len, n; 
    while(1){
        len = sizeof(servMaddr);  //len is value/result 
        //receive COURSE CODE
        n = recvfrom(sock_dp_CS, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servMaddr, &len); 
        if(n<0){            
            perror("No request received by serverCS from Main server"); 
            exit(EXIT_FAILURE); 
        }
        buffer[n] = '\0'; 
        string course_name = buffer;
        bool found_course = false;
        int index;
        for(int i = 0; i < courses.size(); i++){
            string a = courses[i];
            if(a.substr(0,5) == course_name){
                found_course = true;
                index = i;
                break;
            }
        }
        //receive the next quer for category
        char query_buf[10000];
        n = recvfrom(sock_dp_CS, (char *)query_buf, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servMaddr, &len); 
        query_buf[n] = '\0'; 
        string query = query_buf;

        cout << "The ServerCS received a request from the Main Server about the " << query << " of " << course_name << endl;

        //course code is not found
        if(!found_course){
            sendto(sock_dp_CS, "2", 1, MSG_CONFIRM, (const struct sockaddr *) &servMaddr, len); 
            cout << "Didn’t find the course: "<< course_name << "." << endl;
            //printf(".\n"); 
        }
        else{
            string a = courses[index];    
            string credit = a.substr(6,7);
            string a2 = a.substr(8);
            int pos = a2.find(",");
            string prof = a2.substr(0,pos);
            a2 = a2.substr(pos+1);
            pos = a2.find(",");
            string days = a2.substr(0,pos);
            string course_name = a2.substr(pos+1);

                if(query == "Credit"){
                    sendto(sock_dp_CS, (const char*)(credit.c_str()), 1, MSG_CONFIRM, (const struct sockaddr *) &servMaddr, len);    
                    cout << "The course information has been found: The " << query << " of " << course_name<< " is " << credit << "." << endl;                 
                }
                else if(query == "Professor"){
                    sendto(sock_dp_CS, (const char*)(prof.c_str()), 1, MSG_CONFIRM, (const struct sockaddr *) &servMaddr, len);
                    cout << "The course information has been found: The " << query << " of " << course_name<< " is " << prof << "." << endl;       
                }
                else if(query == "Days"){
                    sendto(sock_dp_CS, (const char*)(days.c_str()), 1, MSG_CONFIRM, (const struct sockaddr *) &servMaddr, len);  
                    cout << "The course information has been found: The " << query << " of " << course_name<< " is " << days << "." << endl;     
                }
                else{
                    sendto(sock_dp_CS, (const char*)(course_name.c_str()), 1, MSG_CONFIRM, (const struct sockaddr *) &servMaddr, len);  
                    cout << "The course information has been found: The " << query << " of " << course_name<< " is " << course_name << "." << endl;     
                }
                
          

        }

    

        //printf("Client : %s\n", buffer); 
        //sendto(sockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len); 
        printf("The ServerCS finished sending the response to the Main Server.\n");   

    }

}
