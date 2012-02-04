/*  TFTP Network operations handler
    
    Y Koray Kalmaz 2011
    
    compile with -lpthread
                                                                    */
# include <iostream>            // for print:
# include <cstring>             // for strncpy:
# include <cstdlib>             // for exit() and sleep():
# include <pthread.h>           // for pthread:
# include <vector>              // for tracking thhreads:
# include <netinet/in.h>        // in_addr structure
# include <arpa/inet.h>         // inet_ntoa() to format IP address
# include <netdb.h>             // hostent struct, gethostbyname()
# include <ctime>               // for timeval in recvTimeout()
# include <cctype>              // for character case conversion

# include <fstream>             // temporary for file operations

# define THREADS 50              // max thread count
# define REQUEST_SIZE_MAX 267    // maximum request packet size
# define DATA_SIZE_MAX 516       // max data packet size


/*  OpCodes:                            */
# define	OPCODE_RRQ	    01      // read request
# define	OPCODE_WRQ	    02      // write request
# define	OPCODE_DATA	    03      // data packet
# define	OPCODE_ACK	    04      // acknowledgement
# define	OPCODE_ERROR	05      // error code

# define STATE_SERVER       1
# define STATE_RECEIVE      2
# define STATE_INITIALIZED  4
# define STATE_CONNECTED    8
# define STATE_TRANSFER     16
# define STATE_FINISHED     32
# define STATE_ERROR        128

struct	tftphdr {
    u_int16_t th_opcode;        // opcode
    union {
        u_int16_t tu_block;     // data block no
        u_int16_t tu_code;      // error code
        char tu_stuff[1];       // filename
    } th_u;
    char th_data[1];            // string for error or data
    };

using namespace std;


/*  Status Data for transfer operation */
struct operation {
    pthread_t trd;              // thread of operation
    sockaddr_in rsock;          // remote socket address
    sockaddr_in lsock;          // local socket address
    int s;                      // socket descriptor
    char buf [DATA_SIZE_MAX];   // rx-tx socket buffer
    short int id;               // operation ID
    short int state;            /* Current state bits:
                                    1       Client / Server
                                    2       Receive / Send
                                    4       initialized
                                    8       Connection
                                    16      Transfer
                                    32      Finished
                                    128     Error
                                                                    */
    long fileSize;              // size of the file being transferred
    long progress;              // bytes transferred
    char rhost [256];           // remote host address
    char rfile [256];           // filename to transmit / receive
    bool abort;                 // abort signal received
    char lastSent [512];        // last sent data (in case of resend)
    int packetNo;               // last sent packet #
    bool netascii;              // convert ascii characters
    };

struct serverSettings {
    pthread_t trd;              // thread of server
    int port;                   // listening port
    sockaddr_in lsock;          // local socket address
    sockaddr_in * rsock;          // remote socket address (temporary)
    int s;                      // socket descriptor
    int rbl;                    // request buffer lenght
    
    long int gTimeout;          // general timeout
    long int pTimeout;          // per packet timeout
    bool abort;                 // abort signal for server
    bool running;               // server is running
    };

class rxtx {
    public:
        rxtx();
        int startServer(int port);          // server listener
        int startClient(char * host, char * file, bool send);
        int abort(short int opID);          // abort operation #opID
        void printList();                   // print summary
        void getOps(int * rv, int & rvp);   // active ops
        char * nameLookup(char * host);     // hostname lookup
    private:
        pthread_attr_t attr;                // default thread attributes
        void * status;                      // for join threads
        operation * op(short int opID);     // return pointer to #opID
        char * stateHr(short int * z);             // translate status no
    };
