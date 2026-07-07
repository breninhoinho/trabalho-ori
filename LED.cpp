#include "LED.h"

#include <fstream>
#include <vector>

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

    vector<int> rrns;
    int valor;

    while (arquivo.read(reinterpret_cast<char*>(&valor), sizeof(int)))
        rrns.push_back(valor);

    arquivo.close();

    if (rrns.empty())
        return false;

    // Reutiliza o último RRN liberado (LIFO)
    rrn = rrns.back();
    rrns.pop_back();

    // Atualiza a LED removendo o RRN utilizado
    ofstream saida(nomeArquivo, ios::binary | ios::trunc);

    if (!rrns.empty())
        saida.write(reinterpret_cast<char*>(&rrns[0]), rrns.size() * sizeof(int));

    saida.close();

    return true;
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
