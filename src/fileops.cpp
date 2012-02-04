/*
    TFTP server-client file system operations
    
    Y Koray Kalmaz 2011
    
*/

# include "fileops.h"

/*  TODO:
    Seperate threads for disk i/o and memory buffer
*/


/*
fileOps class:
    create memory buffer for fileOps::bufferSize (8 * 512 byte segments)
    opens a file for read or write (not both!) (up to 32Gib)    */
fileOps::fileOps(string fname, bool Writing, short int bSize){
    // reset fserror:
    fserror = 0;
    cout << "  Object Constructor." << endl;
    // memory buffer size:
    bufferSize = bSize * 1024;
    cout << "  Buffer size: " << bufferSize << endl;
    // memory buffer:
    memBuffer = new char [bufferSize];
    // EOF position (maximum)
    endOffset = bufferSize;
    // set write mode on/off
    writeMode = Writing;
    // cast char array from string
    int i = fname.copy(filename, fname.size(), 0);
    filename[i]='\0';
    // current position:
    curPos = 0;
    // current segment of position
    curSeg = 0;
    // return and wait for write calls:
    if (writeMode == true){
        // Open file for write:
        os.open(filename, ios::binary);
        cout << "  Write mode selected" << endl;
    } else {
        cout << "  Read mode selected. Open file: " << filename << endl;
        is.open(filename, ios::binary);
        // Set fileSize as 0
        fileSize = 0;
        // read first chunk:
        fileOps::read();
        cout << "  Buffer written" << endl;
        }
    }

/*  closes file (saves if write mode = True)                    */
int fileOps::close(){
    is.close();
    return 0;
    }

/*  reset memBuffer and curSeg.
    read file & fill buffer. (4Kib is usually the fastest way to read)
    increase curPos by 1                                        */
int fileOps::read(){
    // TODO: Error check for file access
    // Reset curSeg
    cout << "    Reading from " << filename << endl;
    curSeg = 0;
    if (fileSize == 0){
        // Get file size
        cout << "    File size: ";
        is.seekg (0, ios::end);
        fileSize = is.tellg();
        is.seekg (0, ios::beg);
        cout << fileSize << " bytes" << endl;
        }
    // Seek to address:
    cout << "    Read " << bufferSize << " bytes from " << curPos * bufferSize << endl;
    is.seekg( curPos * bufferSize, ios::beg );
    // Write to buffer:
    is.read(memBuffer, bufferSize);
    // Zerofill rest of the buffer if EOF:
    if (is.eof()) {
        cout << "    EOF detected at " << is.gcount() << endl;
        std::fill(memBuffer + is.gcount(), memBuffer + bufferSize, 0);
        endOffset = fileSize % bufferSize;
        close();
        }
    cout << "    Read done." << endl;
    curPos++;
    return 0;
    }

/*  append  memBuffer to file
    if write size < buffer size then set EOF;
    increase curPos by 1
    reset memBuffer and curSeg.                                 */
int fileOps::write(){
    // Seek to end:
    os.seekp (0, ios::end);
    // Check if write size < 4Kib:
    if (memBuffer[bufferSize - 1]==0){
        // Write buffer with ending offset
        os.write(memBuffer, endOffset);
        close();
        cout << "    Write EOF" << endl;
    } else {
        // Write buffer normally:
        os.write(memBuffer, bufferSize);
        // Reset memBuffer:
        std::fill(memBuffer, memBuffer + bufferSize, 0);
        cout << "    Buffer flushed" << endl;
        }
    curPos++;
    curSeg = 0;
    return 0;
    }

/*  reads from istream and puts to memBuffer at position 512*curSeg
    increase curSeg by 1
    if curSeg = maxSegment then set endOffset & write()         */
istream& operator>>(istream& input, fileOps &f){
    char x [512];
    input.seekg( 0, ios::beg );
    
    input.read(x, 512);
    short int lastbytes = input.gcount();
    cout << "    lastbytes: " << lastbytes << endl;
    memcpy(f.memBuffer + (f.curSeg * 512), x, lastbytes);
    cout << "    Current Segment: " << (int) f.curSeg;
    cout << "\tBuffer size: " << (int) f.bufferSize << endl;
    if (lastbytes < 512) {
        cout << "    EOF detected" << endl;
        f.endOffset = (f.curSeg * 512) + lastbytes;
        }
    f.curSeg++;
    if ((f.curSeg == f.bufferSize / 512) or (f.endOffset != f.bufferSize)){
        cout << "  Calling diskwrite" << endl;
        f.write();
        }
    return input;
    }

/*  writes 512 bytes from position 512*curSeg to ostream
    or less than 512 bytes if EOF
    increase curSeg by 1
    if curSeg = maxSegment then read()                          */
ostream& operator<<(ostream& output, fileOps &f){
    if (f.fserror == 100) {
        cout << "  NULL" << endl;
        return output;
        }
    int startOffset = f.curSeg * 512;
    char * x;
    if (f.endOffset < startOffset + 512) {
        // EOF
        cout << "    Reading last " << f.endOffset - startOffset;
        cout << " bytes" << " from " << f.endOffset << endl;

        x = new char [f.endOffset - startOffset + 1];

        memcpy(x, f.memBuffer + startOffset, f.endOffset - startOffset);

        x[f.endOffset - startOffset] = '\0';
        f.fserror = 100;
    } else {
        // Normal
        cout << "    Reading 512 bytes from ";
        cout << startOffset << " to " << startOffset + 512;
        cout << " as segment: " << f.curSeg << endl;
        x = new char [513];
        memcpy(x, f.memBuffer + startOffset, 512);
        f.curSeg++;
        if (f.curSeg == f.bufferSize / 512) {
            cout << "    Buffer needs refresh" << endl;
            f.read();
            }
        x[512] = 0x00;
        }
    // output.clear();
    output << x;
    return output;
    }
