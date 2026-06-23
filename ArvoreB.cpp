#include "ArvoreB.h"
#include <cstring>

static FILE* abrirOuCriarB(const char* nome) {
    FILE* f = fopen(nome, "r+b");
    if (!f) {
        f = fopen(nome, "w+b");
        CabecalhoBTree c;
        c.raiz = -1; c.numNos = 0; c.livre = -1; // arvore vazia
        fwrite(&c, sizeof(CabecalhoBTree), 1, f);
        fflush(f);
    }
    return f;
}

ArvoreB::ArvoreB(const char* nome) {
    strncpy(nomeArquivo, nome, sizeof(nomeArquivo) - 1);
    nomeArquivo[sizeof(nomeArquivo) - 1] = '\0';
    FILE* f = abrirOuCriarB(nomeArquivo);
    fclose(f);
}

// --------------------------- acesso a disco --------------------------------

CabecalhoBTree ArvoreB::lerCabecalho() {
    FILE* f = abrirOuCriarB(nomeArquivo);
    fseek(f, 0, SEEK_SET);
    CabecalhoBTree c;
    fread(&c, sizeof(CabecalhoBTree), 1, f);
    fclose(f);
    return c;
}

void ArvoreB::escreverCabecalho(const CabecalhoBTree& c) {
    FILE* f = abrirOuCriarB(nomeArquivo);
    fseek(f, 0, SEEK_SET);
    fwrite(&c, sizeof(CabecalhoBTree), 1, f);
    fclose(f);
}

// Offset do no idx = cabecalho + idx * tamanho do no
void ArvoreB::lerNo(int idx, NoBTree& no) {
    FILE* f = abrirOuCriarB(nomeArquivo);
    fseek(f, sizeof(CabecalhoBTree) + (long)idx * sizeof(NoBTree), SEEK_SET);
    fread(&no, sizeof(NoBTree), 1, f);
    fclose(f);
}

void ArvoreB::escreverNo(int idx, const NoBTree& no) {
    FILE* f = abrirOuCriarB(nomeArquivo);
    fseek(f, sizeof(CabecalhoBTree) + (long)idx * sizeof(NoBTree), SEEK_SET);
    fwrite(&no, sizeof(NoBTree), 1, f);
    fclose(f);
}

// Aloca um no: reaproveita um no livre (se houver) ou cria um novo no fim.
int ArvoreB::alocarNo() {
    CabecalhoBTree c = lerCabecalho();
    int idx;
    if (c.livre != -1) {            // ha no livre -> reusa (lista de nos livres)
        idx = c.livre;
        NoBTree no;
        lerNo(idx, no);
        c.livre = no.prox;          // proximo livre vira a nova cabeca
    } else {
        idx = c.numNos;             // cresce o arquivo
        c.numNos++;
    }
    escreverCabecalho(c);
    return idx;
}

// ----------------------------- INSERCAO ------------------------------------

void ArvoreB::inserir(int chave, int rrn) {
    CabecalhoBTree c = lerCabecalho();

    if (c.raiz == -1) {                 // arvore vazia: cria raiz folha
        int idx = alocarNo();
        NoBTree no;
        memset(&no, 0, sizeof(NoBTree));
        no.n = 1; no.folha = 1; no.prox = -1;
        no.chaves[0] = chave; no.dados[0] = rrn;
        escreverNo(idx, no);
        c = lerCabecalho();
        c.raiz = idx;
        escreverCabecalho(c);
        return;
    }

    NoBTree raiz;
    lerNo(c.raiz, raiz);
    if (raiz.n == MAX_CHAVES) {          // raiz cheia: a arvore cresce em altura
        int novaRaizIdx = alocarNo();
        NoBTree novaRaiz;
        memset(&novaRaiz, 0, sizeof(NoBTree));
        novaRaiz.n = 0; novaRaiz.folha = 0; novaRaiz.prox = -1;
        novaRaiz.filhos[0] = c.raiz;
        escreverNo(novaRaizIdx, novaRaiz);

        // atualiza raiz no cabecalho antes de dividir
        c = lerCabecalho();
        c.raiz = novaRaizIdx; 
        escreverCabecalho(c);

        lerNo(novaRaizIdx, novaRaiz);
        dividirFilho(novaRaizIdx, novaRaiz, 0);
        inserirNaoCheio(novaRaizIdx, chave, rrn);
    } else {
        inserirNaoCheio(c.raiz, chave, rrn);
    }
}

// Divide o filho 'i' (que esta cheio) do no 'pai'.
void ArvoreB::dividirFilho(int paiIdx, NoBTree& pai, int i) {
    int yIdx = pai.filhos[i];
    NoBTree y;
    lerNo(yIdx, y);

    int zIdx = alocarNo();
    NoBTree z;
    memset(&z, 0, sizeof(NoBTree));
    z.folha = y.folha;
    z.n = T - 1;
    z.prox = -1;

    // z recebe as ultimas (T-1) chaves de y
    for (int j = 0; j < T - 1; j++) {
        z.chaves[j] = y.chaves[j + T];
        z.dados[j]  = y.dados[j + T];
    }
    if (!y.folha) {
        for (int j = 0; j < T; j++)
            z.filhos[j] = y.filhos[j + T];
    }
    y.n = T - 1; // y fica com as primeiras (T-1) chaves; a chave do meio sobe

    // abre espaco no pai para o novo filho z (desloca pra frente)
    for (int j = pai.n; j >= i + 1; j--)
        pai.filhos[j + 1] = pai.filhos[j];
    pai.filhos[i + 1] = zIdx;

    for (int j = pai.n - 1; j >= i; j--) { 
        pai.chaves[j + 1] = pai.chaves[j];
        pai.dados[j + 1]  = pai.dados[j];
    }
    pai.chaves[i] = y.chaves[T - 1]; // chave mediana sobe para o pai
    pai.dados[i]  = y.dados[T - 1];
    pai.n++;

    escreverNo(yIdx, y);
    escreverNo(zIdx, z);
    escreverNo(paiIdx, pai);
}

void ArvoreB::inserirNaoCheio(int idx, int chave, int rrn) {
    NoBTree x;
    lerNo(idx, x);

    int i = x.n - 1;
    if (x.folha) {
        // insere ordenado na folha (desloca para a direita)
        while (i >= 0 && chave < x.chaves[i]) {
            x.chaves[i + 1] = x.chaves[i];
            x.dados[i + 1]  = x.dados[i];
            i--;
        }
        x.chaves[i + 1] = chave;
        x.dados[i + 1]  = rrn;
        x.n++;
        escreverNo(idx, x);
    } else {
        while (i >= 0 && chave < x.chaves[i]) i--;
        i++;
        NoBTree filho;
        lerNo(x.filhos[i], filho);
        if (filho.n == MAX_CHAVES) {       // filho cheio: divide antes de descer
            dividirFilho(idx, x, i);
            lerNo(idx, x);                 // x mudou
            if (chave > x.chaves[i]) i++;  // decide em qual metade descer
        }
        inserirNaoCheio(x.filhos[i], chave, rrn);
    }
}

