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

// ----------------------------- REMOCAO --------------------------------------

// Devolve o no 'idx' para a lista de nos livres do arquivo (o alocarNo vai
// reaproveitar esse espaco em insercoes futuras, evitando fragmentacao).
void ArvoreB::liberarNo(int idx) {
    CabecalhoBTree c = lerCabecalho();
    NoBTree no;
    lerNo(idx, no);
    no.n = 0;
    no.prox = c.livre;        // encadeia na frente da lista de livres
    escreverNo(idx, no);
    c.livre = idx;
    escreverCabecalho(c);
}

// Remove 'chave' da arvore. Devolve em 'rrnRemovido' o RRN associado a ela,
// para o chamador poder marcar o registro no arquivo de dados e alimentar a LED.
bool ArvoreB::remover(int chave, int& rrnRemovido) {
    CabecalhoBTree c = lerCabecalho();
    if (c.raiz == -1) return false;   // arvore vazia

    bool achou = false;
    removerDe(c.raiz, chave, rrnRemovido, achou);

    // Se a raiz ficou sem chaves, a arvore diminui de altura:
    // o unico filho vira a nova raiz (ou a arvore fica vazia).
    c = lerCabecalho();
    NoBTree raiz;
    lerNo(c.raiz, raiz);
    if (raiz.n == 0) {
        int antiga = c.raiz;
        c.raiz = raiz.folha ? -1 : raiz.filhos[0];
        escreverCabecalho(c);
        liberarNo(antiga);
    }
    return achou;
}

// Remocao recursiva em uma unica descida: antes de descer para um filho,
// garante que ele tenha pelo menos T chaves (assim a remocao nunca precisa
// voltar subindo para rebalancear).
void ArvoreB::removerDe(int idx, int chave, int& rrnRemovido, bool& achou) {
    NoBTree x;
    lerNo(idx, x);

    int i = 0;
    while (i < x.n && chave > x.chaves[i]) i++;

    if (i < x.n && x.chaves[i] == chave) {   // a chave esta neste no
        achou = true;
        rrnRemovido = x.dados[i];            // captura o RRN antes de remover
        if (x.folha) {
            // caso simples: desloca as chaves seguintes para a esquerda
            for (int j = i; j < x.n - 1; j++) {
                x.chaves[j] = x.chaves[j + 1];
                x.dados[j]  = x.dados[j + 1];
            }
            x.n--;
            escreverNo(idx, x);
        } else {
            removerDeInterno(idx, x, i);
        }
        return;
    }

    if (x.folha) return;   // chegou na folha e nao achou: chave nao existe

    bool eraUltimo = (i == x.n);   // a chave estaria na subarvore mais a direita
    NoBTree filho;
    lerNo(x.filhos[i], filho);
    if (filho.n < T) {             // filho no minimo (T-1): reforca antes de descer
        preencherFilho(idx, i);
        lerNo(idx, x);             // o no pode ter mudado (emprestimo ou fusao)
    }
    // Se houve fusao do ultimo filho, ele agora esta na posicao i-1
    if (eraUltimo && i > x.n) removerDe(x.filhos[i - 1], chave, rrnRemovido, achou);
    else                      removerDe(x.filhos[i],     chave, rrnRemovido, achou);
}

