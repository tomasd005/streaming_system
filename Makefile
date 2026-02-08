CC ?= gcc
PKG_CONFIG ?= pkg-config

CSTD := -std=c11
WARN := -Wall -Wextra -Wpedantic
CFLAGS := $(CSTD) $(WARN) -O2 -D_GNU_SOURCE
CPPFLAGS := -Iinclude
LDLIBS := $(shell $(PKG_CONFIG) --libs glib-2.0)
CPPFLAGS += $(shell $(PKG_CONFIG) --cflags glib-2.0)

OBJDIR := build/obj

SRC_COMMON := \
	src/main.c \
	src/utils.c \
	src/output.c \
	src/parsers/parser.c \
	src/validacoes/validacao_comum.c \
	src/validacoes/validacao_musicas.c \
	src/validacoes/validacao_users.c \
	src/validacoes/validacao_artistas.c \
	src/validacoes/validacao_albuns.c \
	src/validacoes/validacao_historico.c \
	src/entidades/musicas.c \
	src/entidades/users.c \
	src/entidades/artistas.c \
	src/entidades/albuns.c \
	src/entidades/historico.c \
	src/gestores/gestor_musicas.c \
	src/gestores/gestor_users.c \
	src/gestores/gestor_artistas.c \
	src/gestores/gestor_albuns.c \
	src/gestores/gestor_historico.c \
	src/gestor_programa/gestor_programa.c \
	src/gestor_programa/gestor_queries.c \
	src/queries/query1.c \
	src/queries/query2.c \
	src/queries/query3.c \
	src/queries/query4.c \
	src/queries/query5.c \
	src/queries/query6.c

SRC_PRINCIPAL := src/programas/main.c
SRC_INTERATIVO := src/programas/interativo.c src/gestor_programas/gestor_interativo.c
SRC_TESTES := src/programas/teste.c src/gestor_programas/gestor_teste.c

OBJ_COMMON := $(addprefix $(OBJDIR)/,$(SRC_COMMON:.c=.o))
OBJ_PRINCIPAL := $(addprefix $(OBJDIR)/,$(SRC_PRINCIPAL:.c=.o))
OBJ_INTERATIVO := $(addprefix $(OBJDIR)/,$(SRC_INTERATIVO:.c=.o))
OBJ_TESTES := $(addprefix $(OBJDIR)/,$(SRC_TESTES:.c=.o))

all: programa-principal

programa-principal: $(OBJ_COMMON) $(OBJ_PRINCIPAL)
	$(CC) $^ $(LDLIBS) -o $@

programa-interativo: $(OBJ_COMMON) $(OBJ_INTERATIVO)
	$(CC) $^ $(LDLIBS) -o $@

programa-testes: $(OBJ_COMMON) $(OBJ_TESTES)
	$(CC) $^ $(LDLIBS) -o $@

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(OBJ_COMMON:.o=.d) $(OBJ_PRINCIPAL:.o=.d) $(OBJ_INTERATIVO:.o=.d) $(OBJ_TESTES:.o=.d)

clean:
	rm -rf build programa-principal programa-interativo programa-testes resultados/*.txt

.PHONY: all clean
