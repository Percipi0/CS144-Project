#include "socket.hh"
#include "address.hh"
#include <cstdlib>
#include <iostream>
#include <span>
#include <string>

using namespace std;
// use TCPSocket & Address classes
// cs144.keithw.org /hello
void get_URL( const string& host, const string& path )
{
  const Address curAddress = Address(host, "http");
  Socket curSocket = TCPSocket();
  curSocket.connect(curAddress);
  curSocket.write("GET " + path + " HTTP/1.1\r\n");
  curSocket.write("HOST: " + host + "\r\n");
  curSocket.write("Connection: close\r\n");
  curSocket.write("\r\n");

  std::string test;
  while(!curSocket.eof()) {
    curSocket.read(test);
    cout << test;
  }

}

int main( int argc, char* argv[] )
{
  try {
    if ( argc <= 0 ) {
      abort(); // For sticklers: don't try to access argv[0] if argc <= 0.
    }

    auto args = span( argv, argc );

    // The program takes two command-line arguments: the hostname and "path" part of the URL.
    // Print the usage message unless there are these two arguments (plus the program name
    // itself, so arg count = 3 in total).
    if ( argc != 3 ) {
      cerr << "Usage: " << args.front() << " HOST PATH\n";
      cerr << "\tExample: " << args.front() << " stanford.edu /class/cs144\n";
      return EXIT_FAILURE;
    }

    // Get the command-line arguments.
    const string host { args[1] };
    const string path { args[2] };

    // Call the student-written function.
    get_URL( host, path );
  } catch ( const exception& e ) {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
