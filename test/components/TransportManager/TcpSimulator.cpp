// TcpSimulator.cpp
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <getopt.h>
#include <fstream>


// -------------------------------------------------------------------------

/**
 * @brief Confituration of the application
 */
struct Config
{
    std::string ipAddress;
    int port;
    int protocolVersion;
    uint32_t actualDataSize;
    uint32_t reportedDataSize;
    std::string fileName;
};



/**
 * 
 * @brief The common part of the header for both protocol versions 
 */
struct PacketHeaderBase
{
    uint8_t frameType:3;
    bool compressionFlag:1;
    uint8_t version:4;
    uint8_t serviceType;
    uint8_t frameData; 
    uint8_t sessionId;
    uint32_t dataSize;
};

/**
 * @brief The header of the protocol version 1
 */
struct PacketHeaderV1
{
    PacketHeaderBase base;
    
    bool isValidVersion() const { return (base.version == 0x01); }

    // no additional fields
};

/**
 * @brief The header of the protocol version 2
 */
struct PacketHeaderV2
{
    PacketHeaderBase base;

    uint32_t messageId;
    
    bool isValidVersion() const { return (base.version == 0x02); }
};

// -------------------------------------------------------------------------
/**
 * @brief Default config values
 */
const Config DefaultConfigValues = 
{
    std::string("127.0.0.1"),       // Default IP address
    12345,                          // Default TCP port
    2,                              // Default protocol version
    32,                             // Default actual data size
    32,                             // Default reported data size
    std::string("")                              // Default file name (no default)
};

// -------------------------------------------------------------------------





/**
 * @brief The class incapsulates connection routines to the TCP socket of the Transport Manager
 */
class CTranspMgrTcpClient
{
public:
 
    /**
     * @brief CTranspMgrTcpClient receives all necesary parameters via this constructor 
     * 
     * @param ipAddress The IP address of the host where Transport Manager is running
     * @param port The TCP port of Transport Manager
     */
    CTranspMgrTcpClient(const std::string ipAddress, const int port);
    
    /**
     * @brief Destructor closes the socket if necessary
     */
    ~CTranspMgrTcpClient();
 
    /**
     * @brief Tries to establish TCP connection to the Transport Manager
     */
    void connect();
    
    /**
     * @brief Closes the socket (if it was opened before)
     */
    void disconnect();
    
    /**
     *  @brief Sends data packet to the Transport Manager
     *
     *  @brief pData Pointer to the data to send
     *  @brief dataSize Size of the buffer to send 
     */
    void send(const void *pData, size_t dataSize);
    
    /**
     * @brief   check if socket connected
     * @return  if socket conncted, TRUE; otherwise FALSE
     */
    inline bool isConnected() { return mIsConencted; }
        
private:
    int mPort;
    std::string mIpAddress;
    bool mIsConencted;
    
    int mSocketFd;                    // socket file descriptor
};

// ----------------------------------------------------------------------------

CTranspMgrTcpClient::CTranspMgrTcpClient(const std::string ipAddress, const int port) 
    : mPort(port)
    , mIpAddress(ipAddress)
    , mIsConencted(false)
    , mSocketFd(-1)
{   
}

// ----------------------------------------------------------------------------

CTranspMgrTcpClient::~CTranspMgrTcpClient()
{
    if (mSocketFd != -1)
    {
        close(mSocketFd);
    }
}

// ----------------------------------------------------------------------------

