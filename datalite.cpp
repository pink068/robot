using namespace std;
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "sqlite3.h"
#include "datarobot.h"
#include <iostream>
#include <fstream>


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







/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2015, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/
/* <DESC>
 * Shows how the write callback function can be used to download data into a
 * chunk of memory instead of storing it in a file.
 * </DESC>
 */



struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

int Server::GetPage(char * buffer, char * url)
{
  CURL *curl_handle;
  CURLcode res;


  struct MemoryStruct chunk;

  chunk.memory = (char *) malloc(1);  /* will be grown as needed by the realloc above */
  chunk.size = 0;    /* no data at this point */

  curl_global_init(CURL_GLOBAL_ALL);

  /* init the curl session */
  curl_handle = curl_easy_init();

  /* specify URL to get */
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);
  /* send all data to this function  */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

  /* we pass our 'chunk' struct to the callback function */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

  /* some servers don't like requests that are made without a user-agent
     field, so we provide one */
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

  /* get it! */
  res = curl_easy_perform(curl_handle);

  /* check for errors */
  if(res != CURLE_OK) 
  {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
  }
  else 
  {
    /*
     * Now, our chunk.memory points to a memory block that is chunk.size
     * bytes big and contains the remote file.
     *
     * Do something nice with it!
     */

//    printf("[%s]\n", chunk.memory); 
//    printf("%lu bytes retrieved\n", (long)chunk.size);
    chunk.memory[chunk.size]=0;
    strcpy(buffer, (char *)chunk.memory);
  }

  /* cleanup curl stuff */
  curl_easy_cleanup(curl_handle);

  free(chunk.memory);

  /* we're done with libcurl, so clean it up */
  curl_global_cleanup();

  return (int)chunk.size;
}
















