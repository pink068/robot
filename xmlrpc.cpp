#include "XmlRpc.h"
#include <sys/types.h>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <time.h>

using namespace XmlRpc;
XmlRpcClient c ("localhost",7362);

int XmlRpc_ReadString(char * rxbuffer, char * request)
{
   XmlRpcValue noArgs, Sresult;
  if (c.execute(request, noArgs, Sresult))
  {
    strcpy(rxbuffer, Sresult);
  }
  else
  {
    strcpy(rxbuffer, "");    
  }
  return strlen(rxbuffer);
}


int XmlRpc_ReadInt(char * request)
{
  XmlRpcValue noArgs, Iresult;
  int reply;
  if (c.execute(request, noArgs, Iresult))
  {
    reply = result);
  }
  else
  {
    strcpy(rxbuffer, "");    
  }
  return strlen(rxbuffer);
}
