/*  TFTP Network operations handler
    
    Y Koray Kalmaz 2011
    
    compile with -lpthread
                                                                    */
/*  Protocol:
1.  Request (WRQ or RRQ)
2.  acknowledgment packet for WRQ, or the first data packet for RRQ.
3.  Data block number is 0 for WRQ acknowledgement.
4.  Reject all other TIDs' data packets.
5.  Finish transfer with data packet under 512 byte.
                                                                    */
/*  Packet types:
    
        RRQ/WRQ:    Max size:   524
        2 bytes  | string   |1 byte|string 8 byte |1 byte
        ------------------------------------
        | Opcode | Filename | 0 | Mode | 0 |
        ------------------------------------
        
        Opcode:     1: RRQ, 2: WRQ
        Filename:   Chars + '\0'
        Mode:       Case insensitive string (netascii, binary, mail)
        
        
        DATA:       Max size:   516
        2 bytes 2 bytes n bytes
        ---------------------------
        | Opcode | Block # | Data |
        ---------------------------
        
        Opcode:     3
        Block #:    Block number
        Data:       Up to 512 byte
        
        
        ACK:        Max size:   4
        2 bytes 2 bytes
        --------------------
        | Opcode | Block # |
        --------------------
        
        Opcode:     4
        
        
        ERROR:      Max size:   86
        2 bytes 2 bytes string 1 byte
        -----------------------------------
        | Opcode | ErrorCode | ErrMsg | 0 |
        -----------------------------------
        
        Opcode:     5
    
    
    Error Codes:
    
    0   Not defined, see error message (if any).
    1   File not found.
    2   Access violation.
    3   Disk full or allocation exceeded.
    4   Illegal TFTP operation.
    5   Unknown transfer ID.
    6   File already exists.
    7   No such user.
*/

# include "netops.h"

using namespace std;

operation ops [THREADS];            // data for threads
int lst[THREADS];                   // for getOps()
serverSettings srvset;              // server settings
short int counter = 0;              // thread ID index
pthread_mutex_t mutexStats;         // mutex to protect thread data
pthread_mutex_t mutexSrvset;        // mutex to protect srvset data
pthread_attr_t attr;                // thread attribute

void packetDump(char * buf, short int blen){
    int i = 0;
    cout << "Dumping " << blen << " bytes:\n";
    while (i < blen) {
        cout << hex << noshowbase << (int) * (buf + i) << ", ";
        i++;
        }
    cout << "Dumping " << blen << " bytes:\n";
    }

// change integer to network byte ordered char array
void itoc(short int * i, char * c){
    uint16_t t = htons(*i);
    memcpy(c, &t, 2);
    }

// change network byte ordered char array to short integer
void ctoi(char * c, short int * i){
    uint16_t * t = new uint16_t;
    memmove(t, c, 2);
    *i = ntohs(*t);
    }

// reads from buf, writes to packetNo and data
// TODO: cleanup
void readDataPacket(char * buf, short int * recvSize, short int * oc, 
                                        int * pNo, char * data){
    
    ctoi(buf, oc);                          // op code
    
    ctoi(buf + 2, (short int *) pNo);       // packet no
    if (*recvSize > 4) {
        memmove(data, buf + 4, (size_t) (*recvSize) - 4); // Ever.else to data
        cout << "memmove ok\n";
        }
//~     unsigned short * b = static_cast<unsigned short*>(static_cast<void*>(ts));
//~     *packetNo = *b;
    }

// fb:from buffer, fb size, tb:to buffer, packet no, op code
void makePacket(char * fb, int *fs, char * tb, short int * pNo, 
                                                    short int oCode){
    cout << "Make packet:\n";
    
    cout << "   makePacket: size: " << *fs << endl;
    cout << "   pNo: " << *pNo << endl;
    cout << "   OpCode: " << oCode << endl;
    
    memset(tb, 0, *fs + 4);     // clear target buffer

    itoc(&oCode, tb);                // write op code
    itoc(pNo, tb + 2);            // write packet no
    if (*fs > 0) {
        memmove(tb + 4, fb, *fs);   // write buffer
        }
    }

