#ifndef REGISTRO_H
#define REGISTRO_H

#pragma pack(push, 1)

const int TAM_TITULO     = 100;
const int TAM_DESENV     = 60;
const int TAM_GENERO     = 30;   
const int TAM_PLATAFORMA = 30;
const int ID_REMOVIDO = -1;

// Registro principal
struct Jogo {
    int   id;                          // CHAVE PRIMARIA (autoincremento, imutavel; -1 = removido)
    char  titulo[TAM_TITULO];
    char  desenvolvedora[TAM_DESENV];
    int   anoLancamento;
    float notaMedia;
    char  genero[TAM_GENERO];          // CHAVE SECUNDARIA 1 (nao exclusiva)
    char  plataforma[TAM_PLATAFORMA];  // CHAVE SECUNDARIA 2 (nao exclusiva)
};

// Cabecalho gravado no inicio de jogos.dat
struct CabecalhoDados {
    int proximoId;   // proximo valor da chave primaria (autoincremento)
    int totalSlots;  // quantidade de slots ja alocados no arquivo de dados
};

#pragma pack(pop)

#endif // REGISTRO_H
