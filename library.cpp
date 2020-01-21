#include "library.h"
#include <cstring>



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
struct MessageStruct command;



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

































char dummy_messages[][100]=
{
  "MRV84D DE MRC19 QRA? K",
  "MRV84D DE MRC11 QME? K",
  "MRV84D DE MRC12 QRK? K",
  "MRV84D DE MRC13 QSL? K",
  "MRV84D DE MRC14 QSM? K",
  "MRV84D DE MRC15 QTR? K",
  "MRV84D DE MRC16 QSG 15 K",
  "MRV84D DE MRC17 QUA? K",
  "MRV84D DE MRC18 QSP MRV07 hello on the cheep service K",
  "MRV84D DE MRV07 QUC K",  
  "MRV84D DE MRV08 QRU 2 K",
};


















int Server::Read( char * buffer)
{
	// get a string for the server
    strcpy(buffer, "MRP99 DE MRC19 QRA? K");
//	GetPage(char * buffer, char * url)    ;
	return strlen(buffer);
}

int Server::Write( char * buffer)
{
	// send a string to the server
	return 1;
}