// Remove a chave na posicao i de um no INTERNO 'x' (ja carregado em memoria).
void ArvoreB::removerDeInterno(int idx, NoBTree& x, int i) {
    int chave = x.chaves[i];

    NoBTree esq, dir;
    lerNo(x.filhos[i],     esq);
    lerNo(x.filhos[i + 1], dir);

    // O RRN da chave original ja foi capturado pelo chamador; as remocoes
    // recursivas abaixo (da substituta) usam variaveis descartaveis.
    int  rrnDescarte  = -1;
    bool achouDescarte = false;

    if (esq.n >= T) {
        // substitui pela PREDECESSORA (maior chave da subarvore esquerda)
        // e remove a predecessora de la
        int pChave, pRrn;
        maiorDaSubarvore(x.filhos[i], pChave, pRrn);
        x.chaves[i] = pChave;
        x.dados[i]  = pRrn;
        escreverNo(idx, x);
        removerDe(x.filhos[i], pChave, rrnDescarte, achouDescarte);
    } else if (dir.n >= T) {
        // substitui pela SUCESSORA (menor chave da subarvore direita)
        int sChave, sRrn;
        menorDaSubarvore(x.filhos[i + 1], sChave, sRrn);
        x.chaves[i] = sChave;
        x.dados[i]  = sRrn;
        escreverNo(idx, x);
        removerDe(x.filhos[i + 1], sChave, rrnDescarte, achouDescarte);
    } else {
        // os dois filhos estao no minimo: funde (filho i + chave + filho i+1)
        // e continua a remocao dentro do no fundido
        int fundidoIdx = x.filhos[i];
        fundirFilhos(idx, i);
        removerDe(fundidoIdx, chave, rrnDescarte, achouDescarte);
    }
}

// Desce sempre pelo ultimo filho ate a folha: a maior chave da subarvore.
void ArvoreB::maiorDaSubarvore(int idx, int& chave, int& rrn) {
    NoBTree no;
    lerNo(idx, no);
    while (!no.folha)
        lerNo(no.filhos[no.n], no);
    chave = no.chaves[no.n - 1];
    rrn   = no.dados[no.n - 1];
}

// Desce sempre pelo primeiro filho ate a folha: a menor chave da subarvore.
void ArvoreB::menorDaSubarvore(int idx, int& chave, int& rrn) {
    NoBTree no;
    lerNo(idx, no);
    while (!no.folha)
        lerNo(no.filhos[0], no);
    chave = no.chaves[0];
    rrn   = no.dados[0];
}

// Garante que o filho i de 'idx' tenha pelo menos T chaves antes da descida:
// tenta emprestar do irmao da esquerda, depois do da direita; se ambos estao
// no minimo, funde com um deles.
void ArvoreB::preencherFilho(int idx, int i) {
    NoBTree x;
    lerNo(idx, x);

    if (i != 0) {
        NoBTree ant;
        lerNo(x.filhos[i - 1], ant);
        if (ant.n >= T) { emprestarDoAnterior(idx, i); return; }
    }
    if (i != x.n) {
        NoBTree prox;
        lerNo(x.filhos[i + 1], prox);
        if (prox.n >= T) { emprestarDoProximo(idx, i); return; }
    }
    if (i != x.n) fundirFilhos(idx, i);       // funde com o irmao da direita
    else          fundirFilhos(idx, i - 1);   // ultimo filho: funde com o da esquerda
}

// Rotacao pela esquerda: a chave separadora do pai desce para o filho i e a
// maior chave do irmao da esquerda sobe para o lugar dela.
void ArvoreB::emprestarDoAnterior(int idx, int i) {
    NoBTree x, filho, irmao;
    lerNo(idx, x);
    lerNo(x.filhos[i],     filho);
    lerNo(x.filhos[i - 1], irmao);

    // abre espaco no inicio do filho
    for (int j = filho.n - 1; j >= 0; j--) {
        filho.chaves[j + 1] = filho.chaves[j];
        filho.dados[j + 1]  = filho.dados[j];
    }
    if (!filho.folha) {
        for (int j = filho.n; j >= 0; j--)
            filho.filhos[j + 1] = filho.filhos[j];
        filho.filhos[0] = irmao.filhos[irmao.n];  // ultimo filho do irmao migra
    }

    filho.chaves[0] = x.chaves[i - 1];    // separadora do pai desce
    filho.dados[0]  = x.dados[i - 1];
    x.chaves[i - 1] = irmao.chaves[irmao.n - 1];  // maior do irmao sobe
    x.dados[i - 1]  = irmao.dados[irmao.n - 1];

    filho.n++;
    irmao.n--;

    escreverNo(x.filhos[i],     filho);
    escreverNo(x.filhos[i - 1], irmao);
    escreverNo(idx, x);
}

