using namespace std;
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "sqlite3.h"
#include "datarobot.h"
#include <iostream>
#include <fstream>

#define NUM_OF_SWITCHES 12
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

class DataLite
{
  private:
    sqlite3 *db;
    int Write( char * query);
    int GetRows( char * query);

  public: 
    int Initialise();
    int Close();
    int LogUpdate( MessageStruct command);
    int LastHeardUpdate( MessageStruct command);
    int LastHeardList( char * buffer);
    int ReadConfig(void );
    int SendMessage( MessageStruct command);
    int ListMessages( char * buffer, char * mailto, int status);
    int ReadMessage( char * buffer, char * mailto, int messno);
    int MakeIfMissing(void);
};


char MyCall[10];
char InfoStationName[100];
char InfoStationEquip[100];
char previousreply[1000];
DataLite Database;
struct MessageStruct command;


int MessageToStruct(char * userstring);
int LookupSwitchCode(string qcode);
int ProcessRequest( char * reply, int maxlen);

int GetDTG(char * dtg);
int GetMiltime(char * dtg);
int GetDATETIME(char * dtg);
int easyrandom(int min, int max);
int CodexGroups(char * buffer, int amount);


char switchtable[][4]=
{
 "QRA", // 0
 "QME", // 1
 "QRK", // 2
 "QSL", // 3
 "QSM", // 4
 "QTR", // 5
 "QSG", // 6
 "QUA", // 7 lastheard
 "QSP", // 8 
 "QTC", // 9 list my mail
 "QUC", // 10  list my unread mail
 "QRU", // 11 read message number ..
};


int LookupSwitchCode(string qcode)
{
 int a;
  
  for (a=0;a<NUM_OF_SWITCHES+1;a++)
  {
    if (!strcmp(qcode.c_str(),switchtable[a]))
    {
      break;
    }
  }

  return a;   
}




/* 
 * this is where all the action happens.
 * the data structure of the line of text that came in from the user
 * it processed buy looink up the Q code, in LookupSwitchCode() which
 * converts the Q code into a number to make it easier to process
 * 
 * The Q number then goes through a switch statement which makes up a 
 * reply string base on the Q code that came in.
 * EG QTR gets decoded to a 5.
 * The switch statment then detects that 5 and goes to find the time
 * it then makes up a reply message and passes it back to the main.cpp
 * 
 * worked examples.
 * mrv64d de mrv07 qra k
 * the program decodes the qra to a 1 it knows that command 1 is 
 * transmit the InfoStationName string
 * 
 */ 



