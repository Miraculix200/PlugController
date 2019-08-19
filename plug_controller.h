#ifndef PLUGCTRL_H
#define PLUGCTRL_H

class PlugController
{

    private:
        std::string targetIP;
        uint16_t targetPort;
        static void serializeUint32(char (&buf)[4], uint32_t val);
        static void encrypt(char *data, uint16_t length);
        static void encryptWithHeader(char *out, char *data, uint16_t length);
        static void decrypt(char* input, uint16_t length);
        static uint16_t sockConnect(char* out, const char *ip_add, int port, const char *cmd, uint16_t length);

    public:
        PlugController();
        PlugController(std::string ip, uint16_t port);
        std::string sendCmd(std::string cmd);
        std::string on();
        std::string off();
        std::string getEmeter();
        std::string getInfo();
        std::string eraseEmeterStats();
        std::string setLedOff();
        std::string setLedOn();
        std::string countDown(uint16_t seconds, bool act);
};

#endif