// returns true if receive timeouts
bool recvPacket(int sock, char * data, size_t dsm, sockaddr_in * sockfrom, 
                    size_t *soclen, int timeout, short int * recvb) {
    fd_set fds;         // file descriptor
    struct timeval t;   // timeout
    
    if (timeout > 999) {
        t.tv_sec = timeout / 1000;
        t.tv_usec = (timeout - (t.tv_sec * 1000)) * 1000;
    } else {
        t.tv_sec = 0;
        t.tv_usec = timeout * 1000;
        }
//~     cout << "Timeout sec : " << t.tv_sec << endl;
//~     cout << "Timeout usec: " << t.tv_usec << endl;
    
    FD_ZERO(&fds);
    FD_SET(sock, &fds);
    int rc = select(sizeof(fds)*4, &fds, NULL, NULL, &t);
    if (rc==-1) {
        cout << "FATAL; select() failed\n";
        exit(-1);}
    if (rc == 0) {
        cout << "   recvfrom() Timeout!\n";
        return true;}    // timeout
    //* recvb = recvfrom(sock, data, DATA_SIZE_MAX, 0, (sockaddr *) sockfrom, soclen);
    * recvb = recvfrom(sock, data, dsm, 0, (sockaddr *) sockfrom, soclen);
    return false;
    }

// TODO: Use memcpy instead
void makeACK(char * buf, int * pNo){
    char * tmp = new char;
    int * tmpi = new int;
    *tmpi = 0;
    makePacket(tmp, tmpi, buf, (short int *) pNo, 4);
    }

