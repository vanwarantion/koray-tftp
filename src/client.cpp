/*
    TFTP client UI
    
    Y Koray Kalmaz 2011
    
*/
# include <iostream>
# include "fsm.h"

/* What does this do:
    interpret command line arguments:
    if url given:
        start download: (fsm in client-direct mode)
    if not:
        load command line cli: (fsm in client mode)
*/

void showHelp(char * cmdName){
    cout << "TFTP Client" << endl << endl;
    cout << "\tUsage:" << endl;
    cout << "\t\t" << cmdName << endl;
    cout << "\t\t" << cmdName << " [remote host]" << endl;
    }

int main(int argc, char *argv[]){
    /* Arguments:
        1. host         Set default host
        2. help | ?     Show help
    */
    
    if (argc == 1) {
        tftpClient c;           // If no arguments:     FSM cli
    } else if (argc == 2) {
        tftpClient c(argv[1]);    // Host given:          FSM w/host
    } else {
        cout << "Too many arguments" << endl;
        }
    return 0;
    }
