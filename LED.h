#ifndef LED_H
#define LED_H

#include <string>

class LED {
private:
    std::string nomeArquivo;

public:
    LED(const std::string& arquivo = "led.dat");

    void adicionar(int rrn);

    bool obter(int &rrn);

    bool vazia();
};

#endif