static void * tx(void * o){
    int s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (s!=0) {
        cout << "FATAL; pthread_setcancelstate" << endl;
        exit(-1);
        }
    operation * op = (operation *) o;
    
    
    cout << "TX loading for: ";
    cout << inet_ntoa(op->rsock.sin_addr) << ':';
    cout << ntohs(op->rsock.sin_port) << endl;
    // open file for write:
    ifstream is;        // input file object
    is.open(op->rfile, ios::binary);
    
    // Create socket here
    socklen_t slen=sizeof(op->rsock);    // other socket memory size
    op->s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (op->s == -1){
        cout << op->id << ": FATAL; Can not create socket!\n";
        exit(-1);
        }
    // bind:
    op->lsock.sin_family = AF_INET;                 // internet
    op->lsock.sin_addr.s_addr = htonl(INADDR_ANY);  // listen from any IP
    
    if (bind(op->s, (struct sockaddr *) &op->lsock, 
                                            sizeof(op->lsock))==-1) {
        cout << "FATAL; bind() failed!\n";
        exit(-1);
        }
    getsockname(op->s,(struct sockaddr *)&op->lsock,&slen);
    
    // set op->state to initialized
    op->state = op->state | STATE_INITIALIZED;
    char * sndbuf;
    int sndlen = 0;
    op->packetNo = 0;   // Set current packet no to FIRST
    if ( (op->state ^ STATE_SERVER) >= op->state) {
        // if client: packet is RRQ (00 01 Filename 00 Mode 00)
        char mtxt [] = "binary";  // TODO: Fix this
        sndlen = 4 + strlen(op->rfile) + strlen(mtxt);
        sndbuf = new char [sndlen];
        sndbuf[0] = 0x00;
        sndbuf[1] = 0x02;   // 1:RRQ, 2:WRQ
        strncpy(sndbuf + 2, op->rfile, strlen(op->rfile));
        sndbuf[strlen(op->rfile) + 2] = 0x00;
        strncpy(sndbuf + 3 + strlen(op->rfile), mtxt, strlen(mtxt));
        sndbuf[strlen(op->rfile) + strlen(mtxt) + 3] = 0x00;
        
        cout << "I am a client:\n";
        srvset.gTimeout = 5000;
        srvset.pTimeout = 2000;
        }
    clock_t reqStart;
    char * ackbuf;    // ACK buffer
    ackbuf = new char [4];
    short int acklen;           // returned ACK packket size
    short int * tmpPacketno = new short int;      // temporary packete no pointer
    short int tmpOc;             // temporary ACK opcode
    char * tmpAckb = new char [50]; // temporary error message buffer
    while (op->abort == false) {
        //  if op->state = connection: 
        if ( (op->state ^ STATE_CONNECTED) < op->state) {
            reqStart = clock();
            while (clock() < reqStart + srvset.gTimeout) {
                // send sndbuf
                cout << "   Sending packet\n";
                if (sendto(op->s, sndbuf, sndlen, 0, 
                                (struct sockaddr *) &op->rsock, slen)==-1){
                    cout << "ERROR; sendto() failed!\n";
                    exit(-1);}
                // Don't wait for last ACK
                if ( (op->state ^ STATE_FINISHED) < op->state) {
                    break;}
                
                /*  Listen for ACK until rexmt;
    bool recvPacket(
        int sock,                   socket
        char * data,                pointer receive to buffer
        size_t dsm,                 buffer size
        sockaddr_in * sockfrom,     remote socket
        size_t *soclen,             socket len
        int timeout,                
        short int * recvb)          pointer received data size
        
                recvPacket(op->s, op->buf, DATA_SIZE_MAX, 
                &op->rsock, &slen, srvset.pTimeout, 
                                                &recvBytes
        */
                if (recvPacket(op->s, ackbuf, 4, &op->rsock, &slen, 
                                srvset.pTimeout, &acklen) == true) {
                    op->state = op->state | STATE_ERROR;
                    cout << "   re-Sending\n";
                } else {
                    cout << "Received something\n";
                    break;
                    }
                }
            if ( (op->state ^ STATE_ERROR) < op->state) {
                cout << op->id << "; TIMEOUT; No ACK returned for packet: ";
                cout << op->packetNo << endl;
                break;}
            op->state = op->state | STATE_TRANSFER;
        } else {
            // set op->state to connection
            if (inet_aton(op->rhost, &op->rsock.sin_addr)==0) {
                cout << "inet_aton() failed\n";
                exit(1);
                }
            op->state = op->state | STATE_CONNECTED;
            if ( (op->state ^ STATE_SERVER) >= op->state) {
                cout << "I will send WRQ\n";
                // Set remote port to 69
                // TODO: Get port info from fsm
                op->rsock.sin_port = htons(69);
                continue;   // and send WRQ
                }
            }
/*      check ACK:
void readDataPacket(
        char * buf,             incomming packet
        short int * recvSize,   packet size
        short int * oc,         op code
        int * pNo,              packet no
        char * data)            packet data
*/
        readDataPacket(ackbuf, &acklen, &tmpOc, (int *) tmpPacketno, tmpAckb);
        cout << "   Op code: 4; " << tmpOc << endl;
        cout << "      ACK : " << op->packetNo << "; " << *tmpPacketno << endl;
        if (op->packetNo == *tmpPacketno) {
            op->packetNo++;
        } else {
            if ((tmpOc != 1) & ( (op->state ^ STATE_FINISHED) < op->state)) {
                cout << "ERROR; Something as gone wrong!\n";
                packetDump(ackbuf, acklen);
                }
            }
        // if EOF: break;
        if ( (op->state ^ STATE_FINISHED) < op->state) {
            break;}
        // read file
        is.read((char *) op->lastSent, 512);
        // prepare packet
        if (sndlen != 4 + is.gcount()) {    // if EOF or first packet
            if (is.gcount() < 512) {
                cout << "Last packet\n";
                op->state = op->state | STATE_FINISHED;}
            sndlen = 4 + is.gcount();       // change buffer size
            sndbuf = new char [sndlen];
            }
        int fbs = sndlen - 4;
        *tmpPacketno = op->packetNo;
        makePacket((char *)op->lastSent, &fbs, sndbuf, tmpPacketno, 3);
        }
    is.close();
    cout << "TX; Exiting: " << op->id << endl;
    if ( (op->state ^ STATE_SERVER) >= op->state) {
        exit(0);
        }
    pthread_exit((void*) o);
    }

void killSocket( short int * st ) {
    short int s = *st;
    s = s | STATE_ERROR;
    }

