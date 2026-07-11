#ifndef GERENCIADOR_JOGOS_H
#define GERENCIADOR_JOGOS_H

#include "Registro.h"
#include "ArvoreB.h"
#include "IndiceSecundario.h"
#include "LED.h"
#include <string>

class GerenciadorJogos {
private:
    char arqDados[256];
    ArvoreB          btree;
    IndiceSecundario idxGenero;
    IndiceSecundario idxPlataforma;
    LED              led;

    CabecalhoDados lerCabecalho();
    void           escreverCabecalho(const CabecalhoDados& c);
    void           escreverRegistro(int rrn, const Jogo& j);
    Jogo           lerRegistro(int rrn);

public:
    GerenciadorJogos();

    // CREATE principal: insere um jogo e retorna o id gerado (autoincremento).
    int inserir(const std::string& titulo, const std::string& desenvolvedora,
                int ano, float nota, const std::string& genero,
                const std::string& plataforma);

    bool buscarPorId(int id, Jogo& saida);

    // DELETE principal: remocao LOGICA do jogo pelo id.
    // Remove o id da Arvore B e das listas invertidas, marca o registro como
    // removido em jogos.dat e cataloga o espaco na LED. Retorna false se o id
    // nao existe.
    bool remover(int id);
};

#endif // GERENCIADOR_JOGOS_H