void CTranspMgrTcpClient::connect()
{    
    struct hostent *server;
    struct sockaddr_in serverAddr;
    
    mSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (mSocketFd < 0) 
    {
        throw new std::string("Error opening socket\n");
    } 
    
    server = gethostbyname(mIpAddress.c_str());
    if (server == NULL) 
    {
        throw new std::string("Error, no such host\n");
    }  
    
    bzero( (char*) &serverAddr, sizeof(serverAddr) );
    serverAddr.sin_family = AF_INET;   
    bcopy((char*)server->h_addr, (char*)&serverAddr.sin_addr.s_addr, server->h_length);
    serverAddr.sin_port = htons(mPort);
    
    if (::connect(mSocketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        throw new std::string("Error connecting\n");
    }
    mIsConencted = true;
}

// ----------------------------------------------------------------------------

void CTranspMgrTcpClient::disconnect()
{
    close(mSocketFd);
    
    mSocketFd = -1;
}

// ----------------------------------------------------------------------------

void CTranspMgrTcpClient::send(const void* pData, size_t dataSize)
{
    if (pData != NULL)
    {
        ssize_t written = write(mSocketFd, pData, dataSize);
        
        if (written == -1)
        {
            throw new std::string("Error writing into the socket");
        }
        if (written != dataSize)
        {
            std::cout << "Warning: expected to send " << dataSize << 
                " bytes, but sent " << written << " bytes\n";
        }
    }
}


/**
 * @brief   class providing paylod data for testing of appLinkCore via TCP(WiFi) link
 * 
 */
class CAppTester
{
private:    
   PacketHeaderV1* mPacketheaderV1;
   PacketHeaderV2* mPacketheaderV2;
   CTranspMgrTcpClient& mTCPClient;
    
    
public:    
    /**
     * Default constructor
     */
    CAppTester(PacketHeaderV2 *hV2, PacketHeaderV1 *hV1, CTranspMgrTcpClient& tcplCient)
        : mPacketheaderV1(hV1)
        , mPacketheaderV2(hV2)
        , mTCPClient(tcplCient)
    {}
    
     /**
        * \brief Sends data from file to appLinkCore 
        * \param fileName name of file with json messages
        * \return Error code (-1 in case of any errors)
        */
     // -------------------------------------------------------------------------
    int sendDataFromFile(const char * fileName)
    // -------------------------------------------------------------------------
    {
        using namespace std;
        printf("%s:%d CAppTester::sendDataFromFile()\n", __FILE__, __LINE__);

        ifstream file_str;
        file_str.open (fileName);

        if (!file_str.is_open())
        {
            printf("File wasn't opened!\n");
            return -1;
        }

        int startpos = 0;
        file_str.seekg(0, ios::end);
        int length = file_str.tellg();
        file_str.seekg(0, ios::beg);
        printf("length = %d \n", length);
        while (startpos < length)
        {
            char * raw_data = new char[length];
            file_str.getline(raw_data, length);
            printf("strlen = %ld \n", strlen(raw_data));
            startpos += strlen(raw_data)+1;
            std::string instr = std::string(raw_data, strlen(raw_data));
            delete[] raw_data;
            printf("Input string:\n %s\n", instr.c_str());
            
            void* packet2Send = 0;
            int32_t packet2SendLength = 0;
            packet2SendLength = generateSingleMessage(mPacketheaderV2, mPacketheaderV1, instr, packet2Send);
            sendData(packet2Send, packet2SendLength);
            
            printf("packet2SendLength = %d \n", packet2SendLength);
            printf("packet2Send = %p \n", packet2Send);
        }
        file_str.close();
        return 0;
    }
 
 
private:
    
    /**
     * @brief   send data (over tcp)
     * 
     * @param   data    data to send
     * @param   length  data length
     * 
     */
    // -------------------------------------------------------------------------
    void sendData(const void *const data, const int length)
    // -------------------------------------------------------------------------
    {
        if ((length > 0) && data != 0)
        {
            if (mTCPClient.isConnected() == false)
            {
                mTCPClient.connect();
            }
            mTCPClient.send(data, length);
        }
        else 
        {
            printf("\n CAppTester::sendData error: no data to send\n");
        }
    }

private:    
    /**
     * @brief   generate single message (i.e. frameType will be forcible set to 0x1 Single Frame in PacketHeader)
     * 
     *          If both parameters are not 0, mesage will be generated for first parameter hv2
     * @note    returned packet should be freed by invoker
     * 
     * @param   hv2         packet header pf protocol version 2. If value is 0, ignored
     * @param   hv1         packet header pf protocol version 1. If value is 0, ignored
     * @param   payload     payload data
     * @param   resultData  generated data
     * @return  length of generated packet 
     */
    // -------------------------------------------------------------------------
    int32_t generateSingleMessage(const PacketHeaderV2 *hV2, const PacketHeaderV1 *hV1, const std::string payload, void *& resultData)
    // -------------------------------------------------------------------------
    {
        if (hV2 == 0 && hV1 == 0)
        {
            printf("\n CAppTester::generateSingleMessage() error: null input parameters");
            return 0;
        }
        
        uint8_t protocolVersion = (hV2 != 0 ? 0x02 : 0x01);
        if ((protocolVersion == 0x02) && (hV2->isValidVersion() == false))
        {
            printf("\n generateSingleMessage() error. Incorrect version of PacketHeaderV2");
            return 0;
        }
        if ((protocolVersion == 0x01) && (hV1->isValidVersion() == false))
        {
            printf("\n generateSingleMessage() error. Incorrect version of PacketHeaderV1");
            return 0;
        }        
        
        PacketHeaderBase phb = ( (hV2 !=0 ) ? hV2->base : hV1->base );
        
        uint32_t headerSize = 8;
        if (protocolVersion == 0x02)
            headerSize = 12;

        uint8_t sVersion        = protocolVersion;
        uint8_t sCompressedFlag = phb.compressionFlag;
        uint8_t sFrameType      = 0x01; //Single is set forcible over phb.frameType for this method
        uint8_t sServiceType    = phb.serviceType;
        uint8_t sFrameData      = phb.frameData; 
        uint8_t sSessionID      = phb.sessionId;
        uint32_t sDataSize      = payload.length();//' + 1; //
        uint32_t sMessageID     = (protocolVersion == 0x02 ? hV2->messageId : 12345);

        void* sPacketData = malloc(headerSize + sDataSize);

        uint8_t firstByte = ( (sVersion << 4) & 0xF0 )
                        | ( (sCompressedFlag << 3) & 0x08)
                        | (sFrameType & 0x07);

        uint32_t offset = 0;
        memcpy((uint8_t*)sPacketData + offset++, &firstByte, 1);
        memcpy((uint8_t*)sPacketData + offset++, &sServiceType, 1);
        memcpy((uint8_t*)sPacketData + offset++, &sFrameData, 1);
        memcpy((uint8_t*)sPacketData + offset++, &sSessionID, 1);

        uint8_t tmp = sDataSize >> 24;
        memcpy((uint8_t*)sPacketData + offset++, &tmp, 1);
        tmp = sDataSize >> 16;
        memcpy((uint8_t*)sPacketData + offset++, &tmp, 1);
        tmp = sDataSize >> 8;
        memcpy((uint8_t*)sPacketData + offset++, &tmp, 1);
        tmp = sDataSize;
        memcpy((uint8_t*)sPacketData + offset++, &tmp, 1);


        if (protocolVersion == 0x02)
        {
            uint8_t tmp1 = sMessageID >> 24;
            memcpy((uint8_t*)sPacketData + offset++, &tmp1, 1);
            tmp1 = sMessageID >> 16;
            memcpy((uint8_t*)sPacketData + offset++, &tmp1, 1);
            tmp1 = sMessageID >> 8;
            memcpy((uint8_t*)sPacketData + offset++, &tmp1, 1);
            tmp1 = sMessageID;
            memcpy((uint8_t*)sPacketData + offset++, &tmp1, 1);
        }

        memcpy((uint8_t*)sPacketData + offset, (void*)const_cast<char*>(payload.c_str()), sDataSize);

        resultData = sPacketData;
        printf("SINGLE MESSAGE GENERATED!\n");
        
        return (headerSize + sDataSize);
    }
};


// -------------------------------------------------------------------------

static void printUsage(const std::string &programName)
{
    std::cout << "Usage: " << programName << " [OPTION]" << std::endl;
    std::cout << "Send test packet to the TransportManager in order to test TCP Adapter" << std::endl;
    std::cout << std::endl;
    std::cout << "-i, --ip IP_ADDRESS           IP address where to send the packet (Default: " 
        << DefaultConfigValues.ipAddress << ")" << std::endl;

    std::cout << "-p, --port PORT_NUMBER        TCP port number (Default: " 
        << DefaultConfigValues.port << ")" << std::endl;

    std::cout << "-v, --version [1 | 2]         Protocol version. (Default: " 
        << DefaultConfigValues.protocolVersion << ")" << std::endl;

    std::cout << "-a, --actual-size SIZE        The real size of the data in the packet to send (Default: "
        << DefaultConfigValues.actualDataSize << ")" << std::endl;
    
    std::cout << "-r, --reported-size SIZE      The size of the packet which is written in the packet header (Default: " 
        << DefaultConfigValues.reportedDataSize << ")" << std::endl;
        
    std::cout << "-f, --file FILE                The name of the file whose content to be sent over TCP (if option is set '--actual-size' and '--reported-size' are ignored)" 
        << std::endl;
        

    std::cout << "-h, --help                    Print this help" << std::endl << std::endl;
}

// -------------------------------------------------------------------------

static bool initConfig(int argc, char **argv, Config *pConfig)
{
    bool result = true;
   
    struct option long_options[] =
    {
        {"help", no_argument, 0, 'h'},
        {"ip", required_argument, 0, 'i'},
        {"port", required_argument, 0, 'p'},
        {"version", required_argument, 0, 'v'},
        {"actual-size", required_argument, 0, 'a'}, 
        {"reported-size", required_argument, 0, 'r'},
        {"file", required_argument, 0, 'f'},
        {0, 0, 0, 0}
    };

    *pConfig = DefaultConfigValues;

    while (true)
    {
        int option_index = 0;
        int c;
    
        c = getopt_long(argc, argv, "hi:p:v:a:r:f:", long_options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
            case 'i':
                if (optarg)
                {
                    // Todo: parse this string for valid IP address
                    pConfig->ipAddress = optarg;
                }
                else
                {
                    std::cout << "Error: IP address is missing" << std::endl;
                    result = false;
                }
                break;

            case 'p':
                if (optarg)
                {
                    pConfig->port = atoi(optarg);
                }
                else 
                {
                    std::cout << "Error: port is missing" << std::endl;
                    result = false;
                }
                break;
 
            case 'v':
                if (optarg)
                {
                    pConfig->protocolVersion = atoi(optarg);
                }
                else 
                {
                    std::cout << "Default protocol version is" << DefaultConfigValues.protocolVersion << std::endl;
                }
                break;
 
            case 'a':
                if (optarg)
                {
                    pConfig->actualDataSize = atoi(optarg);
                }
                else 
                {
                    std::cout << "Error: data size is missing" << std::endl;
                    result = false;
                }
                break;
 
            case 'r':
                if (optarg)
                {
                    pConfig->reportedDataSize = atoi(optarg);
                }
                else 
                {
                    std::cout << "Error: data size is missing" << std::endl;
                    result = false;
                }
                break;
            
            case 'f':
                if (optarg)
                {
                    pConfig->fileName = optarg;
                }
                else
                {
                    std::cout << "Error: file name is missing" << std::endl;
                    result = false;
                }
                break;

            default:
                result = false;
        }
    }
    
    if (result == false)
    {
        printUsage(argv[0]);
    }

    return result;
}

// -------------------------------------------------------------------------

static uint32_t uint32ToNetOrder(const uint32_t value)
{
    uint32_t result = 0;

    result |= (value & 0x000000FF) << 24;
    result |= (value & 0x0000FF00) << 8;
    result |= (value & 0x00FF0000) >> 8;
    result |= (value & 0xFF000000) >> 24;

    return result;
}

// -------------------------------------------------------------------------

static uint8_t* makePacket(const Config &config, int &packetSize)
{
    uint8_t *pBuff = 0;
    PacketHeaderBase *pBase;
    size_t headerSize;

    if (config.protocolVersion == 1)
    {
        headerSize = sizeof(PacketHeaderV1);
    }
    else if (config.protocolVersion == 2)
    {
        headerSize = sizeof(PacketHeaderV2);
    }
    else
    {
        throw new std::string("Unsupported protocol version");
    }
    
    packetSize = headerSize + config.actualDataSize;

    pBuff = new uint8_t[packetSize];
    pBase = (PacketHeaderBase*)pBuff;

    pBase->version = config.protocolVersion;
    pBase->compressionFlag = false;
    pBase->frameType = 1;
    pBase->serviceType = 0x0F;
    pBase->frameData = 0;
    pBase->sessionId = 0;
    pBase->dataSize = uint32ToNetOrder(config.reportedDataSize);           // convert data size to network order 
 
    if (config.protocolVersion == 2)
    {
        PacketHeaderV2 *pHeader = (PacketHeaderV2*)pBuff;

        pHeader->messageId = 0;
    }
    
    // just fill the data with the single value
    memset(&(pBuff[headerSize]), 0xFF, config.actualDataSize);

    return pBuff;
}


static void makePacketHeader(size_t version, PacketHeaderV1 *& phv1, PacketHeaderV2 *& phv2)
{
    uint8_t *pBuff = 0;
    PacketHeaderBase *pBase;
    size_t headerSize;
    
    phv1 = 0; phv2 = 0;

    if (version == 1)
    {
        headerSize = sizeof(PacketHeaderV1);
    }
    else if (version == 2)
    {
        headerSize = sizeof(PacketHeaderV2);
    }
    else
    {
        throw new std::string("Unsupported protocol version");
    }    

    pBuff = new uint8_t[headerSize];
    pBase = (PacketHeaderBase*)pBuff;

    pBase->version = version;
    pBase->compressionFlag = false;
    pBase->frameType = 1;
    pBase->serviceType = 0x0F;
    pBase->frameData = 0;
    pBase->sessionId = 0;
    //pBase->dataSize = uint32ToNetOrder(config.reportedDataSize);           // convert data size to network order 
 
    if (version == 2)
    {
        PacketHeaderV2 *pHeader = (PacketHeaderV2*)pBuff;

        pHeader->messageId = 0;
        
        phv2 = pHeader;
    }
    else
    {
        phv1 = (PacketHeaderV1*)pBuff;
    }
   
}

// ----------------------------------------------------------------------------

int main(int argc, char **argv)
{
    Config config;
    uint8_t *pBuff = 0;
    int buffSize;

    if (initConfig(argc, argv, &config) == false)
    {
        return 0;  // can't continue, do not have enough configuration data
    }

    CTranspMgrTcpClient client(config.ipAddress, config.port);
   
    if (config.fileName.empty())
    {
        try
        {
            pBuff = makePacket(config, /*out*/buffSize);
            
            client.connect();

            client.send(pBuff, buffSize);    

            std::cout << "The packet has been sent successfully" << std::endl;
        }
        catch (std::string *pError)
        {
            std::cout << *pError << std::endl;
        }
        delete pBuff;
    }
    else 
    {
        PacketHeaderV1* phv1;
        PacketHeaderV2* phv2;
        
        makePacketHeader(config.protocolVersion, phv1, phv2);
        
        CAppTester appTester(phv2, phv1, client);
        appTester.sendDataFromFile(config.fileName.c_str());
    }
    
        

    client.disconnect();
 
    return 0;
}


