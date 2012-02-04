/*
    TFTP server-client file system operations
    
    Y Koray Kalmaz 2011
    
    fserror codes:
    
    100 EOF reached while reading from file
*/
# include <fstream>
# include <iostream>
# include <cstring>

using namespace std;

class fileOps {
    public:
        fileOps(string fname, bool Writing, short int bSize);
        friend ostream& operator<<(ostream& output, fileOps &f);
        friend istream& operator>>(istream& input, fileOps &f);
        int close();
        int write();
        short int fserror;        // error code if any 0-100
    protected:
        int curPos;         // current address in file
        bool writeMode;     // Write mode: True or False
    private:
        char filename[256];    // File path
        short int curSeg;        // Current segment of curPos 0-7
        char * memBuffer;    // Memory buffer
        int read();
        ifstream::pos_type fileSize;    // for positioning over 2Gib
        ifstream is;        // input file object
        ofstream os;        // output file object
        short int endOffset;  // end offset for writing
        int bufferSize;
};