// TODO: Second run of this function fails. Fix it!
static void * rx(void * o){
    int s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    operation * op = (operation *) o;
    if (s!=0) {
        cout << op->id << ": FATAL; pthread_setcancelstate" << endl;
        exit(-1);
        }
    
    
    if (inet_aton(op->rhost, &op->rsock.sin_addr)==0) {
        cout << "inet_aton() failed\n";
        exit(1);
        }
    
    cout << "RX; loading for: ";
    cout << inet_ntoa(op->rsock.sin_addr) << ':';
    cout << ntohs(op->rsock.sin_port) << endl;
    
    // open file for write:
    ofstream of;        // output file object
    of.open(op->rfile, ios::binary);
    
    // Create socket here
    socklen_t slen=sizeof(op->rsock);    // other socket memory size
    /* kernel will figure out what protocol to use if 0 is given 
    instead of IPPROTO_UDP */
    op->s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (op->s == -1){
        cout << op->id << ": FATAL; Can not create socket!\n";
        exit(-1);
        }
    // bind:
    op->lsock.sin_family = AF_INET;                 // internet
    op->lsock.sin_addr.s_addr = htonl(INADDR_ANY);  // listen from any IP
    // inet_pton(AF_INET, op->rhost, &op->lsock.sin_addr.s_addr); 
    
    if (bind(op->s, (struct sockaddr *) &op->lsock, 
                                            sizeof(op->lsock))==-1) {
        cout << "FATAL; bind() failed!\n";
        exit(-1);
        }
    // set op->state to initialized
    op->state = op->state | STATE_INITIALIZED;
    cout << "State: Initialize\n";
    char * sndbuf;
    int sndlen = 0;
    if ( (op->state ^ STATE_SERVER) >= op->state) {
        // if client: packet is RRQ (00 01 Filename 00 Mode 00)
        char mtxt [] = "binary";  // TODO: Fix this
        sndlen = 4 + strlen(op->rfile) + strlen(mtxt);
        sndbuf = new char [sndlen];
        sndbuf[0] = 0x00;
        sndbuf[1] = 0x01;
        strncpy(sndbuf + 2, op->rfile, strlen(op->rfile));
        sndbuf[strlen(op->rfile) + 2] = 0x00;
        strncpy(sndbuf + 3 + strlen(op->rfile), mtxt, strlen(mtxt));
        sndbuf[strlen(op->rfile) + strlen(mtxt) + 3] = 0x00;
        // Set remote port to 69
        // TODO: Get port info from fsm
        op->rsock.sin_port = htons(69);
        cout << "I am a client:\n";
        srvset.gTimeout = 5000;
        srvset.pTimeout = 2000;
        packetDump(sndbuf, (short int) sndlen);
    } else {
        // if server: packet is ACK with #0 (00 04 00 00)
        sndbuf = new char [4];
        sndbuf[1] = 0x04;
        sndlen = 4;
        cout << "I am a SERVER\n";
        }
    // set op->state to Connection
    op->state = op->state | STATE_CONNECTED;
    cout << "State: Connected\n";
    // initialize write-behind object
    op->packetNo = 0;
    int currentPacketno = 0;
    op->progress = 0;
    while (op->abort == false) {
        if (op->progress < 512) {
            op->state = op->state | STATE_TRANSFER;
            cout << "State: Transfer (op->progress < 512)\n";
            cout << "Session timeout: " << srvset.gTimeout << endl;
            }
        short int recvBytes = 0;
        memset(&op->buf, 0, DATA_SIZE_MAX);     // flush buffer
        clock_t reqStart = clock();         // general timeout check
        while (clock() < reqStart + srvset.gTimeout) {
            // send ACK or RRQ
            cout << "Sending ACK: " << op->packetNo << endl;
            if (sendto(op->s, sndbuf, sndlen, 0, 
                    (struct sockaddr *) &op->rsock, slen)==-1){
                cout << "ERROR; sendto() failed!\n";
                exit(-1);
                }
            cout << "Sent Packet\n";
            // if not finished: wait new packet
            if ( (op->state ^ STATE_FINISHED) >= op->state) {
                cout << "Listening for next data packet\n";
                if (recvPacket(op->s, op->buf, DATA_SIZE_MAX, 
                &op->rsock, &slen, srvset.pTimeout, 
                                                &recvBytes) == false) {
                    break;  // no timeout
                    }
                cout << "Got some packet\n";
            } else {
                break;  // sent last ACK
                }
            }
        // Check rhost, rport
//~         cout << "IP: " << inet_ntoa(srvset.rsock->sin_addr) << endl;
//~         
//~         char * kxtmp = inet_ntoa(op->rsock.sin_addr);
//~         if (strcmp((char *) op->rhost, kxtmp) != 0) {
//~             cout << "WARNING; Got data from wrong host. Ignoring...\n";
//~             cout << "...Expected to get from: " << op->rhost << endl;
//~             cout << "...Instead i got from  : " << kxtmp << endl;
//~             //cout << inet_ntoa(op->lsock.sin_addr) << endl;
//~             continue; // try listening again
//~             }
        // TODO: Check op code
        // Did we just send last ACK?
        if ( (op->state ^ STATE_FINISHED) < op->state) {
            break;
            }
        // check packet #
        memset(&op->lastSent, 0, 512);     // flush buffer
        short int ackOpcode = 0;
        // Read packet
        readDataPacket((char *) op->buf, &recvBytes, 
                            &ackOpcode, &currentPacketno, op->lastSent);
        // check packet:
        if (currentPacketno != op->packetNo + 1) {
            cout << "Current state  : " << op->state << endl;
            cout << "Expected packet: " << op->packetNo + 1 << endl;
            cout << "Received packet: " << currentPacketno << endl;
            cout << "Received OpCode: " << ackOpcode << endl;
            packetDump((char *) op->buf, recvBytes);
            break;
            //continue;
            }
        // if after 1 packet: set op->state to Transfer
        if (op->packetNo == 0) {
            op->state = op->state | STATE_TRANSFER;
            cout << "State: Transfer2\n";
            }
        op->packetNo++;
        op->progress = op->progress + recvBytes - 4;
        // ACK
        sndbuf = new char [4];
        makeACK(sndbuf, &op->packetNo);
        cout << "Bytes to write: " << recvBytes - 4 << endl;
        // write HERE:
        of.seekp (0, ios::end);             // Seek to end:
        of.write(op->lastSent, recvBytes - 4);
        // check EOF & listen or set op->state to Complete & exit
        if (recvBytes < DATA_SIZE_MAX) {
            op->state = op->state | STATE_FINISHED;
            cout << "State: Finished\n";
            }
        if (op->packetNo == 65535) {
            cout << "RX; RFC1350 size limit\n";
            op->state = op->state | STATE_ERROR;
            break;
            }
        }
    of.close();
    cout << "Total: " << op->progress << endl;
    cout << "RX; Exiting: " << op->id << endl << endl;
    if ( (op->state ^ STATE_SERVER) >= op->state) {
        exit(0);
        }
    pthread_exit((void*) o);
    }

