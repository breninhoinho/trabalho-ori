# Sistema de Cadastro de Jogos de Videogame

Trabalho final de **Organização e Recuperação de Informação** (UFSCar — 2026/1).

Um "SGBD do zero": camada de persistência e indexação implementada em C++ puro,
manipulando arquivos binários diretamente em disco (`fseek`/`fread`/`fwrite`),
sem uso de banco de dados pronto ou biblioteca de serialização.

## Integrantes

| Integrante | Responsabilidade principal |
|---|---|
| Breno Dias | Inserção |
| Guilherme Athayde | Remoção (Árvore B, listas invertidas, remoção lógica) |
| Leonardo Miasiro | Busca e atualização |
| Lorenna | LED (lista de espaços disponíveis) |

## O registro principal

Registros de **tamanho fixo** (232 bytes), definidos em `Registro.h`:

| Campo | Tipo | Papel |
|---|---|---|
| `id` | `int` | **Chave primária** (autoincremento, imutável; `-1` = removido) |
| `titulo` | `char[100]` | — |
| `desenvolvedora` | `char[60]` | — |
| `anoLancamento` | `int` | — |
| `notaMedia` | `float` | — |
| `genero` | `char[30]` | **Chave secundária 1** (não exclusiva) |
| `plataforma` | `char[30]` | **Chave secundária 2** (não exclusiva) |

## Arquitetura de arquivos

Todos os arquivos são criados automaticamente na primeira execução, no
diretório de trabalho:

| Arquivo | Conteúdo |
|---|---|
| `jogos.dat` | Dados principais: cabeçalho (próximo id + total de slots) + registros acessados por RRN |
| `btree.dat` | Índice primário: Árvore B persistida em disco (id → RRN) |
| `genero.idx` / `genero.inv` | Índice secundário de gênero (entradas ordenadas) + lista invertida (nós encadeados por RRN) |
| `plataforma.idx` / `plataforma.inv` | Índice secundário de plataforma + lista invertida |
| `led.dat` | LED — pilha (LIFO) de RRNs livres no arquivo de dados |

O acesso a qualquer posição segue sempre `offset = cabeçalho + posição × sizeof(struct)`.

## Estruturas implementadas

- **Árvore B (índice primário)** — ordem 6 (grau mínimo T = 3; máx. 5 chaves e
  mín. 2 chaves por nó, exceto a raiz), inteiramente em disco. Inserção com
  cisão (split) e promoção da chave mediana; remoção em uma única descida com
  redistribuição (rotação através do pai) e concatenação (merge) para tratar
  underflow, incluindo encolhimento de altura quando a raiz esvazia. Nós
  liberados entram numa lista de nós livres e são reaproveitados.
- **Índices secundários com lista invertida** — ligação fraca (a lista guarda a
  chave primária, não o endereço físico). O `.idx` é mantido ordenado para
  busca binária; o `.inv` encadeia os ids de cada chave por RRN. A remoção usa
  a estratégia *delete all references*, devolvendo o nó desligado à lista de
  livres do `.inv`.
- **Remoção lógica + LED (LIFO)** — remover marca o registro com tombstone
  (`id = -1`) e empilha o RRN no `led.dat`; a inserção desempilha o último RRN
  liberado e grava por cima do buraco, de modo que o arquivo de dados só cresce
  quando não há espaço reaproveitável.

## Compilação e execução

Requisitos: `g++` (C++11) e `make`.

```bash
make          # compila e gera o executavel "jogos"
./jogos       # menu interativo
make clean    # remove binario, objetos e arquivos de dados
```

## Funcionalidades do menu

- [x] Inserir jogo (com reaproveitamento de espaço via LED)
- [x] Remover jogo (remoção lógica + atualização de todos os índices) 
- [X] Buscar jogo por id (Árvore B)
- [X] Buscar jogos por gênero / plataforma (listas invertidas)
- [X] Atualizar jogo
