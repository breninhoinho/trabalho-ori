#include "IndiceSecundario.h"
#include <cstring>
#include <cstdlib>   // bsearch

// Comparador usado por bsearch: compara duas entradas pela chave.
static int cmpEntrada(const void* a, const void* b) {
    const EntradaIdx* ea = (const EntradaIdx*) a;
    const EntradaIdx* eb = (const EntradaIdx*) b;
    return strncmp(ea->chave, eb->chave, TAM_CHAVE_SEC);
}

static FILE* abrirOuCriarIdx(const char* nome) {
    FILE* f = fopen(nome, "r+b");
    if (!f) {
        f = fopen(nome, "w+b");
        CabecalhoIdx c; c.numEntradas = 0;
        fwrite(&c, sizeof(CabecalhoIdx), 1, f);
        fflush(f);
    }
    return f;
}

static FILE* abrirOuCriarInv(const char* nome) {
    FILE* f = fopen(nome, "r+b");
    if (!f) {
        f = fopen(nome, "w+b");
        CabecalhoInv c; c.totalNos = 0; c.livre = -1;
        fwrite(&c, sizeof(CabecalhoInv), 1, f);
        fflush(f);
    }
    return f;
}

// Copia uma string para um buffer fixo, preenchendo o resto com '\0'.
static void copiarChave(char* dest, const std::string& src, int tam) {
    memset(dest, 0, tam);
    strncpy(dest, src.c_str(), tam - 1);
}

IndiceSecundario::IndiceSecundario(const char* nomeIdx, const char* nomeInv) {
    strncpy(arqIdx, nomeIdx, sizeof(arqIdx) - 1); arqIdx[sizeof(arqIdx) - 1] = '\0';
    strncpy(arqInv, nomeInv, sizeof(arqInv) - 1); arqInv[sizeof(arqInv) - 1] = '\0';
    FILE* a = abrirOuCriarIdx(arqIdx); fclose(a);
    FILE* b = abrirOuCriarInv(arqInv); fclose(b);
}

// ------------------------------- .idx --------------------------------------

CabecalhoIdx IndiceSecundario::lerCabIdx() {
    FILE* f = abrirOuCriarIdx(arqIdx);
    fseek(f, 0, SEEK_SET);
    CabecalhoIdx c; fread(&c, sizeof(CabecalhoIdx), 1, f);
    fclose(f);
    return c;
}

void IndiceSecundario::escreverCabIdx(const CabecalhoIdx& c) {
    FILE* f = abrirOuCriarIdx(arqIdx);
    fseek(f, 0, SEEK_SET);
    fwrite(&c, sizeof(CabecalhoIdx), 1, f);
    fclose(f);
}

// Procura uma chave por BUSCA BINARIA (bsearch). Carrega as entradas (que estao
// ordenadas por chave) para a memoria e aplica bsearch.
// Retorna a posicao da entrada (0..n-1) ou -1, e preenche 'saida'.
int IndiceSecundario::buscarEntrada(const char* chave, EntradaIdx& saida) {
    CabecalhoIdx c = lerCabIdx();
    if (c.numEntradas == 0) return -1;

    std::vector<EntradaIdx> entradas(c.numEntradas);
    FILE* f = abrirOuCriarIdx(arqIdx);
    fseek(f, sizeof(CabecalhoIdx), SEEK_SET);
    fread(&entradas[0], sizeof(EntradaIdx), c.numEntradas, f);
    fclose(f);

    EntradaIdx chaveBusca;
    memset(&chaveBusca, 0, sizeof(EntradaIdx));
    memcpy(chaveBusca.chave, chave, TAM_CHAVE_SEC);

    EntradaIdx* achado = (EntradaIdx*) bsearch(
        &chaveBusca, &entradas[0], entradas.size(),
        sizeof(EntradaIdx), cmpEntrada);

    if (!achado) return -1;
    saida = *achado;
    return (int)(achado - &entradas[0]);   // posicao == RRN no .idx
}

void IndiceSecundario::escreverEntrada(int pos, const EntradaIdx& e) {
    FILE* f = abrirOuCriarIdx(arqIdx);
    fseek(f, sizeof(CabecalhoIdx) + (long)pos * sizeof(EntradaIdx), SEEK_SET);
    fwrite(&e, sizeof(EntradaIdx), 1, f);
    fclose(f);
}

// Cria uma entrada nova mantendo o .idx ORDENADO por chave (pre-requisito do
// bsearch). Carrega tudo, insere na posicao ordenada e regrava.
int IndiceSecundario::criarEntrada(const char* chave) {
    CabecalhoIdx c = lerCabIdx();

    std::vector<EntradaIdx> entradas(c.numEntradas);
    if (c.numEntradas > 0) {
        FILE* f = abrirOuCriarIdx(arqIdx);
        fseek(f, sizeof(CabecalhoIdx), SEEK_SET);
        fread(&entradas[0], sizeof(EntradaIdx), c.numEntradas, f);
        fclose(f);
    }

    EntradaIdx nova;
    memset(&nova, 0, sizeof(EntradaIdx));
    memcpy(nova.chave, chave, TAM_CHAVE_SEC);
    nova.cabeca = -1;              // lista invertida ainda vazia

    // localiza a posicao de insercao que mantem a ordem crescente das chaves
    int pos = 0;
    while (pos < (int)entradas.size() &&
           strncmp(entradas[pos].chave, chave, TAM_CHAVE_SEC) < 0)
        pos++;
    entradas.insert(entradas.begin() + pos, nova);

    // regrava todas as entradas em ordem
    FILE* f = abrirOuCriarIdx(arqIdx);
    fseek(f, sizeof(CabecalhoIdx), SEEK_SET);
    fwrite(&entradas[0], sizeof(EntradaIdx), entradas.size(), f);
    fclose(f);

    c.numEntradas++;
    escreverCabIdx(c);
    return pos;
}

