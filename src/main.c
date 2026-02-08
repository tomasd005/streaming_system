#include "main.h"

#include "gestor_programa/gestor_programa.h"
#include "gestor_programa/gestor_queries.h"

int executar_programa_principal(const char *dataset_path, const char *input_file) {
    int rc = 0;
    gestor_programa_t *gp = gestor_programa_criar();
    if (!gp) return 1;
    if (gestor_programa_carregar_dataset(gp, dataset_path) != 0) rc = 1;
    else if (gestor_queries_processar_ficheiro(gp, input_file) != 0) rc = 1;
    gestor_programa_destruir(gp);
    return rc;
}
