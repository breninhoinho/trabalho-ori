#ifndef GERENCIADOR_JOGOS_H
#define GERENCIADOR_JOGOS_H

#include "Registro.h"
#include "ArvoreB.h"
#include "IndiceSecundario.h"
#include <string>

class GerenciadorJogos {
private:
    char arqDados[256];
    ArvoreB          btree;
    IndiceSecundario idxGenero;
    IndiceSecundario idxPlataforma;

    CabecalhoDados lerCabecalho();
    void           escreverCabecalho(const CabecalhoDados& c);
    void           escreverRegistro(int rrn, const Jogo& j);

public:
    GerenciadorJogos();

    // CREATE principal: insere um jogo e retorna o id gerado (autoincremento).
    int inserir(const std::string& titulo, const std::string& desenvolvedora,
                int ano, float nota, const std::string& genero,
                const std::string& plataforma);
};

#endif // GERENCIADOR_JOGOS_H