int ProcessRequest( char * reply, int maxlen)
{
 //        this is where the action happens
 //        extract the q code and formulate a reply.
 //        the transmitting routine handles the callsigns
  
  int Siglevel;
  char buffer[1000];
  int switchcode;
  char localreply[1000];
  int amount;
  int narg;
  char fromcall[50];

  strcpy(localreply,"");
  switchcode = LookupSwitchCode(command.Qcode);
  // printf("decoded %s as %d\r\n",command.Qcode.c_str(),switchcode);

  // QRA = name of station
  switch (switchcode)
  {

    case 0: // QRA
      sprintf(localreply,"QRA %s",InfoStationName);
      break;

    case 1:  // QME
      sprintf(localreply,"QME %s",InfoStationEquip);     
      break;

    case 2:
    // here we fake a psk31 signal quality report.
    // we should be able to get this from the reciever at some point
      Siglevel = 90;
      sprintf(localreply,"QRK %d%%",Siglevel);
      break;

    case 3:
     // just sends a roger.
      sprintf(localreply,"QSL R");
      break;

    case 4:
    // replies to a say-again by sending the previous transmission
      sprintf(localreply,"QSM %s", previousreply); //
      break;

    case 5:
	// gets the date time group and sends it
      GetDTG(buffer);
      sprintf(localreply,"QTR %s", buffer);
      break;

    case 6:
    // chucks out a random drill message
      amount = atoi(command.Arg.c_str());
      if (amount > 0)
      {
		  // if they asked for a specific number then send codex groups
		  // EG  "MRV64D de MRC18 QSG 6 K"  sends a 6 group message
        CodexGroups(buffer,amount);
        sprintf(localreply,"QSG CODEX DRILL MESSAGE GROUPS %d \r\n%s",amount,buffer );
      }
      else
      {
        sprintf(localreply,"QSG Random message to be generated here");
      }
      break;

     case 7:
        // in here we format up the heard list.
        Database.LastHeardList(buffer);
        sprintf(localreply,"QUA \r\n%s", buffer);
        break;


     case 8: //QSP  send message
       narg = Database.SendMessage(command);
       sprintf(localreply,"QSP Sent as message number %d",narg);
       break;

     case 9: // "QTC" list all my mail
//       printf("%s , %d",command.FromCall.c_str(), 0);
       strcpy(fromcall,command.FromCall.c_str());
       Database.ListMessages( localreply, fromcall, 0);
       break;

      case 10:  // "QUC" list my unread mail
       strcpy(fromcall,command.FromCall.c_str());
       Database.ListMessages( localreply, fromcall, 1);    
       break;

      case 11:  // QRU read message
       strcpy(fromcall,command.FromCall.c_str());
       Database.ReadMessage( localreply, fromcall, atoi(command.Arg.c_str()) );    
       break;


      case NUM_OF_SWITCHES+1:   // no match
        strcpy(localreply,"Command Not Recognised"); //just reply callsigns get sent
        break;

     }   // End Select

  strcpy(reply,"Debug : reply too long");
  if((int)strlen(localreply)< maxlen)
  {
    strcpy(previousreply,localreply);   
    strcpy(reply,localreply);
  }
  
  return strlen(reply);
}






int easyrandom(int min, int max)
{
  int random;
  random = rand()%(max-min)+min;
  return random;
}




int CodexGroups(char * buffer, int amount)
{
  int a;
  int b;
  int position;
  int bufferpos;

  char alpha[] = "1234567890ABCDEFGHIJKLMONPQRSTUVWXYZ";
  
  bufferpos=0;
  for(a=0;a<amount;a++)
  {
    for(b=0;b<5;b++)
    {
      position = easyrandom(0,35);
      buffer[bufferpos]= alpha[position];
      bufferpos++;
    }
    buffer[bufferpos]= ' ';
    bufferpos++;  
  }
  buffer[bufferpos]=0;
  return strlen(buffer);
}







int GetMiltime(char * dtg)
{
  time_t rawtime;
  struct tm * timeinfo;  

  time ( &rawtime );
  timeinfo = localtime ( &rawtime );

  strftime(dtg,50,"%Y%m%d%H%M%S",timeinfo);
  // 20151231235959
  return strlen(dtg);
}




int GetDTG(char * dtg)
{
  time_t rawtime;
  struct tm * timeinfo;  

  time ( &rawtime );
  timeinfo = localtime ( &rawtime );

  strftime(dtg,50,"%d%H%Mz %b %Y",timeinfo);
  return strlen(dtg);
}


int GetDATETIME(char * dtg)
{
// 'YYYY-MM-DD HH:MM:SS'

  time_t rawtime;
  struct tm * timeinfo;  

  time ( &rawtime );
  timeinfo = localtime ( &rawtime );

  strftime(dtg,25,"%Y-%m-%d %H:%M:%S",timeinfo);
  return strlen(dtg);
}







//////////////////////////////////////////////////////////////////////////////////////////
//  
//     code below here is tested
//
//////////////////////////////////////////////////////////////////////////////////////////



int DataLite::ReadConfig( void)
{
  ofstream mywritefile;
  ifstream myreadfile;
  string line;

  myreadfile.open ("datarobot.ini");
  if (myreadfile.is_open())
  {
    getline (myreadfile,line);
    strcpy(MyCall,line.c_str());

    getline (myreadfile,line);
    strcpy(InfoStationName,line.c_str());

    getline (myreadfile,line);
    strcpy(InfoStationEquip,line.c_str());

    myreadfile.close();
  }
  else
  {
    mywritefile.open ("datarobot.ini");
    mywritefile << "MRE01\nRAC-CRN\nFT897 100W FULL DIPOLE NORTH SOUTH AT 4M\n";
    mywritefile.close(); 
  }
  return 0;
}