// return 0 for WRQ or STATE_RECEIVE for RRQ
int processRequest(char * buf, char * fn, bool * ascii){
    /* RRQ/WRQ:    Max size:   524
    2 bytes  | char[255] |1 byte|string 8 byte |1 byte
    ------------------------------------
    | Opcode | Filename | 0 | Mode | 0 |
    ------------------------------------
    
    Opcode:     1: RRQ, 2: WRQ
    Filename:   Chars + '\0'
    Mode:       Case insensitive string (netascii, binary, mail)
    */
    if (buf[1] > 2) {
        return buf[1];
        }
    bool wrq = (1 == (int16_t) buf[1]);
    ascii = new bool;
    
    // get filename
    strncpy (fn, buf + 2, REQUEST_SIZE_MAX - 10);
    char * i;
    i = (char *) memchr(buf+2, 0, REQUEST_SIZE_MAX - 10);
    
    // get mode
    char * mstr = new char [9];
    int x;
    for(int j = 0; j < 7; j++) {
        x = (int)*(i + j);
        *(mstr + j) = tolower(x);
        }
    //strncpy ( mstr, i+1, 9);
    if (strlen(mstr) > 0) {
        *ascii = (strcmp(mstr, (char *) "netascii") == 0);
        if (strcmp(mstr, (char *) "binary") != 0) {
            cout << "Transfer mode: '" << mstr;
            cout << "' has not been implemented for this version!";
            cout << " (only binary please)\n";
            return -1;
            }
        }
    // return 0 if wrq
    return (int) (wrq == false) * STATE_RECEIVE;
    }

