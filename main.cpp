#include <sys/types.h>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include "datarobot.h"
#include <time.h>
#include "myxmlrpc.h"

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

 

int main(int argc, char **argv)
{
  int rxlength;
  char buffer[300];
  
  rxlength = XmlRpc_ReadString(buffer, "fldigi.name_version");
  cout << "Appname: " <<buffer  << "\n";
 
  RobotInitialise(); 

  do
  {
    sleep(2);
    rxlength = XmlRpc_ReadInt("text.get_rx_length");
    if (rxlength> 0)
    {
      printf("buffer length=%d\n",rxlength); 
      rxlength = XmlRpc_ReadCharArray(buffer, "rxtx.get_data");
      cout << "rx_data: " << buffer << "\n";
// rx.get_data


//      if (c.execute("text.clear_rx", noArgs, Sresult))
//        std::cout << "cleared rx buffer\n";
    }  // end of rxlength
       
  } while (1);
  
  RobotClose();
  XmlRpc_Close();
  return 0;
}


#ifdef sjfhsfk


int TransmitString(char* txmessage, int messlength)
{
  int rxlength;
  XmlRpcValue IArg, noArgs, Iresult, SArg;
  printf("Transmitting [%s] \r\n",txmessage); 
  
  IArg = 1000;
  c.execute("modem.set_carrier", IArg, Iresult);
  c.execute("main.tx", noArgs, Iresult);
  
  c.execute("text.clear_rx", noArgs, Iresult);  
  
  SArg=txmessage;
  c.execute("text.add_tx", SArg, Iresult);

  // wait for TX end
  do
  { 
     sleep(2);
    if (c.execute("text.get_rx_length", noArgs, Iresult))
      rxlength =  Iresult;
    else
      std::cout << "Error calling 'rx_length'\n";
  } while ((int)rxlength < messlength);
  sleep(2); 

  c.execute("main.rx", noArgs, Iresult);  
  return 0;
}



//    strcpy(rxmessage,dummy_messages[0]);
//    messlength = RobotProcessMessage(txmessage,500,rxmessage);
    
/*    
    // every hour send the time
    GetMiltime(NowTime);
      // 20151231235959
      // 0123456789abcd
    NowTime[10]=0;    // end string at hours
    NowHour = atoi(&NowTime[8]);
    printf("Time:%i/%i\n",NowHour,LastHour);
	
		if (NowHour != LastHour)
		{
			LastHour = NowHour;
			// transmit the time.
			GetMiltime(NowTime);
			sprintf(txmessage,"DE %s QTR %s\n.end",MyCall,NowTime);
			TransmitString(txmessage, strlen(txmessage));
		}

		//    TransmitString(txmessage, messlength);
  */



/*
Libraries have been installed in:
   /usr/local/lib

If you ever happen to want to link against installed libraries
in a given directory, LIBDIR, you must either use libtool, and
specify the full pathname of the library, or use the `-LLIBDIR'
flag during linking and do at least one of the following:
   - add LIBDIR to the `LD_LIBRARY_PATH' environment variable
     during execution
   - add LIBDIR to the `LD_RUN_PATH' environment variable
     during linking
   - use the `-Wl,-rpath -Wl,LIBDIR' linker flag
   - have your system administrator add LIBDIR to `/etc/ld.so.conf'

See any operating system documentation about shared libraries for
more information, such as the ld(1) and ld.so(8) manual pages.
*/
#endif
