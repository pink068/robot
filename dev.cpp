using namespace std;
//
//
//   dev.cpp hold the file i am currently working on to make compiling faster
//
//

#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "sqlite3.h"
#include "datarobot.h"
#include <iostream>
#include <fstream>


struct MessageStruct
{
    string ToCall;
    string DE;
    string FromCall;
    string Qcode;
    string Param;
    string Arg;
    string CallMiltime;
    string CallDTG;
      int ErrorCode;
};



extern char MyCall[10];
extern struct MessageStruct command;



extern int GetDTG(char * dtg);
extern int GetMiltime(char * dtg);

//
// this needs to be rewritten to look for DE with a callsign before and after.
//
//



int MessageToStruct( char *  ustring)
{
//  this is where we pull the command apart into a struct
  char userstring[100];
  char * found;
  char dtg[100];

  strcpy(userstring,ustring);
//	printf("recieved[%s]\r\n",userstring);

  found = strtok(userstring," ");  
  command.ToCall=found;

  found = strtok(NULL," ");
  command.DE=found;

  found = strtok(NULL," ");
  command.FromCall=found;

  found = strtok(NULL," ");
  command.Qcode=found;
  command.Qcode[3]=0;  // force 3 digits

//  found = strtok(NULL," ");
  command.Arg=&found[4];  // put the rest of the line into arg.
    
  if (command.ToCall.compare(0,strlen(MyCall),MyCall))
  {    // if it's not for me then nothing to do
 //    printf("%s Not for %s\r\n", command->ToCall.c_str(),MyCall);
     command.ErrorCode =1;
     return 0;
  }

  if (command.DE.compare("DE"))
  {
    command.ErrorCode =2;
    return 0;
  }

  GetDTG(dtg);
  command.CallDTG = dtg; 

  GetMiltime(dtg);
  command.CallMiltime =dtg;

  return 0;
}






