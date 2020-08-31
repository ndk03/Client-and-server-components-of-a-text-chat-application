#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <arpa/inet.h>
#include <netdb.h>
#include "../include/logger.h"
#include "../include/everything.h"

#define STDIN 0
using namespace std;

bool compare_client(socket_info si1,socket_info si2){
  return si1.port_num < si2.port_num;
}

bool client::valid_ip(char *server_ip,int p){
  struct sockaddr_in ip4addr;
  ip4addr.sin_family = AF_INET;
  ip4addr.sin_port = htons(p);
  if(inet_pton(AF_INET,server_ip,&ip4addr.sin_addr) != 1)
    return false;
  return true;
}

client::client(char *port){
  //get port number 
  strcpy(sock_info.port_number,port);

  //get IP address 
  struct hostent *host;
  char hostname[1024];
  if (gethostname(hostname,1024) < 0){
    cerr<<"gethostname\n";
    exit(1);
  }
  if ((host=gethostbyname(hostname)) == NULL){
    cerr<<"gethostbyname\n";
    exit(1);
  }
  struct in_addr **addr_list = (struct in_addr **)host->h_addr_list;
  for(int i = 0;addr_list[i] != NULL;++i){
    strcpy(sock_info.ip_address,inet_ntoa(*addr_list[i]));
  }

  //Create socket
  if ((sock_info.listener = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    cerr<<"socket\n";
    exit(1);
  }

  //Bind socket
  struct sockaddr_in client_addr; 
  bzero(&client_addr,sizeof(client_addr));
  client_addr.sin_family = AF_INET; 
  client_addr.sin_port = htons(atoi(port)); 
  client_addr.sin_addr = *((struct in_addr*)host->h_addr);
  if (bind(sock_info.listener, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
    cerr<<"bind\n";
    exit(1);
  }

  char buf[1024];
  while(true){
    bzero(&buf,sizeof(buf));
    read(STDIN,buf,1024);
    buf[strlen(buf)-1]='\0';
    if (strcmp(buf,"EXIT") == 0){
      cse4589_print_and_log("[EXIT:SUCCESS]\n");
      cse4589_print_and_log("[EXIT:END]\n");
      break;
    }
    else if (strcmp(buf,"AUTHOR") == 0){
      print_author();
    }
    else if (strcmp(buf,"PORT") == 0){
      print_port();
    }
    else if (strcmp(buf,"IP") == 0){
      print_ip();
    }
    else if (strcmp(buf,"LIST") == 0){
      cse4589_print_and_log("[LIST:SUCCESS]\n");
      int i = 0;
      sock_info.clients.sort(compare_client);
      for(list<socket_info>::iterator iter = sock_info.clients.begin();iter != sock_info.clients.end();++iter){
        if (strcmp(iter->status,"logged-in") == 0)
           cse4589_print_and_log("%-5d%-35s%-20s%-8d\n",++i,iter->hostname,iter->ip_addr,iter->port_num);
      }
      cse4589_print_and_log("[LIST:END]\n");
    }
    else if (strncmp(buf,"LOGIN",5) == 0){
      char *server_ip;
      char *server_port;
      strtok(buf," ");
      server_ip = strtok(NULL," ");
      server_port = strtok(NULL," ");
      

      bool valid_port = true;
      if(server_port == NULL){
        print_error("LOGIN");
        continue;
      }   
      for(int i = 0;i != strlen(server_port);++i){
        if(server_port[i] >= '0' && server_port[i] <= '9'){
          continue;
        }
        else{
          print_error("LOGIN");
          valid_port = false;
          break;
        }
      }
      if(!valid_port) continue;
      int port = atoi(server_port);
      if(port < 0 || port > 65535){
        print_error("LOGIN");
        continue;
      }

      //Invalid ip address
      if (!valid_ip(server_ip,port)){
        print_error("LOGIN");
        continue;
      }
      else{
        struct addrinfo hints;
        struct addrinfo *result;
        bzero(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        if (getaddrinfo(server_ip, server_port, &hints, &result) != 0) {
          print_error("LOGIN");
          continue;
        }
        else{
          //Get socket fd
          if ((sock_info.listener = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            print_error("LOGIN");
            continue;
          }

          //Connect to server
          struct sockaddr_in dest_addr; 
          bzero(&dest_addr,sizeof(dest_addr));
          dest_addr.sin_family = AF_INET;
          dest_addr.sin_port = htons(port);
          dest_addr.sin_addr.s_addr = inet_addr(server_ip);
          if ((connect(sock_info.listener, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr))) < 0){
            print_error("LOGIN");
            continue;
          }
           
          char client_port[8];
          bzero(&client_port,sizeof(client_port));
          strcat(client_port,sock_info.port_number);
          if(send(sock_info.listener,client_port,strlen(client_port),0)<0){
            cerr<<"port\n";
          }


          char buf[1024];
          for(;;){
            // Add the listener to master_list 
            bzero(&buf,sizeof(buf));
            fd_set watch_list;
            FD_ZERO(&watch_list);
            FD_SET(STDIN,&watch_list);
            FD_SET(sock_info.listener,&watch_list);

            int fd_max = sock_info.listener;
            select(fd_max+1, &watch_list, NULL, NULL, NULL);
            if (FD_ISSET(STDIN, &watch_list)){
              read(STDIN,buf,1024);
              buf[strlen(buf)-1]='\0';

              if (strcmp(buf,"AUTHOR") == 0){
                print_author();
              }
              else if (strcmp(buf,"PORT") == 0){
                print_port();
              }
              else if (strcmp(buf,"IP") == 0){
                print_ip();
              }
              else if (strcmp(buf,"LIST") == 0){
                  cse4589_print_and_log("[LIST:SUCCESS]\n");
                  int i = 0;
                  sock_info.clients.sort(compare_client);
                  for(list<socket_info>::iterator iter = sock_info.clients.begin();iter != sock_info.clients.end();++iter){
                    if (strcmp(iter->status,"logged-in") == 0)
                      cse4589_print_and_log("%-5d%-35s%-20s%-8d\n",++i,iter->hostname,iter->ip_addr,iter->port_num);
                  }
                  cse4589_print_and_log("[LIST:END]\n");
              }
              else if(strcmp(buf,"REFRESH") == 0){
                strcat(buf," ");
                strcat(buf,sock_info.ip_address);
                if(send(sock_info.listener,buf,strlen(buf),0)<0){
                  print_error("REFRESH");
                }
                cse4589_print_and_log("[REFRESH:SUCCESS]\n");
                cse4589_print_and_log("[REFRESH:END]\n");
              }
              else if(strncmp(buf,"SEND",4) == 0){
                char send_message[1024];
                bzero(&send_message,sizeof(send_message));
                strcpy(send_message,buf);
                char *arg[3];
                arg[0] = strtok(buf," ");
                for(int i = 1;i != 3;++i){
                  arg[i] = strtok(NULL," ");
                }
                //check  in current list 
                bool isval = false;
                for(list<socket_info>::iterator it = sock_info.clients.begin();it != sock_info.clients.end();++it){
                  if(strcmp(it->ip_addr,arg[1]) == 0) 
                    isval = true;
                }
            
                if(!isval || send(sock_info.listener,send_message,strlen(send_message),0)<0){
                  print_error("SEND");
                  continue;
                }
                cse4589_print_and_log("[SEND:SUCCESS]\n");
                cse4589_print_and_log("[SEND:END]\n");
              }
              else if(strncmp(buf,"BROADCAST",9) == 0){
                if(send(sock_info.listener,buf,strlen(buf),0)<0){
                  print_error("BROADCAST");
                }
                cse4589_print_and_log("[BROADCAST:SUCCESS]\n");
                cse4589_print_and_log("[BROADCAST:END]\n");
              }
              else if(strncmp(buf,"BLOCK",5) == 0){
                char temp_buf[1024];
                bzero(&temp_buf,sizeof(temp_buf));
                strcpy(temp_buf,buf);
                strtok(temp_buf," ");
                char *block_ip = strtok(NULL," ");
                bool isval = false;
                bool isblocked = false;
                block b;
                for(list<socket_info>::iterator it = sock_info.clients.begin();it != sock_info.clients.end();++it){
                  if(strcmp(it->ip_addr,block_ip) == 0) {
                    isval = true;
                    b.listen_port_num = it->port_num;
                    strcpy(b.host,it->hostname);
                    strcpy(b.ip,it->ip_addr);
                    break;
                  }
                }
                for(list<block>::iterator block_it = sock_info.block_list.begin();block_it != sock_info.block_list.end();++block_it){
                  if(strcmp(block_ip,block_it->ip) == 0){
                    isblocked = true;
                    break;
                  }
                }

                if(!isval || isblocked){
                  print_error("BLOCK");
                  continue;
                }
                if(send(sock_info.listener,buf,strlen(buf),0)<0){
                  print_error("BLOCK");
                  continue;
                }
                else{
                  sock_info.block_list.push_back(b);
                }
                cse4589_print_and_log("[BLOCK:SUCCESS]\n");
                cse4589_print_and_log("[BLOCK:END]\n");
              }
              else if(strncmp(buf,"UNBLOCK",7) == 0){
                char *arg[2];
                char temp_buf[1024];
                bzero(&temp_buf,sizeof(temp_buf));
                strcpy(temp_buf,buf);
                arg[0] = strtok(temp_buf," ");
                arg[1] = strtok(NULL," ");

                bool valid = false;
                for(list<block>::iterator block_it = sock_info.block_list.begin();block_it != sock_info.block_list.end();++block_it){
                  if(strcmp(block_it->ip,arg[1]) == 0){
                    sock_info.block_list.erase(block_it);
                    valid = true;
                    break;
                  }
                }
                if(!valid || send(sock_info.listener,buf,strlen(buf),0)<0){
                  print_error("UNBLOCK");
                  continue;
                }
                cse4589_print_and_log("[UNBLOCK:SUCCESS]\n");
                cse4589_print_and_log("[UNBLOCK:END]\n");
              }
              else if(strcmp(buf,"LOGOUT") == 0){
                cse4589_print_and_log("[LOGOUT:SUCCESS]\n");
                close(sock_info.listener);
                cse4589_print_and_log("[LOGOUT:END]\n");
                break;
              }
              else if(strcmp(buf,"EXIT") == 0){
                close(sock_info.listener);
                cse4589_print_and_log("[EXIT:SUCCESS]\n");
                cse4589_print_and_log("[EXIT:END]\n");
                exit(0);
              }
            }
            else{
              char msg[1024];
              bzero(&msg,sizeof(msg));
              int recvbytes;
              if((recvbytes = recv(sock_info.listener,msg,sizeof(msg),0)) <= 0){
                cout<<"recv\n";
              }
              
              char *arg_zero = strtok(msg," ");

              //Process received data 
              
              if(FD_ISSET(sock_info.listener,&watch_list)){
                
                if(strcmp(arg_zero,"SEND") == 0){
                  cse4589_print_and_log("[%s:SUCCESS]\n", "RECEIVED");
                  char *arg[4];
                  arg[1] = strtok(NULL," ");
                  arg[2] = strtok(NULL," ");
                  arg[3] = strtok(NULL,"");
                  cse4589_print_and_log("msg from:%s\n[msg]:%s\n",arg[1],arg[3]);
                  cse4589_print_and_log("[%s:END]\n", "RECEIVED");
                }
                else if(strcmp(arg_zero,"BROADCAST") == 0){
                  cse4589_print_and_log("[%s:SUCCESS]\n", "RECEIVED");
                  char *arg[3];
                  arg[1] = strtok(NULL," ");
                  arg[2] = strtok(NULL,"");
                  cse4589_print_and_log("msg from:%s\n[msg]:%s\n",arg[1],arg[2]);
                  cse4589_print_and_log("[%s:END]\n", "RECEIVED");
                }
                else if(strcmp(arg_zero,"LOGIN") == 0){
                  sock_info.clients.clear();
                  while(true){
                    char *list_msg[3];

                    //check for buffered message
                    list_msg[0] = strtok(NULL," ");
                    char mesg[512];
                    char messag[4096];
                    bzero(&messag,sizeof(messag));
                    while(list_msg[0] != NULL && strcmp(list_msg[0],"BUFFER") == 0){
                      char original_messag[4096];
                      bzero(&original_messag,sizeof(original_messag));
                      strcpy(messag,strtok(NULL,""));
                      strcpy(original_messag,messag);

                      char *fr = strtok(original_messag," ");
                      char *l = strtok(NULL," ");

                      int length = atoi(l);
                      char *next;
                      next = strtok(NULL,"");
                      bzero(&mesg,sizeof(mesg));
                      strncpy(mesg,next,length);
                      cse4589_print_and_log("[%s:SUCCESS]\n", "RECEIVED");
                      cse4589_print_and_log("msg from:%s\n[msg]:%s\n",fr,mesg);
                      cse4589_print_and_log("[%s:END]\n", "RECEIVED");

                      list_msg[0] = strtok(messag," ");
                      if(strcmp(list_msg[0],"BUFFER") == 0 || list_msg[0] == NULL) {
                        continue;
                      }

                      while((list_msg[0] = strtok(NULL," ")) != NULL && strcmp(list_msg[0],"BUFFER") != 0)
                        continue;
                    }
                    if(list_msg[0] == NULL){
                      cse4589_print_and_log("[LOGIN:SUCCESS]\n");
                      cse4589_print_and_log("[LOGIN:END]\n");
                      break;
                    }


                    for(int j = 1;j != 3;++j){
                      bzero(&list_msg[j],sizeof(list_msg[j]));
                      list_msg[j] = strtok(NULL," ");
                    }
                    struct socket_info si;
                    strcpy(si.hostname,list_msg[0]);
                    strcpy(si.ip_addr,list_msg[1]);
                    int port_n = atoi(list_msg[2]);
                    si.port_num = port_n;
                    strcpy(si.status,"logged-in");
                    sock_info.clients.push_back(si);
                  }
                }
                else if(strcmp(arg_zero,"REFRESH") == 0){
                  sock_info.clients.clear();
                  while(true){
                    char *list_msg[3];
                    if((list_msg[0] = strtok(NULL," ")) == NULL)
                      break;
                    
                    for(int j = 1;j != 3;++j){
                      bzero(&list_msg[j],sizeof(list_msg[j]));
                      list_msg[j] = strtok(NULL," ");
                    }
                    struct socket_info si;
                    strcpy(si.hostname,list_msg[0]);
                    strcpy(si.ip_addr,list_msg[1]);
                    int port_n = atoi(list_msg[2]);
                    si.port_num = port_n;
                    strcpy(si.status,"logged-in");
                    sock_info.clients.push_back(si);
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}



