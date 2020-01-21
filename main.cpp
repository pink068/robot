#include <sys/types.h>
#include <string>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include "library.h"

//#include <cstdio>
//#include <cstdlib>

//#include "datarobot.h"
//#include <time.h>
//#include <curl/curl.h>


using namespace std;


/*
 * this is the documentation for the program
 * this file main.cpp is basically just a loop 
 * after setting up the database and other memory it just 
 * 1 - waits for a string input from getInput
 * 2 - converts it to a structure for easier accessing
 * 3 - processes that structure to make a reply
 * 4 - updates the log
 * 5 - updates the last heard stations list
 * 6 - transmits the reply
 * 
 * 
 * library.cpp is just the home for all the routines. 
 * makereply processes the string that came in from the reciever 
 * and makes up a single string to be transmitted via the psk program 
 * as the reply to the user
 * 
 * 
 * the program still needs to do some housekeeping
 * ToDo list.  
 * 1 - delete really old contacts from the Heard list
 * 2 - read the real signal quality report from the PSK handler.
 * 
 */

int main(int argc, char **argv)
{
  int rxlength;
  char buffer[300];
  Server chd;
  char me[8];
//  RobotInitialise(); 
  
  
  strcpy(me,"MRP99");
  
  do
  {
    sleep(2);
    rxlength = chd.Read(buffer);
    
    if (rxlength> 8)
    {
      printf("buffer length=%d\n",rxlength); 
      cout << "rx_data: " << buffer << "\n";
      if(strncmp(buffer,me,5) ==0)
      {
         cout << "process the buffer " << endl;	  
         
	  }
	  else
	  {
	    // it's not for me, ignore it.
        cout << "Not for me " << endl;	
	  }
      
    }  // end of rxlength
       
  } while (1);
  
//  RobotClose();
  return 0;
}




