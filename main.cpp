
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

// Print Dados

static void imprimirJogo(const Jogo& j) {
    printf("  %-4s | %-30s | %-18s | %-4s | %-8s | %-20s\n", "ID", "Titulo", "Desenvolvedora", "Ano", "Nota", "Genero / Plataforma");
    printf("  %-4d | %-30s | %-18s | %-4d | %-8.1f | %s / %s\n", j.id, j.titulo, j.desenvolvedora, j.anoLancamento, j.notaMedia, j.genero, j.plataforma);
}

static void imprimirLista(const vector<Jogo>& v) {
    if (v.empty()) { cout << "  (nenhum registro encontrado)\n"; return; }
    for (size_t i = 0; i < v.size(); i++) imprimirJogo(v[i]);
    cout << "  Total: " << v.size() << " registro(s).\n";
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

static void opRemover(GerenciadorJogos& g) {
    cout << "\n--- Remover jogo ---\n";
    int id = lerInt("ID do jogo a remover: ");

    if (g.remover(id))
        cout << ">> Jogo de ID " << id << " removido (remocao logica);"
             << " espaco catalogado na LED.\n";
    else
        cout << ">> Jogo de ID " << id << " nao encontrado.\n";
}

static void opBuscarId(GerenciadorJogos& g) {
    int id = lerInt("\nID a buscar: ");
    Jogo j;
    if (g.buscarPorId(id, j)) { cout << "Encontrado:\n"; imprimirJogo(j); }
    else cout << ">> Nenhum jogo ativo com ID " << id << ".\n";
}

static void opBuscarGenero(GerenciadorJogos& g) {
    vector<string> chaves = g.generosIndexados();
    cout << "\nGeneros indexados: ";
    for (size_t i = 0; i < chaves.size(); i++) cout << chaves[i] << (i+1<chaves.size()?", ":"");
    cout << "\n";
    string gen = lerLinha("Genero a buscar: ");
    imprimirLista(g.buscarPorGenero(gen));
}

static void opBuscarPlataforma(GerenciadorJogos& g) {
    vector<string> chaves = g.plataformasIndexadas();
    cout << "\nPlataformas indexadas: ";
    for (size_t i = 0; i < chaves.size(); i++) cout << chaves[i] << (i+1<chaves.size()?", ":"");
    cout << "\n";
    string p = lerLinha("Plataforma a buscar: ");
    imprimirLista(g.buscarPorPlataforma(p));
}

static void opAtualizar(GerenciadorJogos& g) {
    int id = lerInt("\nID a atualizar: ");
    Jogo j;
    if (!g.buscarPorId(id, j)) {
        cout << ">> Nenhum jogo ativo com ID " << id << ".\n";
        return;
    }
    cout << "Registro atual:\n"; imprimirJogo(j);
    cout << "(a chave primaria/ID NAO pode ser alterada)\n";

    string titulo = lerLinha("Novo titulo: ");
    string desenv = lerLinha("Nova desenvolvedora: ");
    int    ano    = lerInt("Novo ano: ");
    float  nota   = lerFloat("Nova nota media: ");
    string genero = lerLinha("Novo genero: ");
    string plat   = lerLinha("Nova plataforma: ");

    if (g.atualizar(id, titulo, desenv, ano, nota, genero, plat))
        cout << ">> Registro atualizado.\n";
    else
        cout << ">> Falha ao atualizar.\n";
}


// -------------------------------- menu -------------------------------------

static void menu() {
    cout << "\n==================== SGBD de Jogos ====================\n";
    cout << " 1  - Inserir jogo\n";
    cout << " 2  - Buscar por ID\n";
    cout << " 3  - Atualizar Jogo\n";
    cout << " 4  - Remover jogo\n";
    cout << " 5  - Buscar por Genero\n";
    cout << " 6  - Buscar por Plataforma\n";
    cout << " 0  - Sair\n";
    cout << "======================================================\n";
}

int main() {
    GerenciadorJogos g;   // abre/cria todos os arquivos do sistema

    while (true) {
        menu();
        int op = lerInt("Opcoes: ");
        switch (op) {
            case 1:  
                opInserir(g);
                break;
            case 2:
                opBuscarId(g);
                break;
            case 3:
                opAtualizar(g);
                break;
            case 4:  
                opRemover(g);
                break;
            case 5:
                opBuscarGenero(g);
                break;
            case 6:
                opBuscarPlataforma(g);
                break;
            case 0:  
                cout << "Encerrando.\n"; 
                return 0;
            default: 
                cout << "Opcao invalida.\n"; 
                break;
        }
    }
    return 0;
}
