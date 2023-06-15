#include "hsocket.hpp"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "hcore.hpp"
#include "OScommon.hpp"




namespace OS{

    static int socket_server = -1;
    static int socket_client = -1;


    bool socket_connected = false;


    void socket_set_core(HCore::HCore *core){
        //m_core = core;
    }

    void socket_init(SocketMode sm){
        
    }

    void socket_close() {
        close(socket_client);
    }

    static void error(const char *s) {
        //LOG(LOG_ERROR,"%s failed (%d): %s\n", s, errno, strerror(errno));
    }

    int socket_connect(const char *server, unsigned short port) {
        struct sockaddr_in sock_desc;
        struct hostent     *host_desc;

        if (server == NULL) return -2;

        host_desc = gethostbyname(server);
        if (host_desc == 0) {
            m_core->log(LOG_ERROR,"Hostname resolution");
            return -2;
        }

        socket_client = socket(AF_INET,SOCK_STREAM,0);
        if (socket_client < 0) {
            m_core->log(LOG_ERROR,"Socket creation");
            return -1;
        }

        sock_desc.sin_family = AF_INET;

        memcpy(&sock_desc.sin_addr,
        host_desc->h_addr,
        host_desc->h_length);
        sock_desc.sin_port = htons(port);

        if (connect(socket_client,(struct sockaddr *)&sock_desc,sizeof(sock_desc)) < 0) {
            m_core->log(LOG_ERROR,"Connect");
            close(socket_client);
            socket_client = -1;
            return -3;
        }

        signal(SIGPIPE, SIG_IGN);

        socket_connected = true;

        return socket_client;
    }

    int socket_listen(unsigned short port) {
        struct sockaddr_in sock_desc;
        int v;

        socket_server = socket(AF_INET,SOCK_STREAM,0);
        if (socket_server < 0) {
            m_core->log(LOG_ERROR,"Socket creation");
            return -1;
        }
        sock_desc.sin_family      = AF_INET; 
        sock_desc.sin_addr.s_addr = htonl(INADDR_ANY);
        sock_desc.sin_port        = htons(port);
        if (bind(socket_server,(struct sockaddr *)&sock_desc,sizeof(sock_desc)) < 0) {
            m_core->log(LOG_ERROR,"Bind");
            close(socket_server);
            socket_server = -1;
            return -2;
        }
        v = 1;
        setsockopt(socket_server,SOL_SOCKET, SO_REUSEADDR,&v, sizeof(v));
        listen(socket_server,1);

        return socket_server;
    }

    int socket_accept() {
        struct sockaddr_in sock_desc;
        socklen_t length = sizeof(sock_desc);

        socket_client = accept(socket_server,(struct sockaddr *)&sock_desc,&length);
        if (socket_client < 0) {
            m_core->log(LOG_ERROR,"Accept");
            return -1;
        }

        //printf("Connection from: %s\n", inet_ntoa(sock_desc.sin_addr));
        
        signal(SIGPIPE, SIG_IGN);

        return socket_client;
    }

    int socket_receive(void *data, unsigned int size) {
        int n;
        unsigned char *dst = (unsigned char *)data;
        //LOG(//LOG_DEBUG,"Socket receiving %d\n");
        while(size) {
            n = read(socket_client, dst, size);
            if (n<0) {
                m_core->log(LOG_ERROR,"Receive");
                return n;
            }
            size -= n;
            dst += n;
        }
        return size;
    }

    int socket_send(void *data, unsigned int size) {
        int n;
        unsigned char *dst = (unsigned char *)data;
        //LOG(//LOG_DEBUG,"Socket sending %d\n");
        while(size) {
            n = write(socket_client, dst, size);
            if (n<0) {
                m_core->log(LOG_ERROR,"Send");
                return n;
            }
            size -= n;
            dst += n;
        }
        return size;
    }

}