/*  Socket Listener thread:
        1. get general settings:
        2. open port for listening:
        3. create transfer thread for each request
*/
void * server(void * ss){
    cout << "Loading server thread\n";
    int s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (s!=0) {
        cout << "FATAL; pthread_setcancelstate" << endl;
        exit(-1);
        }
    // clean ops[]:
    int  i = 0; //for loops
    pthread_mutex_lock(&mutexStats);
    for (i = 0; i < THREADS; i++){
        ops[i].id = -1;
    }
    srvset.rsock = new sockaddr_in;
    pthread_mutex_unlock(&mutexStats);
    // create server socket:
    socklen_t slen=sizeof(*srvset.rsock);    // socket memory size
    char buf[srvset.rbl];                   // initialize request buffer
    /* kernel will figure out what protocol to use if 0 is given 
    instead of IPPROTO_UDP */
    srvset.s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (srvset.s == -1){
        cout << "FATAL; Can not create socket!\n";
        exit(-1);
        }
    // Socket Interface: ME
    srvset.lsock.sin_family = AF_INET;                 // internet
    srvset.lsock.sin_port = htons(srvset.port);        // port
    srvset.lsock.sin_addr.s_addr = htonl(INADDR_ANY);  // listen from any IP
    
    // bind port:
    if (bind(srvset.s, (struct sockaddr *) &srvset.lsock, 
                                            sizeof(srvset.lsock))==-1) {
        cout << "FATAL; bind() failed for port " << srvset.port;
        cout << "!\nAre you root?\n";
        exit(-1);
        }
    int slot = -1;
    // while abort==false:
    while ( srvset.abort == false ){
        // reset buffer:
        memset (buf, '\0', srvset.rbl);
        
        // check srvset for abort flag & set running
        if (srvset.abort != true) {
            pthread_mutex_lock(&mutexSrvset);
            srvset.running = true;
            pthread_mutex_unlock(&mutexSrvset);
            }
        // start listening
        cout << "Ready for new client\n";
        if (recvfrom(srvset.s, buf, srvset.rbl, 0, 
                        (struct sockaddr *) srvset.rsock, &slen)==-1){
            cout << "FATAL; recvfrom() failed!\n";
            exit(-1);
            }
        // understand request:
        char * rfn = new char [REQUEST_SIZE_MAX - 11];     // requested file name
        bool * ascii = new bool;
        ascii = false;
        int lastReq = processRequest(buf, rfn, ascii);
        if (lastReq < 0) {
            cout << "WARNING; unknown request from: ";
            cout << inet_ntoa(srvset.rsock->sin_addr) << ':';
            cout << ntohs(srvset.rsock->sin_port) << endl;
            continue;   // listen next
            }
        cout << "NEW; request from: ";
        cout << inet_ntoa(srvset.rsock->sin_addr) << ':';
        cout << ntohs(srvset.rsock->sin_port) << endl;
        
        // create new op for incomming connection
        counter++;      // hit threadID counter
        operation op;
        op.id = counter;
        op.rsock = *srvset.rsock;    // copy socket
        strncpy((char *)op.rhost, inet_ntoa(srvset.rsock->sin_addr), 256);
        strncpy((char *)op.rfile, rfn, 256);
        op.state = STATE_SERVER | lastReq;
        op.netascii = ascii;
        op.abort = false;
        // find empty slot
        for (i = 0; i < THREADS; i++){
            if (ops[i].id == -1) {
                slot = i;
                break;
                }
            }
        
        // lock mutex Stats & put op into ops
        pthread_mutex_lock(&mutexStats);
        ops[slot] = op;
        pthread_mutex_unlock(&mutexStats);
        
        // create new thread for rx or tx 
        int rc;
        if (lastReq == STATE_RECEIVE){
            rc = pthread_create(&(ops[slot].trd), &attr, &rx, &ops[slot]); 
        } else {
            rc = pthread_create(&(ops[slot].trd), &attr, &tx, &ops[slot]); 
            }
        if (rc){
            cout << "ERROR; pthread_create(): " << rc << endl;
            exit(-1);
            }
        cout << "Thread created!\n";
        }
    // safe exit:
    pthread_mutex_lock(&mutexSrvset);
    srvset.running = false;
    pthread_mutex_unlock(&mutexSrvset);
    
    pthread_exit((void*) ss);
    }

