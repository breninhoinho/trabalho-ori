#include "GerenciadorJogos.h"
#include <cstring>
#include <cstdio>

// Nomes dos arquivos do sistema (todos persistidos em disco)
static const char* ARQ_DADOS      = "jogos.dat";
static const char* ARQ_BTREE      = "btree.dat";
static const char* ARQ_GEN_IDX    = "genero.idx";
static const char* ARQ_GEN_INV    = "genero.inv";
static const char* ARQ_PLAT_IDX   = "plataforma.idx";
static const char* ARQ_PLAT_INV   = "plataforma.inv";

static FILE* abrirOuCriarDados(const char* nome) {
    FILE* f = fopen(nome, "r+b");
    if (!f) {
        f = fopen(nome, "w+b");
        CabecalhoDados c;
        c.proximoId = 1;   // a chave primaria comeca em 1
        c.totalSlots = 0;
        fwrite(&c, sizeof(CabecalhoDados), 1, f);
        fflush(f);
    }
    return f;
}

static void copiarCampo(char* dest, const std::string& src, int tam) {
    memset(dest, 0, tam);
    strncpy(dest, src.c_str(), tam - 1);
}

GerenciadorJogos::GerenciadorJogos()
    : btree(ARQ_BTREE),
      idxGenero(ARQ_GEN_IDX, ARQ_GEN_INV),
      idxPlataforma(ARQ_PLAT_IDX, ARQ_PLAT_INV)
{
    strncpy(arqDados, ARQ_DADOS, sizeof(arqDados) - 1);
    arqDados[sizeof(arqDados) - 1] = '\0';
    FILE* f = abrirOuCriarDados(arqDados);
    fclose(f);
}

// --------------------------- acesso a jogos.dat ----------------------------

CabecalhoDados GerenciadorJogos::lerCabecalho() {
    FILE* f = abrirOuCriarDados(arqDados);
    fseek(f, 0, SEEK_SET);
    CabecalhoDados c; fread(&c, sizeof(CabecalhoDados), 1, f);
    fclose(f);
    return c;
}

void GerenciadorJogos::escreverCabecalho(const CabecalhoDados& c) {
    FILE* f = abrirOuCriarDados(arqDados);
    fseek(f, 0, SEEK_SET);
    fwrite(&c, sizeof(CabecalhoDados), 1, f);
    fclose(f);
}

// offset do registro = cabecalho + rrn * tamanho do registro
void GerenciadorJogos::escreverRegistro(int rrn, const Jogo& j) {
    FILE* f = abrirOuCriarDados(arqDados);
    fseek(f, sizeof(CabecalhoDados) + (long)rrn * sizeof(Jogo), SEEK_SET);
    fwrite(&j, sizeof(Jogo), 1, f);
    fclose(f);
}

// ------------------------------- INSERIR -----------------------------------

int GerenciadorJogos::inserir(const std::string& titulo,
                              const std::string& desenvolvedora, int ano,
                              float nota, const std::string& genero,
                              const std::string& plataforma) {
    CabecalhoDados cab = lerCabecalho();

    Jogo j;
    memset(&j, 0, sizeof(Jogo));
    j.id = cab.proximoId;                 // chave primaria (autoincremento)
    copiarCampo(j.titulo, titulo, TAM_TITULO);
    copiarCampo(j.desenvolvedora, desenvolvedora, TAM_DESENV);
    j.anoLancamento = ano;
    j.notaMedia = nota;
    copiarCampo(j.genero, genero, TAM_GENERO);
    copiarCampo(j.plataforma, plataforma, TAM_PLATAFORMA);

    // 1) Escolhe o RRN: reaproveita um buraco da LED ou cresce o arquivo
    int rrn = led.desempilhar();          // estrategia LIFO
    if (rrn == -1) {
        rrn = cab.totalSlots;             // append no fim
        cab.totalSlots++;
    }

    // 2) Grava o registro no arquivo de dados
    escreverRegistro(rrn, j);

    // 3) Atualiza o cabecalho (proximo id + total de slots)
    cab.proximoId++;
    escreverCabecalho(cab);

    // 4) Atualiza o indice primario (Arvore B) e os secundarios
    btree.inserir(j.id, rrn);
    idxGenero.inserir(genero, j.id);
    idxPlataforma.inserir(plataforma, j.id);

    return j.id;
}
