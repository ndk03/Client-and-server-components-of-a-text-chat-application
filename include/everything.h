#ifndef EVERYTHING_H
#define EVERYTHING_H

#include <string.h>
#include <queue>
#include <list>

struct block{
  block():listen_port_num(-1){
    bzero(&host,sizeof(host));
    bzero(&ip,sizeof(ip));
  }
  char host[1024];
  char ip[1024];
  int listen_port_num;
};

struct buffer_info{
  buffer_info(){
    bzero(&des_ip,sizeof(des_ip));
    bzero(&mesg,sizeof(mesg));
    bzero(&fr,sizeof(fr));
  }
  char des_ip[32];
  char mesg[1024];
  char fr[32];
};

struct socket_info{
  socket_info():num_msg_sent(0),num_msg_rcv(0),list_id(-1),fd(-1),port_num(-1){
    bzero(&hostname,sizeof(hostname));
    bzero(&ip_addr,sizeof(ip_addr));
    bzero(&status,sizeof(status));
  }
  int fd;
  int list_id;
  char hostname[40];
  char ip_addr[20];
  int port_num;
  int num_msg_sent;
  int num_msg_rcv;
  char status[16];
  std::list<block> blocked_list;
  std::queue<buffer_info> buffer;
};

struct info{
  info():yes(1),clients_number(0){
    bzero(&ip_address,sizeof(ip_address));
    bzero(&port_number,sizeof(port_number));
  }
  std::list<socket_info> clients;
  std::list<block> block_list;
  char ip_address[1024];
  char port_number[1024];
  int listener;
  int yes;        
  int clients_number;
};

class command_handling
{
protected:
  info sock_info;
public:
  command_handling();

  void print_error(const char* command_str);
  void print_author();
  void print_ip();
  void print_port();
};

class server:public command_handling
{
public:
  server(char* port);
  bool valid_ip(char *server_ip);
};

class client:public command_handling
{
public:
  client(char *port);
private:
  void break_cmd(const char* cmd,char *&server_ip,char *&server_port);
  bool valid_ip(char *server_ip,int port);
};

#endif