/*  returns slot index for opID (first emty one when -1 given)      
    *This is not in the class as it is called by server function
                                                                    */
int getSlot(int opID){
    for (int i = 0; i < THREADS; i++){
        if (ops[i].id == opID) {
            return i;
            }
        }
        return 1;       // Not found
    }

// TODO: Create a non class member function and call from here instead
char * rxtx::nameLookup(char * rhost){
    hostent * host;     /* host information */
    in_addr h_addr;    /* internet address */
    
    host = gethostbyname(rhost);
    if (host == NULL) {
        return (char *) '\0';      // not found
        }
    h_addr.s_addr = *((unsigned long *) host->h_addr_list[0]);
    return (char *) inet_ntoa(h_addr);
    }

void * transfer(void * opID){
    //cout << "Starting transfer\n";
    // Get the communication:
    operation * a;
    a = & ops[ getSlot( (int) opID) ];
    // Raise error on array fault
    if (a->id < 0) {
        cout << "ERROR: opID cannot be found in slots!\n";
        exit(2);
        }
    // Find out operation by checking state:
    bool sending = ( (a->state ^ 2) < a->state);      // transmit bit set
    
    // load related function:
    if (sending) {
        tx(a);
    } else {
        rx(a);
        }
    pthread_exit((void*) opID);     // Exit
    }

/* Initialize and set thread detached attribute */
rxtx::rxtx(){
    pthread_mutex_init(&mutexStats, NULL);  //  init Stats mutex
    pthread_mutex_init(&mutexSrvset, NULL); //  init Srvset mutex
    pthread_attr_init(&attr);               // init & set thread attr
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    counter = 0;                            // reset counter
    for (int i = 0; i < THREADS; i++){
        ops[i].id = -1;                     // empty thread slots
        }
    }

/* create thread for sending & receiving data as client. Return opID
    @gp: 1: Get, 2:Put
    TODO:   Overload this function to make command line downloader
            (something like: startClient(host, file) )
    
    myNet.startClient(remoteAddress, www, (tkn.at(0) == "put"));
*/
int rxtx::startClient(char * host, char * file, bool send){
    counter++;              // increase counter
    operation o;            // new slot
    o.state = 2 * send;     // 0: receive, 2: send
    strncpy(o.rhost, host, 256);    // remote host address
    strncpy(o.rfile, file, 256);    // treated as local when sending
    o.abort = false;
    o.id = counter;
    
    short int slotIndex = getSlot(-1);
    
    pthread_mutex_lock(&mutexStats);        // lock mutex
    ops[slotIndex] = o;                     // write  new one to ops
    pthread_mutex_unlock(&mutexStats);      // unlock
    
    int rc;
    rc = pthread_create(&(ops[slotIndex].trd), &attr, transfer, 
                                                    (void *)counter); 
    if (rc){
        cout << "ERROR; Client pthread_create(): " << rc << endl;
        exit(-1);
        }
    
    // wait for thread to mark initialization:
    while ((ops[slotIndex].state ^ 16) < ops[slotIndex].state){
        pthread_yield();
        }
    return counter;
    }

