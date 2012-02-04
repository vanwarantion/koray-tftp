/*
    TFTP server UI
    
    Y Koray Kalmaz 2011
    
*/
# include <iostream>
# include "fsm.h"

void showHelp(char * cmdName){
    cout << "TFTP Server" << endl << endl;
    cout << "\tUsage:" << endl;
    cout << "\t\t" << cmdName << endl;
    cout << "\t\t" << cmdName << " [port]" << endl;
    }


int main(int argc, char *argv[]){
    /* Arguments:
        port
    */
    if (argc == 1) {
        tftpServer s;           // If no arguments:     FSM cli
    } else if (argc == 2) {
        // TODO: Check integer
        tftpServer s( (int) argv[1]);    // Port given:          FSM w/port
    } else {
        cout << "Too many arguments" << endl;
        }
    return 0;
    }
