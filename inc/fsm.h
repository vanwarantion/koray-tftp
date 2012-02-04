/*
    TFTP UI
    
    Y Koray Kalmaz 2011
    
*/
#ifndef FSM_H
#define FSM_H

# include <algorithm>
# include <iostream>
# include <istream>
# include <iterator>
# include <cstdlib>
# include <sstream>
# include <string>
# include <strings.h>
# include <vector>

# include "netops.h"

using namespace std;


class fsm {
    public:
        fsm();
        short int getState();
        void dw(char * hede);
    protected:
        short int curState;         // 0:idle, 1:sending, 2:receiving
        struct task {
            string cmdName;
            string helpText;
            };
        vector <task> commands;     // commands list
        int rexmt;                  // general packet timeout
        int sessionTimeout;         // general timeout
        bool verboseMode;           // verbose mode
        rxtx myNet;                 // network object
        void abort(vector <string> arg);        // abort operation opID
        void printStatus();                     // put, get, timeouts
    };

class tftpServer : fsm {
    public:
        tftpServer();                   // listen port 69
        tftpServer(int listenPort);     // start with custom port
    private:
        struct transfer {
            string clientAddress;
            short int tMode;                    // 1: binary, 2: ascii
            long int fileSize;                  // total size in bytes
            long int completed;                 // bytes done
            };
        vector <transfer> clients;
        int port;                               // listening port
        void runCommand(vector<string> tokens); // server interpreter
        int startCLI();                         // command cursor
        void fillCommands();                    // fill server commands
        void checkCommand(char cmdLine [256]);  // written command
        rxtx mysrv;                             // server object
    };

class tftpClient : fsm {
    public:
        tftpClient();
        tftpClient(char * rHost);
        int startCLI();                         // command cursor
        short int transferMode(string mode);    // Not implemented
        void checkCommand(char cmdLine [256]);  // written command
    private:
        char remoteAddress [256];               // server address
        short int tMode;                        // 1: binary, 2: ascii
        long int fileSize;                      // total size in bytes
        long int completed;                     // bytes done
        void runCommand(vector<string> tokens); // client interpreter
        void fillCommands();                    // fill client commands
        int beginTransfer(vector <string> tkn);
    };

#endif
