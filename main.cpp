
//  Compilar:
//    g++ -std=c++11 -O2 -o jogos main.cpp GerenciadorJogos.cpp ArvoreB.cpp IndiceSecundario.cpp
// ===========================================================================

#include "GerenciadorJogos.h"
#include <iostream>
#include <string>
#include <limits>
#include <cstdio>

using namespace std;

static void limparBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

static int lerInt(const string& prompt) {
    int x;
    while (true) {
        cout << prompt;
        if (cin >> x) { limparBuffer(); return x; }
        cout << "  Valor invalido, tente novamente.\n";
        limparBuffer();
    }
}

static float lerFloat(const string& prompt) {
    float x;
    while (true) {
        cout << prompt;
        if (cin >> x) { limparBuffer(); return x; }
        cout << "  Valor invalido, tente novamente.\n";
        limparBuffer();
    }
}

static string lerLinha(const string& prompt) {
    cout << prompt;
    string s;
    getline(cin, s);
    return s;
}

// ------------------------------ operacoes ----------------------------------

static void opInserir(GerenciadorJogos& g) {
    cout << "\n--- Inserir novo jogo ---\n";
    string titulo = lerLinha("Titulo: ");
    string desenv = lerLinha("Desenvolvedora: ");
    int    ano    = lerInt("Ano de lancamento: ");
    float  nota   = lerFloat("Nota media (0-10): ");
    string genero = lerLinha("Genero (ex: RPG, Acao): ");
    string plat   = lerLinha("Plataforma (ex: PC, PS5): ");

    int id = g.inserir(titulo, desenv, ano, nota, genero, plat);
    cout << ">> Jogo inserido com ID " << id << ".\n";
}

// -------------------------------- menu -------------------------------------

static void menu() {
    cout << "\n==================== SGBD de Jogos ====================\n";
    cout << " 1  - Inserir jogo\n";
    cout << " 0  - Sair\n";
    cout << "======================================================\n";
}

int main() {
    GerenciadorJogos g;   // abre/cria todos os arquivos do sistema

    while (true) {
        menu();
        int op = lerInt("Opcao: ");
        switch (op) {
            case 1:  opInserir(g);          break;
            case 0:  cout << "Encerrando.\n"; return 0;
            default: cout << "Opcao invalida.\n"; break;
        }
    }
    return 0;
}
