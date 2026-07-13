#ifndef ARVOREB_H
#define ARVOREB_H

#include <cstdio>
#include <vector>

const int T          = 3;
const int MAX_CHAVES = 2 * T - 1;
const int MAX_FILHOS = 2 * T;

#pragma pack(push, 1)

struct NoBTree {
    int n;
    int folha;
    int chaves[MAX_CHAVES];
    int dados[MAX_CHAVES];
    int filhos[MAX_FILHOS];
    int prox;
};

struct CabecalhoBTree {
    int raiz;
    int numNos;
    int livre;
};

#pragma pack(pop)

class ArvoreB {
private:
    char nomeArquivo[256];

    // ACESSO AO DISCO
    CabecalhoBTree lerCabecalho();
    void           escreverCabecalho(const CabecalhoBTree& c);
    void           lerNo(int idx, NoBTree& no);
    void           escreverNo(int idx, const NoBTree& no);
    int            alocarNo();

    // INSERCAO
    void dividirFilho(int paiIdx, NoBTree& pai, int i);
    void inserirNaoCheio(int idx, int chave, int rrn);

    // REMOCAO
    void liberarNo(int idx);       
    void removerDe(int idx, int chave, int& rrnRemovido, bool& achou);
    void removerDeInterno(int idx, NoBTree& x, int i);
    void maiorDaSubarvore(int idx, int& chave, int& rrn);
    void menorDaSubarvore(int idx, int& chave, int& rrn);
    void preencherFilho(int idx, int i);
    void emprestarDoAnterior(int idx, int i);
    void emprestarDoProximo(int idx, int i);
    void fundirFilhos(int idx, int i);

public:
    explicit ArvoreB(const char* nome);

    // CREATE: insere uma nova associacao entre a chave (ID) e o RRN do registro.
    void inserir(int chave, int rrn);

    // READ: busca uma chave na Arvore B e retorna o RRN associado.
    // Retorna -1 caso a chave nao exista.
    int  buscar(int chave);

    // UPDATE: atualiza o RRN associado a uma chave existente.
    // Retorna true se a chave foi encontrada e atualizada.
    bool atualizar(int chave, int rrn);

    // DELETE: remove uma chave da Arvore B.
    // Retorna true se a chave existia e devolve em 'rrnRemovido' o RRN que estava associado a ela.
    bool remover(int chave, int& rrnRemovido);
};

#endif // ARVOREB_H
