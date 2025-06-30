#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "terminal.h"


int main(){
    const char* arquivo_entrada = "in.txt";
    
    no* raiz = construir_arvore_do_arquivo(arquivo_entrada);
    
    if (raiz != NULL) {
        iniciar_terminal(raiz, arquivo_entrada);
        
        printf("Liberando memoria...\n");
        liberar_no_recursivo(raiz);
        printf("Memoria liberada com sucesso!\n");
    } else {
        printf("Erro ao construir a árvore.\n");
        return 1;
    }
    
    return 0;
}