# Trabalho Pratico LI3 - Streaming de Musica

Projeto inicial da Fase 2, estruturado em modulos (`entidades`, `gestores`, `parsers`, `validacoes`, `queries`) e com tres executaveis:

- `programa-principal`
- `programa-interativo`
- `programa-testes`

## Compilar

```bash
make
make programa-interativo
make programa-testes
```

## Executar

```bash
./programa-principal <pasta_dataset> <input.txt>
./programa-interativo
./programa-testes <pasta_dataset> <input.txt> <resultados_esperados>
```

## Estado atual

- Estrutura completa criada conforme enunciado.
- Validacoes sintaticas base implementadas.
- Queries Q1..Q6 com stubs seguros (retornam output vazio sem crash).
- Proximo passo: implementar regras logicas e resultados reais de cada query.
