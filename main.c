#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "terminal.h"


int main(){
    // Construir a árvore a partir do arquivo
    no* raiz = construir_arvore_do_arquivo("in.txt");
    
    if (raiz != NULL) {
        // Iniciar o simulador de terminal
        iniciar_terminal(raiz);
        
        // Liberar toda a memória alocada antes de terminar
        printf("Liberando memoria...\n");
        liberar_no_recursivo(raiz);
        printf("Memoria liberada com sucesso!\n");
    } else {
        printf("Erro ao construir a árvore.\n");
        return 1;
    }
    
    return 0;
}