// Rotacao pela direita: a chave separadora do pai desce para o fim do filho i
// e a menor chave do irmao da direita sobe para o lugar dela.
void ArvoreB::emprestarDoProximo(int idx, int i) {
    NoBTree x, filho, irmao;
    lerNo(idx, x);
    lerNo(x.filhos[i],     filho);
    lerNo(x.filhos[i + 1], irmao);

    filho.chaves[filho.n] = x.chaves[i];  // separadora do pai desce
    filho.dados[filho.n]  = x.dados[i];
    if (!filho.folha)
        filho.filhos[filho.n + 1] = irmao.filhos[0];  // primeiro filho do irmao migra

    x.chaves[i] = irmao.chaves[0];        // menor do irmao sobe
    x.dados[i]  = irmao.dados[0];

    // fecha o buraco no inicio do irmao
    for (int j = 0; j < irmao.n - 1; j++) {
        irmao.chaves[j] = irmao.chaves[j + 1];
        irmao.dados[j]  = irmao.dados[j + 1];
    }
    if (!irmao.folha) {
        for (int j = 0; j < irmao.n; j++)
            irmao.filhos[j] = irmao.filhos[j + 1];
    }

    filho.n++;
    irmao.n--;

    escreverNo(x.filhos[i],     filho);
    escreverNo(x.filhos[i + 1], irmao);
    escreverNo(idx, x);
}

// Funde o filho i, a chave separadora i do pai e o filho i+1 em um unico no
// (ambos os filhos tem T-1 chaves, entao o resultado tem 2T-1 = no cheio).
// O no do irmao da direita e devolvido para a lista de nos livres.
void ArvoreB::fundirFilhos(int idx, int i) {
    NoBTree x, filho, irmao;
    lerNo(idx, x);
    int filhoIdx = x.filhos[i];
    int irmaoIdx = x.filhos[i + 1];
    lerNo(filhoIdx, filho);
    lerNo(irmaoIdx, irmao);

    // separadora do pai desce para o meio do no fundido
    filho.chaves[T - 1] = x.chaves[i];
    filho.dados[T - 1]  = x.dados[i];

    // copia as chaves e filhos do irmao para a metade direita
    for (int j = 0; j < irmao.n; j++) {
        filho.chaves[j + T] = irmao.chaves[j];
        filho.dados[j + T]  = irmao.dados[j];
    }
    if (!filho.folha) {
        for (int j = 0; j <= irmao.n; j++)
            filho.filhos[j + T] = irmao.filhos[j];
    }
    filho.n += irmao.n + 1;

    // fecha o buraco no pai (remove a separadora e o ponteiro para o irmao)
    for (int j = i; j < x.n - 1; j++) {
        x.chaves[j] = x.chaves[j + 1];
        x.dados[j]  = x.dados[j + 1];
    }
    for (int j = i + 1; j < x.n; j++)
        x.filhos[j] = x.filhos[j + 1];
    x.n--;

    escreverNo(filhoIdx, filho);
    escreverNo(idx, x);
    liberarNo(irmaoIdx);
}

// BUSCAR

// Retorna o RNN ou -1, caso não exista
int ArvoreB::buscar(int chave) {
    CabecalhoBTree c = lerCabecalho();

    int idx = c.raiz;
    while (idx != -1) {
        NoBTree x;
        lerNo(idx, x);

        int i = 0;
        while (i < x.n && chave > x.chaves[i]) 
            i++;
        
        if (i < x.n && x.chaves[i] == chave)
            return x.dados[i];

        if (x.folha) 
            return -1;

        idx = x.filhos[i];
    }
    return -1;
}
