#include "LED.h"

#include <fstream>

using namespace std;

LED::LED(const string& arquivo) {
    nomeArquivo = arquivo;
}

void LED::adicionar(int rrn) {
    ofstream arquivo(nomeArquivo, ios::binary | ios::app);

    if (!arquivo.is_open())
        return;

    arquivo.write(reinterpret_cast<char*>(&rrn), sizeof(int));

    arquivo.close();
}

bool LED::obter(int &rrn) {
    ifstream arquivo(nomeArquivo, ios::binary);

    if (!arquivo.is_open())
        return false;

    arquivo.seekg(0, ios::end);

    if (arquivo.tellg() == 0) {
        arquivo.close();
        return false;
    }

    arquivo.close();

    return false;
}

bool LED::vazia() {
    ifstream arquivo(nomeArquivo, ios::binary);

    if (!arquivo.is_open())
        return true;

    arquivo.seekg(0, ios::end);

    bool resultado = (arquivo.tellg() == 0);

    arquivo.close();

    return resultado;
}
