#include <sys/types.h>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <time.h>
#include "flxml.h"

// using namespace XmlRpc;
//XmlRpcClient c ("localhost",7362);

int flxml::Initialise(void)
{

    return 0;
}
int flxml::ReadString(char * rxbuffer, const char * request)
{

  return strlen(rxbuffer);
}

int flxml::ReadCharArray(char * rxbuffer, const char * request)
{

  return strlen(rxbuffer);
}

static void flxml:: dieIfFaultOccurred (xmlrpc_env * const envP)
{
    if (envP->fault_occurred) {
        fprintf(stderr, "ERROR: %s (%d)\n",
                envP->fault_string, envP->fault_code);
        exit(1);
    }
}