int DataLite::Initialise(void)
{
  int rc;

  rc = sqlite3_open("datarobot.db", &db);
  MakeIfMissing();

  if( rc )
  {
    printf("Can't open database: %s.\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return(1);
  }
   
 return 0;
}


int DataLite::Close(void)
{
 sqlite3_close(db);
 return 0;
}






int DataLite::MakeIfMissing(void)
{
#define LH "CREATE TABLE `tbl_lastheard` (`dtg` text,`callsign` text  )"
#define LL "CREATE TABLE `tbl_log` ('entry' integer primary key, `dtg` text,`callsign` text, 'qcode' text, 'arg' text  )"
#define MT "CREATE TABLE `tbl_mail` ('entry' integer primary key, `dtg` text,`fromcall` text,`tocall` text, 'message' text, 'status' integer )"

  int rc;
  sqlite3_stmt *statement;
  char query[200];

  strcpy(query,"SELECT * FROM tbl_log");
  rc = sqlite3_prepare(db,query,-1, &statement, 0);
  rc = sqlite3_step(statement);  
  
  if (rc != SQLITE_ROW)
  {
    // database is empty, so make new tables
    printf("Database is empty. Making new tables\r\n");
    strcpy(query,LH);
    rc = sqlite3_prepare(db,query,-1, &statement, 0);
    rc = sqlite3_step(statement);  
    sqlite3_finalize(statement); // clear the memory

    strcpy(query,LL);
    rc = sqlite3_prepare(db,query,-1, &statement, 0);
    rc = sqlite3_step(statement);  
    sqlite3_finalize(statement); // clear the memory

    strcpy(query,MT);
    rc = sqlite3_prepare(db,query,-1, &statement, 0);
    rc = sqlite3_step(statement);  
    sqlite3_finalize(statement); // clear the memory

    strcpy(query,"INSERT INTO tbl_log VALUES (1, 'none', 'none', 'none', 'none')");
    // printf("query is [%s]\r\n",querystring);
    rc = sqlite3_prepare(db,query,-1, &statement, 0);
    rc = sqlite3_step(statement);  
    sqlite3_finalize(statement); // clear the memory
  }
  else
  {
 //   printf("Database found OK\r\n");

  }

  return 0;
}


int DataLite::LogUpdate( MessageStruct command)
{
// CREATE TABLE `tbl_log` ('entry' integer, `dtg` text,`callsign` text, 'qcode' text, 'arg' text  );
  int status=0;
  char querystring[100];

  sprintf(querystring,
     "INSERT INTO tbl_log (dtg,callsign,qcode,arg) VALUES ('%s', '%s', '%s', '%s')",
      command.CallDTG.c_str() ,
      command.FromCall.c_str(),
      command.Qcode.c_str(),
      command.Arg.c_str()  );
  status = Write(querystring);
  if(status != SQLITE_DONE)   
  {
    printf("Write(querystring) error [%d]\r\nDatabase reported [%s]\r\n",status, sqlite3_errmsg(db)); 
    return 1;
  }
  return 0;
}

int DataLite::LastHeardUpdate(  MessageStruct command)
{
// CREATE TABLE `tbl_lastheard` (`dtg` text,`callsign` text  );

  int status=0;
  char querystring[100];

  sprintf(querystring,"select * from tbl_lastheard where callsign = '%s'", command.FromCall.c_str());
  status = GetRows(querystring);

  if (status >0 ) // it's present to update it
  {
    sprintf(querystring,"UPDATE tbl_lastheard SET dtg = '%s' where callsign = '%s'",  
            command.CallDTG.c_str() ,command.FromCall.c_str());
    status = Write(querystring);
  }
  else          // it's not in there already so add it
  {
    sprintf(querystring,"INSERT INTO tbl_lastheard VALUES ('%s', '%s')", 
           command.CallDTG.c_str() ,command.FromCall.c_str());
    status = Write(querystring); 
  }
 return 0;
}


int DataLite::Write( char * query)
{
  int rc;
  sqlite3_stmt *statement;
 
  rc = sqlite3_prepare(db,query,-1, &statement, 0);
  if( rc )
  {
    printf("Can't make query [%s]. \nDatabase reported [%s]\r\n", query, sqlite3_errmsg(db));
    sqlite3_close(db);
    return(rc);
  }
  rc = sqlite3_step(statement);  // process the query
  sqlite3_finalize(statement); // clear the memory

  return rc;
};


int DataLite::SendMessage( MessageStruct command)
{
  int status;
  char querystring[500];
  char datetime[50];
  char mailtext[1000];
  char * found;
  char mailto[100];

// `tbl_mail` ('entry' integer, `dtg` text,`fromcall` text,`tocall` text, 'message' text, 'status' integer );
  GetDATETIME(datetime);
  
// first strip out the To and text fields from arg.
  strcpy(querystring,command.Arg.c_str());
  strcat(querystring, " Z Z"); // dummy text to stop overflow.

  found = strchr(querystring,' '); 
  found[0] = 0;
  strcpy(mailto,querystring);
  strcpy(mailtext,found+1);

  sprintf(querystring,
     "INSERT INTO tbl_mail ( dtg,fromcall,tocall, message, status) VALUES('%s', '%s', '%s', '%s', 0  )", 
      command.CallDTG.c_str(),command.FromCall.c_str(), mailto,mailtext );

  printf("query is [%s]\r\n",querystring);
  status=0;
  status = Write(querystring);
//  status = DatabaseGetRows( (char*)"select * from tbl_mail");

  return status;
}



int DataLite::LastHeardList( char * buffer)
{
  int rc;
  sqlite3_stmt *statement;
  int rows;
  char query[100];
  char localreply[1000];

  strcpy(query,"select * from tbl_lastheard");
  rc = sqlite3_prepare(db,query,-1, &statement, 0);
  if( rc )
  {
    printf("Can't make query [%s]. \nDatabase reported [%s]\r\n", query, sqlite3_errmsg(db));
    sqlite3_close(db);
    return(1);
  }
  
  sprintf(localreply,"DATE TIME GROUP , Call\r\n")  ;
  strcpy(buffer,localreply);
  sprintf(localreply,"-----------------------\r\n");
  strcat(buffer,localreply);

  


  for(rows=0;rows<10000;rows++)
  {  
    rc = sqlite3_step(statement);  // process the query
    if (rc == SQLITE_DONE)
    {
      break;
    }
    sprintf(localreply,"%s, %s\r\n", sqlite3_column_text(statement, 0) , 
         sqlite3_column_text(statement,1));
    strcat(buffer,localreply);
  } 

  sprintf(localreply,"===== End Of List =====\r\n"); 
  strcat(buffer,localreply);
  sqlite3_finalize(statement); // clear the memory

  return strlen(buffer);
}


int DataLite::ListMessages( char * buffer, char * mailto, int status)
{
  int rc;
  sqlite3_stmt *statement;
  int rows;
  char query[500];
  char summary[1000];
  char newtag;

  sprintf(query,"select * from tbl_mail where tocall = '%s'",mailto );
  if(status ==1)
  {
    strcat(query, " and status = 0"); 
  }

  rc = sqlite3_prepare(db,query,-1, &statement, 0);
  if( rc )
  {
    printf("Can't make query [%s]. \nDatabase reported [%s]\r\n", query, sqlite3_errmsg(db));
    sqlite3_close(db);
    return(1);
  }

  sprintf(query,"\r\n Num:NEW:    Date Time    :  From  :   To   : Subject\r\n")  ;
  strcpy(buffer,query);
  for(rows=0;rows<10000;rows++)
  {  
    rc = sqlite3_step(statement);  // process the query
    if (rc == SQLITE_DONE)
    {
      if (rows==0) 
      {
        sprintf(query,"\r\nNo Messages\r\n")  ;
        buffer[0]=0;
      }
      break;
    }
// `tbl_mail` ('entry' integer, `dtg` text,`fromcall` text,`tocall` text, 'message' text, 'status' integer );

    newtag='N';
    if (sqlite3_column_int(statement,5) == 0)
    {
      newtag='Y';
    }
    sprintf(summary,"%s                    ",sqlite3_column_text(statement,4)); // pad 20 in case empty
    summary[20]=0;
    sprintf(query,"%4d: %c : %15s: %7s: %7s: %s\r\n",
        sqlite3_column_int(statement,0), newtag,
        sqlite3_column_text(statement,1), sqlite3_column_text(statement,2), 
        sqlite3_column_text(statement,3), summary);

    strcat(buffer,query);
  } 

  if (rows < 1)
  {
    sprintf(query,"\r\nNo Messages\r\n")  ;
  }
  else
  {
    sprintf(query,"===== End Of List =====\r\n"); 
  }
  strcat(buffer,query);

// At the end, we must free the memory:
//  mysql_free_result(result);
//  mysql_close(connection);

  return strlen(buffer);

}



int DataLite::ReadMessage( char * buffer, char * mailto, int messno)
{
  int rc;
  sqlite3_stmt *statement;
  char query[100];


  sprintf(query,"select * from tbl_mail where tocall = '%s' and entry = %d", mailto, messno);

  rc = sqlite3_prepare(db,query,-1, &statement, 0);
  if( rc )
  {
    printf("Can't make query [%s]. \nDatabase reported [%s]\r\n", query, sqlite3_errmsg(db));
    sqlite3_close(db);
    return(1);
  }

  rc = sqlite3_step(statement);  // process the query
//  printf("returned code was %d\r\n", rc);
  if (sqlite3_column_int(statement, 0) == 0)
  {
      sprintf(buffer,"\r\nNo message Number %i for you", messno)  ;
  }
  else
  {

// `tbl_mail` ('entry' integer, `dtg` text,`fromcall` text,`tocall` text, 'message' text, 'status' integer );

    sprintf(query,"\r\nMessage: %d\r\nDate: %s\r\nFrom: %s\r\nMessage: %s\r\n",
                 sqlite3_column_int(statement, 0), 
                 sqlite3_column_text(statement, 1) , 
                 sqlite3_column_text(statement,2), 
                 sqlite3_column_text(statement,4));
    strcpy(buffer,query);

    sprintf(query,"===== End Of List =====\r\n"); 
    strcat(buffer,query);
    sqlite3_finalize(statement); // clear the memory

    sprintf(query,"UPDATE tbl_mail SET status = 1 where entry = %d", messno);
    rc = Write(query);
  }
  return strlen(buffer);
}



int DataLite::GetRows( char * query)
{
  int rc;
  sqlite3_stmt *statement;
  int rows=0;

  rc = sqlite3_prepare(db,query,-1, &statement, 0);
  if( rc )
  {
    printf("Can't make query [%s]. \n", query);
    sqlite3_close(db);
    return(1);
  }

  for(rows=0;rows<10000;rows++)
  {  
    rc = sqlite3_step(statement);  // process the query
    if (rc == SQLITE_DONE)
    {
      break;
    }
  } 
  sqlite3_finalize(statement); // clear the memory
  return rows;
};


//
//
//  these are the only 3 routines callable
// from the outside world
//
//
//

int RobotInitialise(void)
{
  Database.Initialise();
  Database.ReadConfig();
  return 0;
}

int RobotClose(void)
{
	Database.Close();
	return 0;
}

int RobotProcessMessage(char * reply, int maxlen, char * message)
{
	char localbuffer[1000];
	printf("Processing [%s] \r\n",message);
	MessageToStruct(message);
	ProcessRequest( localbuffer, maxlen);

    sprintf(reply,"%s DE %s %s K\n.end",command.FromCall.c_str(), MyCall, localbuffer);
    return strlen(reply);
}

