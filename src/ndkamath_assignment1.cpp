#include <iostream>
#include <stdio.h>
#include "../include/logger.h"
#include "../include/everything.h"

using namespace std;

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
command_handling ::command_handling(){
}

void command_handling::print_author(){
  cse4589_print_and_log("[AUTHOR:SUCCESS]\n");
  cse4589_print_and_log("I, ndkamath, have read and understood the course academic integrity policy.\n");
  cse4589_print_and_log("[AUTHOR:END]\n");
}
void command_handling::print_ip(){
  cse4589_print_and_log("[IP:SUCCESS]\n");
  cse4589_print_and_log("IP:%s\n",sock_info.ip_address);
  cse4589_print_and_log("[IP:END]\n");
}
void command_handling::print_port(){
  cse4589_print_and_log("[PORT:SUCCESS]\n");
  cse4589_print_and_log("PORT:%s\n",sock_info.port_number);
  cse4589_print_and_log("[PORT:END]\n");
}
void command_handling::print_error(const char* command_str){
  cse4589_print_and_log("[%s:ERROR]\n",command_str);
  cse4589_print_and_log("[%s:END]\n",command_str);
}
int main(int argc, char **argv)
{
  
  /*Init. Logger*/
  cse4589_init_log(argv[2]);

  /* Clear LOGFILE*/
  fclose(fopen(LOGFILE, "w"));

  /*Start Here*/
  if (strcmp(argv[1],(char*)"c") == 0)
    client c(argv[2]);
  if (strcmp(argv[1],(char*)"s") == 0)
    server s(argv[2]);
  
  return 0;
}