/* return operation #opID */
operation * rxtx::op(short int opID){
    for (int i = 0; i < THREADS; i++){
        if (ops[i].id == opID) {
            return &ops[i];
            }
        }
    cout << "Thread " << opID << " cannot be found: " << endl;
    cout << "ops: [ ";
    for (int i = 0; i < THREADS; i++){
        if (ops[i].id != -1){
            cout << ops[i].id << ',';
            }
        }
    cout << "]\n";
    exit(-1);
    }
/*  abort operation #opID   */
int rxtx::abort(short int opID){
    operation * a;
    a = op(opID);
    
    if (a->abort == true) {
        cout << "Already aborted.\n";
    } else {
        pthread_mutex_lock(&mutexStats);
        a->abort = true;                    // set abort flag
        pthread_mutex_unlock(&mutexStats);
        }

    int rc;
    rc = pthread_join(a->trd, &status);     // join, wait for finish
    if (rc) {
            cout << "ERROR; pthread_join(): " << rc << endl;
            exit(-1);
            }
    for (int i = 0; i < THREADS; i++){
        if (ops[i].id == opID) {
            pthread_mutex_lock(&mutexStats);
            ops[i].id = -1;              // remove id
            pthread_mutex_unlock(&mutexStats);
            }
        }
    return rc;
    }
// print summary:
void rxtx::printList(){
    operation a;
    int opcount;
    cout << "Slot\tID\tHost\t\tFile\tTransferred\tTotal\tState\n";
    for (int i = 0; i < THREADS; i++){
        if (ops[i].id == -1) {
            continue;
            }
        opcount++;
        a = ops[i];
        cout << i << '\t' << a.id << '\t' << a.rhost << '\t' << a.rfile;
        cout << '\t' << a.progress << "\t\t" << a.fileSize << '\t';
        cout << a.state << ':' << stateHr(&a.state) << endl;
        }
    }

char * rxtx::stateHr(short int * z){
    /*  # define STATE_RECEIVE      Receive: | Send:      9
        # define STATE_INITIALIZED  Initialized         
        # define STATE_CONNECTED    Connected
        # define STATE_TRANSFER     Transferring        13
        # define STATE_FINISHED     Finished
        # define STATE_ERROR        Error
        */
    char * msgs [] = {"Receive: ", "Send: ", "Initialized ", "Connected ", 
                                "Transferring ", "Finished ", "Error"};
                                
    char * rv = new char [29];
    short int al = 0;
    short int pl = 0;
    int s = *z;
    if ( (s ^ STATE_RECEIVE) >= s) {
        strncpy(rv, msgs[1], 9);   // send
        al = 6;
    } else {
        strncpy(rv, msgs[0], 9);   // receive
        al = 9;
        }
    if ( (s ^ STATE_INITIALIZED) < s) {
        pl = 2;
    } 
    if ( (s ^ STATE_CONNECTED) < s) {
        pl = 3;
    } 
    if ( (s ^ STATE_TRANSFER) < s) {
        pl = 4;
    } 
    if ( (s ^ STATE_FINISHED) < s) {
        pl = 5;
    }
    strncpy(rv+al, msgs[pl], 13);
    
    if ( (s ^ STATE_ERROR) < s) {
        short int el = strlen(msgs[pl]);
        strncpy(rv + al + el, msgs[6], 13);
        }
    return rv;
    }

/* fire up the server thread. Return 0 if successful                */
int rxtx::startServer(int port){
    srvset.abort = false;
    srvset.rbl = REQUEST_SIZE_MAX;
    srvset.port = port;
    srvset.gTimeout = 7000; // timeout
    srvset.pTimeout = 2000; // rexmt
    int rc;
    rc = pthread_create(&(srvset.trd), &attr, server, (void *)counter);
    if (rc){
        cout << "ERROR; Server pthread_create(): " << rc << endl;
        exit(-1);
        }
    while (srvset.running != true){
        pthread_yield();
        }
    return 0;
    }


/* fill in array for active ops and return pointer                  */
void rxtx::getOps(int * rv, int & rvp){
    memset(rv, 0, sizeof (int) * THREADS);
    rvp = 0;
    for (int i = 0; i < THREADS; i++){
        if (ops[i].id == -1) {
            continue;
            }
        rv[rvp] = ops[i].id;
        rvp++;
        }
    }
