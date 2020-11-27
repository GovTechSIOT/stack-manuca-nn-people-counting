#ifndef MQTTNETWORK_H
#define MQTTNETWORK_H

#include "NetworkInterface.h"

#undef USE_TLS
#if defined(MBED_CONF_APP_USE_TLS) && (MBED_CONF_APP_USE_TLS == 1)
  #define USE_TLS
#endif

#ifdef USE_TLS
#include "TLSSocket.h"
#else
#include "TCPSocket.h"
#endif  // USE_TLS

class MQTTNetwork {
public:
    MQTTNetwork(NetworkInterface* aNetwork) : network(aNetwork) {
#ifdef USE_TLS
        socket = new TLSSocket();
#else
        socket = new TCPSocket();
#endif  // USE_TLS
    }

    ~MQTTNetwork() {
        delete socket;
    }

    int read(unsigned char* buffer, int len, int timeout) {
        return socket->recv(buffer, len);
    }

    int write(unsigned char* buffer, int len, int timeout) {
        return socket->send(buffer, len);
    }

    int connect(const char* hostname, int port, const char *ssl_ca_pem = NULL,
            const char *ssl_cli_pem = NULL, const char *ssl_pk_pem = NULL) {
        nsapi_error_t ret = 0;
        if ((ret = socket->open(network)) != 0) {
            return ret;
        }
#ifdef USE_TLS
        socket->set_root_ca_cert(ssl_ca_pem);
        socket->set_client_cert_key(ssl_cli_pem, ssl_pk_pem);
#endif  // USE_TLS
        return socket->connect(hostname, port);
    }

    int disconnect() {
        return socket->close();
    }

private:
    NetworkInterface* network;
#ifdef USE_TLS
    TLSSocket* socket;
#else
    TCPSocket* socket;
#endif  // USE_TLS
};

#endif  // MQTTNETWORK_H
