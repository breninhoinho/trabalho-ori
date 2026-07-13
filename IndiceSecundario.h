#ifndef INDICE_SECUNDARIO_H
#define INDICE_SECUNDARIO_H

#include <cstdio>
#include <string>
#include <vector>

const int TAM_CHAVE_SEC = 30; // tamanho fixo da chave secundaria (genero/plataforma)

#pragma pack(push, 1)

struct EntradaIdx {
    char chave[TAM_CHAVE_SEC];
    int  cabeca;   // indice do primeiro no da lista invertida no .inv (-1 se vazia)
};

struct CabecalhoIdx {
    int numEntradas;
};

struct NoInv {
    int idJogo;  // chave primaria do jogo que possui esta chave secundaria
    int prox;    // proximo no da lista (-1 = fim); tambem encadeia nos livres
};

struct CabecalhoInv {
    int totalNos; // quantidade de slots de no ja alocados
    int livre;    // cabeca da lista de nos livres (-1 = nenhum) -> reuso
};

#pragma pack(pop)

class IndiceSecundario {
private:
    char arqIdx[256];
    char arqInv[256];

    // .IDX
    CabecalhoIdx lerCabIdx();
    void         escreverCabIdx(const CabecalhoIdx& c);
    int          buscarEntrada(const char* chave, EntradaIdx& saida);
    void         escreverEntrada(int pos, const EntradaIdx& e);
    int          criarEntrada(const char* chave);

    // .INV
    CabecalhoInv lerCabInv();
    void         escreverCabInv(const CabecalhoInv& c);
    void         lerNoInv(int idx, NoInv& no);
    void         escreverNoInv(int idx, const NoInv& no);
    int          alocarNoInv();

public:
    IndiceSecundario(const char* nomeIdx, const char* nomeInv);

    // CREATE do indice secundario: associa chave -> id
    void inserir(const std::string& chave, int idJogo);

    // DELETE: desassocia chave -> id (remove o no da lista invertida;
    // o no removido volta para a lista de livres do .inv).
    void remover(const std::string& chave, int idJogo);

    // READ: retorna os ids associados a uma chave (busca binaria no .idx
    // via bsearch + percurso da lista invertida no .inv).
    std::vector<int> buscar(const std::string& chave);

    // Listar todas as chaves
    std::vector<std::string> listarChaves();
};

#endif // INDICE_SECUNDARIO_H
