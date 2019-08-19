/* ########################################################################### */
/* ########################################################################### */

#include <arpa/inet.h>  // for inet_pton
#include <netinet/in.h> // for sockaddr_in, htons
#include <stdint.h>     // for uint16_t, uint8_t, uint32_t
#include <stdio.h>      // for printf
#include <sys/socket.h> // for AF_INET, connect, send, socket, SOCK_STREAM
#include <unistd.h>     // for close, read
#include <string>       // for string
#include <cstring>      // for ??? memcpy, memset, strncpy

#include "plug_controller.h"

/* ########################################################################### */
/* ########################################################################### */

//Default Constructor

PlugController::PlugController(){};

PlugController::PlugController(std::string ip, uint16_t port)
{
    targetIP = ip;
    targetPort = port;
}

/* ########################################################################### */

void PlugController::serializeUint32(char (&buf)[4], uint32_t val)
{
    buf[0] = (val >> 24) & 0xff;
    buf[1] = (val >> 16) & 0xff;
    buf[2] = (val >> 8) & 0xff;
    buf[3] = val & 0xff;
}

/* ########################################################################### */

void PlugController::decrypt(char *input, uint16_t length)
{
    uint8_t key = 171;
    uint8_t next_key;
    for (uint16_t i = 0; i < length; i++)
    {
        next_key = input[i];
        input[i] = key ^ input[i];
        key = next_key;
    }
}

/* ########################################################################### */

void PlugController::encrypt(char *data, uint16_t length)
{
    uint8_t key = 171;
    for (uint16_t i = 0; i < length + 1; i++)
    {
        data[i] = key ^ data[i];
        key = data[i];
    }
}

/* ########################################################################### */

void PlugController::encryptWithHeader(char *out, char *data, uint16_t length)
{
    char serialized[4];
    serializeUint32(serialized, length);
    encrypt(data, length);
    std::memcpy(out, &serialized, 4);
    std::memcpy(out + 4, data, length);
}

/* ########################################################################### */

std::string PlugController::getInfo()
{
    const std::string cmd = "{\"system\":{\"get_sysinfo\":{}}}";
    return sendCmd(cmd);
}

/* ########################################################################### */

std::string PlugController::on()
{
    const std::string cmd = "{\"system\":{\"set_relay_state\":{\"state\":1}}}";
    return sendCmd(cmd);
}

/* ########################################################################### */

std::string PlugController::off()
{
    const std::string cmd = "{\"system\":{\"set_relay_state\":{\"state\":0}}}";
    return sendCmd(cmd);
}

/* ########################################################################### */

std::string PlugController::eraseEmeterStats()
{
    const std::string cmd = "{\"emeter\":{\"erase_emeter_stat\":null}}";
    return sendCmd(cmd);
}

/* ########################################################################### */

std::string PlugController::countDown(uint16_t seconds, bool act)
{
    std::string cmd = "{\"count_down\":{\"add_rule\":{\"enable\":1,\"delay\":";
    cmd += std::to_string(seconds);
    cmd += ",\"act\":";
    if (act)
        cmd += "1,\"name\":\"turn on\"}}}";
    else
        cmd += "0,\"name\":\"turn off\"}}}";

    const std::string delete_all_rules = "{\"count_down\":{\"delete_all_rules\":null}}";
    sendCmd(delete_all_rules);

    return sendCmd(cmd);
}

/* ########################################################################### */

std::string PlugController::getEmeter()
{
    const std::string cmd = "{\"emeter\":{\"get_realtime\":{}}}";
    return sendCmd(cmd);
}

/* ########################################################################### */

std::string PlugController::setLedOff()
{
    const std::string cmd = "{\"system\":{\"set_led_off\":{\"off\":1}}}";
    return sendCmd(cmd);
}

/* ########################################################################### */

std::string PlugController::setLedOn()
{
    const std::string cmd = "{\"system\":{\"set_led_off\":{\"off\":0}}}";
    return sendCmd(cmd);
}

/* ########################################################################### */

std::string PlugController::sendCmd(std::string cmd)
{
    char encrypted[cmd.length() + 4];
    encryptWithHeader(encrypted, const_cast<char *>(cmd.c_str()), cmd.length());
    char response[2048] = {0};
    uint16_t length = sockConnect(response, this->targetIP.c_str(), this->targetPort, encrypted, cmd.length() + 4);
    if (length > 0)
        decrypt(response, length - 4);
    else
        return std::string("");
    return std::string(response);
}

/* ########################################################################### */

uint16_t PlugController::sockConnect(char *out, const char *ip_add, int port, const char *cmd, uint16_t length)
{

    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buf[2048] = {0};
    //  char buffer[2048] = {0};
    //    char buffer[2048] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return 0;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, ip_add, &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return 0;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return 0;
    }
    send(sock, cmd, length, 0);

    valread = read(sock, buf, 2048);
    close(sock);

    if (valread == 0)
    {
        printf("\nNo bytes read\n");
    }
    else
    {
        // buf + 4 strips 4 byte header
        // valread - 3 leaves 1 byte for terminating null character
        strncpy(out, buf + 4, valread - 3);
    }

    return valread;
}

/* ########################################################################### */
/* ########################################################################### */