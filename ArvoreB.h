#ifndef ARVOREB_H
#define ARVOREB_H

#include <cstdio>
#include <vector>

const int T          = 3;        // grau minimo da Arvore B
const int MAX_CHAVES = 2 * T - 1; // 5
const int MAX_FILHOS = 2 * T;     // 6

#pragma pack(push, 1)

struct NoBTree {
    int n;                      // numero de chaves atualmente no no
    int folha;                  // 1 = folha, 0 = no interno
    int chaves[MAX_CHAVES];     // chaves primarias (ids), em ordem crescente
    int dados[MAX_CHAVES];      // RRN correspondente a cada chave (no arquivo de dados)
    int filhos[MAX_FILHOS];     // indices dos nos filhos no arquivo btree.dat
    int prox;                   // usado apenas quando o no esta livre (lista de nos livres)
};

struct CabecalhoBTree {
    int raiz;    // indice do no raiz (-1 se a arvore esta vazia)
    int numNos;  // total de slots de no ja alocados no arquivo
    int livre;   // cabeca da lista de nos livres (-1 se nao ha) -> reuso de nos
};

#pragma pack(pop)

class ArvoreB {
private:
    char nomeArquivo[256];

    // --- acesso a disco ---
    CabecalhoBTree lerCabecalho();
    void           escreverCabecalho(const CabecalhoBTree& c);
    void           lerNo(int idx, NoBTree& no);
    void           escreverNo(int idx, const NoBTree& no);
    int            alocarNo();        // devolve indice de um no novo (reusa livres)

    // --- operacoes internas de insercao ---
    void dividirFilho(int paiIdx, NoBTree& pai, int i);
    void inserirNaoCheio(int idx, int chave, int rrn);

public:
    explicit ArvoreB(const char* nome);

    // CRUD do indice primario:
    void inserir(int chave, int rrn);  // CREATE: associa id -> rrn

};

#endif // ARVOREB_H
