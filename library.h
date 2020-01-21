

class Server
{
  public: 
	 int Write(char * message);
	 int Read( char * buffer);
	 
  private:
    int GetPage(char * buffer, char * url);
};
