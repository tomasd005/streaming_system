# Streaming System (LI3 - Fase 2)

Sistema em C para processamento de um dataset de streaming de musica, com validacao de dados, execucao de queries e analise de desempenho.

## Executaveis

- `programa-principal`: carrega o dataset e executa um ficheiro de comandos.
- `programa-interativo`: interface de consola para executar queries manualmente.
- `programa-testes`: executa comandos, compara com resultados esperados e gera benchmark.

## Arquitetura

Estrutura modular (encapsulada por modulo):

- `src/entidades`: modelos (`musicas`, `users`, `artistas`, `albuns`, `historico`).
- `src/gestores`: repositores e validacao logica por entidade.
- `src/parsers`: parser CSV streaming com suporte a aspas e erros por ficheiro.
- `src/validacoes`: validacoes sintaticas.
- `src/queries`: implementacao das queries Q1..Q6.
- `src/gestor_programa`: orquestracao de carregamento e dispatch de queries.
- `src/gestor_programas`: modos de execucao (`teste` e `interativo`).

## Requisitos

- `gcc` (ou `clang`)
- `make`
- `pkg-config`
- `glib-2.0`

## Compilacao

```bash
make
make programa-interativo
make programa-testes
```

## Utilizacao

### Programa principal

```bash
./programa-principal <pasta_dataset> <input.txt>
```

Exemplo:

```bash
./programa-principal dataset input.txt
```

Outputs por comando:

- `resultados/command1_output.txt`
- `resultados/command2_output.txt`
- ...

Erros de validacao por entidade:

- `resultados/artists_errors.csv`
- `resultados/albums_errors.csv`
- `resultados/musics_errors.csv`
- `resultados/users_errors.csv`
- `resultados/history_errors.csv`

### Programa de testes

```bash
./programa-testes <pasta_dataset> <input.txt> <resultados_esperados>
```

Exemplo:

```bash
./programa-testes dataset input.txt expected_outputs
```

Gera:

- resumo de tempo/memoria no terminal
- metrica por query (Q1..Q6)
- primeira discrepancia (se existir)
- `resultados/benchmark.json`

### Make targets para benchmark

```bash
make benchmark EXPECTED=<pasta_resultados_esperados> [DATASET=dataset] [INPUT=input.txt]
make autotune EXPECTED=<pasta_resultados_esperados> [DATASET=dataset] [INPUT=input.txt]
```

## Tuning de performance

O parser suporta paralelismo configuravel por variaveis de ambiente:

- `LI3_PARSE_THREADS` (default: `1`, max: `16`)
- `LI3_PARSE_BATCH` (default: `2048`, max: `65536`)

Exemplo manual:

```bash
LI3_PARSE_THREADS=4 LI3_PARSE_BATCH=4096 ./programa-testes dataset input.txt expected_outputs
```

### Autotune automatico

Para procurar automaticamente a melhor configuracao de parse na tua maquina:

```bash
LI3_AUTOTUNE_PARSE=1 ./programa-testes dataset input.txt expected_outputs
```

Ou via make:

```bash
make autotune EXPECTED=expected_outputs
```

O autotune escreve:

- `resultados/autotune_parse.csv` (todas as combinacoes testadas)
- `resultados/benchmark.json` (resultado final com melhor configuracao encontrada)

## Limpeza

```bash
make clean
```

Remove binarios, objetos e artefactos de benchmark/output em `resultados/`.
