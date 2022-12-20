#include <unistd.h>
#include <fcntl.h>
#include <wait.h>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <mqueue.h>
#include <string.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <pthread.h>
#include <omp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <string>




int set_non_block(int fd){
    int flags;
#if defined O_NONBLOCK
    if(-1 == (flags= fcntl(fd, F_GETFL, 0))){
        flags = 0;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    flags = -1;
    return ioctl(fd, FIOBIO, &flags);
#endif
}

struct SocketData{
    int socketfd;
    std::string WorkingDir;
};
std::string WORKING_DIRECTORY;

int parse_http(std::string* message, std::string* filePath){
    size_t start_of_path = message->find_first_of(' ') + 1;
    // char* coc[2];
    // *coc[0] = ' ';
    // *coc[1] = '?';
    size_t end_of_path = message->find_first_of(" ?", start_of_path);
    *filePath = message->substr(start_of_path, end_of_path - start_of_path);
    //std::string* pFilePath = (std::string*)malloc(filePath.size());
    //*pFilePath = filePath.substr(0);
    //std::cout << "File path = " << *filePath << std::endl;
    return 0;
}

int create_response(std::string* fullPath,std::string* response)
{
    //FILE* fd = fopen(fullPath[0].c_str(), "r");
    std::fstream input;
    //std::cout << "full path = " << *fullPath << std::endl;
    input.open(*fullPath, std::fstream::in | std::fstream::out);
    std::stringstream ssBuffer;
    ssBuffer << input.rdbuf();
    std::string content = ssBuffer.str();

    //contLen = 
    if(input.is_open()){
        //200
        //std::stringstream buf;
        //buf <<"HTTP/1.0 200 OK\r\n"<<"Content-length: "<< 
        response->append("HTTP/1.0 200 OK\r\n");
        response->append("Content-length: ");
        response->append(std::to_string(content.size()));
        response->append("\r\n");
        response->append("Content-Type: text/html\r\n\r\n");
        response->append(content);
        response->append("\r\n\r\n");

    }
    else{
        //404
        response->append("HTTP/1.0 404 NOT FOUND\r\n") ;
        response->append("Content-Length: 0\r\n") ;
        response->append("Content-Type: text/html\r\n\r\n");
        //response->append() "\r\n\r\n";
    }
    input.close();
    return 0;
}

void send_response(SocketData *sd, std::string* response)
{
    
    char *cstr = new char[response->length() + 1];
    strcpy(cstr, response->c_str());
    // do stuff
    send(sd->socketfd, cstr, response->size(), 0);

    delete [] cstr;
}

void* slavesocket_work(void* pSocketData)
{
    //int* coc = (int*)slavesocket;
    SocketData socketData = *(SocketData*)pSocketData;
    std::vector<char> buffer(10000);
    std::string rcv;
    int received;
    do
    {
        received = recv(socketData.socketfd, &buffer[0], buffer.size(), 0);
        if(received == -1)
        {
            //std::cout << "0 bytes received (error)" << std::endl;
        }
        else
        {
            rcv.append(buffer.cbegin(), buffer.cend());
        }

    } while(received == buffer.size());
    //std::cout << "Received message:\r\n" << rcv << std::endl;
    std::string filePath;
    parse_http(&rcv, &filePath);
    std::string response;
    std::stringstream sfullPath;
    //sfullPath << socketData.WorkingDir;
    sfullPath << WORKING_DIRECTORY;
    sfullPath << filePath;
    std::string fullPath = sfullPath.str();
    create_response(&fullPath, &response);
    //std::cout << "Response is = \r\n" << response << std::endl;
    send_response(&socketData, &response);
    return NULL;
}

void do_daemon_things(){
    pid_t pid, sid;

    /* Fork off the parent process */       
    pid = fork();
    if (pid < 0) 
    {
        exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then
        we can exit the parent process. */
    if (pid > 0) 
    {
        exit(EXIT_SUCCESS);
    }
    /* Change the file mode mask */
    umask(0);

    /* Open any logs here */
        
    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) 
    {
        /* Log any failure */
        exit(EXIT_FAILURE);
    }

    /* Change the current working directory */
    if ((chdir("/")) < 0) 
    {
        /* Log any failure here */
        exit(EXIT_FAILURE);
    }

    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

int main(int argc, char** argv)
{
    int opt;
    std::string raw_host;
    std::string raw_port;
    std::string directory; 
    
    //std::cout << "niec!" << std::endl;
    while((opt = getopt(argc, argv, "h:p:d:")) != -1){
        switch(opt){
            case 'h':
            raw_host = optarg;
            //std::cout << "host Saved!"<< std::endl;
            break;

            case 'p':
            //std::cout << "port saved" << std::endl;
            raw_port = optarg;
            break;

            case 'd':
            //std::cout << "directory saved!" << std::endl;
            directory = optarg;
            WORKING_DIRECTORY = optarg;
            break;
        }
    }
    //do_daemon_things();
    daemon(1,0);
    int MasterSocket = socket(AF_INET, SOCK_STREAM, 0);
    int port = atoi(&raw_port[0]);
    //char host[sizeof(struct in_addr)];
    in_addr host;
    inet_pton(AF_INET, &raw_host[0], &host);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr = host;

    socklen_t socklen = sizeof(sockaddr_in);
    

    bind(MasterSocket, (struct sockaddr*)(&addr), sizeof(addr));
    
    set_non_block(MasterSocket);
    listen(MasterSocket, SOMAXCONN);

    while(1){
        int SlaveSocket = accept(MasterSocket, (struct sockaddr*)(&addr), &socklen);
        if(SlaveSocket == -1)
        {
            //std::cout << "Slave == -1" << std::endl;
        }
        else
        {
            //std::cout << "Connection POG!" << std::endl;
            pthread_t slavethread;
            //pthread_attr_t slavethread_attr;
            SocketData sd;
            sd.socketfd = SlaveSocket;
            sd.WorkingDir = directory;
            int pthreadCheck = pthread_create(&slavethread, NULL, slavesocket_work, &sd);
            if(pthreadCheck != 0)
            {
                //std::cout << "Something wrong with pthread creation" << std::endl;
            }
            int pthrDet = pthread_detach(slavethread);
            if(pthrDet != 0)
            {
                //std::cout << "Something wrong with pthread detaching" << std::endl;
            }
        }
    }
}