// ------------------------------- .inv --------------------------------------

CabecalhoInv IndiceSecundario::lerCabInv() {
    FILE* f = abrirOuCriarInv(arqInv);
    fseek(f, 0, SEEK_SET);
    CabecalhoInv c; fread(&c, sizeof(CabecalhoInv), 1, f);
    fclose(f);
    return c;
}

void IndiceSecundario::escreverCabInv(const CabecalhoInv& c) {
    FILE* f = abrirOuCriarInv(arqInv);
    fseek(f, 0, SEEK_SET);
    fwrite(&c, sizeof(CabecalhoInv), 1, f);
    fclose(f);
}

void IndiceSecundario::lerNoInv(int idx, NoInv& no) {
    FILE* f = abrirOuCriarInv(arqInv);
    fseek(f, sizeof(CabecalhoInv) + (long)idx * sizeof(NoInv), SEEK_SET);
    fread(&no, sizeof(NoInv), 1, f);
    fclose(f);
}

void IndiceSecundario::escreverNoInv(int idx, const NoInv& no) {
    FILE* f = abrirOuCriarInv(arqInv);
    fseek(f, sizeof(CabecalhoInv) + (long)idx * sizeof(NoInv), SEEK_SET);
    fwrite(&no, sizeof(NoInv), 1, f);
    fclose(f);
}

int IndiceSecundario::alocarNoInv() {
    CabecalhoInv c = lerCabInv();
    int idx;
    if (c.livre != -1) {           // reaproveita no livre
        idx = c.livre;
        NoInv no; lerNoInv(idx, no);
        c.livre = no.prox;
    } else {
        idx = c.totalNos;
        c.totalNos++;
    }
    escreverCabInv(c);
    return idx;
}

// ------------------------------ CRUD ---------------------------------------

// Insere o id na lista invertida da chave (insercao no inicio da lista).
void IndiceSecundario::inserir(const std::string& chave, int idJogo) {
    char ch[TAM_CHAVE_SEC];
    copiarChave(ch, chave, TAM_CHAVE_SEC);

    EntradaIdx e;
    int pos = buscarEntrada(ch, e);
    if (pos == -1) {               // chave nova: cria entrada no .idx
        pos = criarEntrada(ch);
        buscarEntrada(ch, e);
    }

    int novo = alocarNoInv();
    NoInv no;
    no.idJogo = idJogo;
    no.prox   = e.cabeca;          // aponta para a antiga cabeca
    escreverNoInv(novo, no);

    e.cabeca = novo;               // nova cabeca da lista
    escreverEntrada(pos, e);
}

// Remove o id da lista invertida da chave. Percorre a lista encadeada no .inv
// desligando o no encontrado; o no liberado entra na lista de livres para ser
// reaproveitado por insercoes futuras (mesma ideia da LED no arquivo de dados).
void IndiceSecundario::remover(const std::string& chave, int idJogo) {
    char ch[TAM_CHAVE_SEC];
    copiarChave(ch, chave, TAM_CHAVE_SEC);

    EntradaIdx e;
    int pos = buscarEntrada(ch, e);
    if (pos == -1) return;             // chave nao indexada: nada a fazer

    int atual = e.cabeca;
    int anterior = -1;
    while (atual != -1) {
        NoInv no;
        lerNoInv(atual, no);
        if (no.idJogo == idJogo) {
            if (anterior == -1) {      // era a cabeca: entrada aponta p/ o proximo
                e.cabeca = no.prox;
                escreverEntrada(pos, e);
            } else {                   // no do meio/fim: religa o anterior
                NoInv ant;
                lerNoInv(anterior, ant);
                ant.prox = no.prox;
                escreverNoInv(anterior, ant);
            }
            // devolve o no para a lista de livres do .inv
            CabecalhoInv c = lerCabInv();
            no.prox = c.livre;
            escreverNoInv(atual, no);
            c.livre = atual;
            escreverCabInv(c);
            return;
        }
        anterior = atual;
        atual = no.prox;
    }
}

std::vector<int> IndiceSecundario::buscar(const std::string& chave) {
    char ch[TAM_CHAVE_SEC];
    copiarChave(ch, chave, TAM_CHAVE_SEC);

    std::vector<int> ids;
    EntradaIdx e;
    int pos = buscarEntrada(ch, e);    // <- busca binaria via bsearch
    if (pos == -1) return ids;         // chave nao indexada

    int atual = e.cabeca;
    while (atual != -1) {
        NoInv no; lerNoInv(atual, no);
        ids.push_back(no.idJogo);
        atual = no.prox;
    }
    return ids;
}

// Listar as Chaves Secundárias
std::vector<std::string> IndiceSecundario::listarChaves() {
    std::vector<std::string> chaves;
    CabecalhoIdx c = lerCabIdx();

    FILE* f = abrirOuCriarIdx(arqIdx);
    fseek(f, sizeof(CabecalhoIdx), SEEK_SET);

    for (int i = 0; i < c.numEntradas; i++) {
        EntradaIdx e;
        fread(&e, sizeof(EntradaIdx), 1, f);
        if (e.cabeca != -1)
            chaves.push_back(std::string(e.chave));
    }
    fclose(f);
    return chaves;
}
