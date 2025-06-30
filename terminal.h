typedef enum{
    PASTA,
    ARQUIVO
}tipoNo;

typedef struct no{
    char* nome;
    tipoNo tipo;
    struct no* primeiroFilho;
    struct no* proxIrmao;
    struct no* pai;
}no;

typedef no* Arvore;

no* criar_no(const char* nome, tipoNo tipo, no* pai);
void processar_caminho(no *raiz, char *caminho);
void imprimir_arvore(no *raiz, int nivel);

char* ler_arquivo(const char *nome_ficheiro);
no* construir_arvore_do_arquivo(const char* nome_arquivo);
void salvar_arvore_no_arquivo(no* raiz, const char* nome_arquivo);
void escrever_caminhos_recursivo(no* no_atual, char* caminho_atual, FILE* arquivo);

void iniciar_terminal(no* raiz, const char* arquivo_entrada);
void processar_comando(char* comando, no** diretorio_atual, no* raiz);

void comando_ls(no* diretorio_atual);
void comando_cd(char* caminho, no** diretorio_atual, no* raiz);
void comando_pwd(no* diretorio_atual);
void comando_tree(no* diretorio_atual);
void comando_search(char* nome, no* raiz);
void comando_rm(char* nome_arquivo, no* diretorio_atual);
void comando_rmdir(char* nome_pasta, no* diretorio_atual);
void comando_mkdir(char* nome_pasta, no* diretorio_atual);
void comando_touch(char* nome_arquivo, no* diretorio_atual);
void comando_clear();
void comando_help();

no* encontrar_diretorio(no* base, char* nome);
char* obter_caminho_completo(no* no_atual);
void mostrar_alternativas(no* diretorio_atual, char* nome_parcial);
void buscar_recursivo(no* no_atual, char* nome_procurado, int* contador);
void liberar_no_recursivo(no* no_para_remover);
int strcasecmp_custom(const char* s1, const char* s2);
int strncasecmp_custom(const char* s1, const char* s2, size_t n);
char* stristr_custom(const char* str, const char* substr);