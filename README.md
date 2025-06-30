

-----

# Simulador de Terminal em C

Este projeto é uma implementação de um simulador de terminal de linha de comando em C. Ele gerencia uma estrutura de arquivos e pastas em memória usando uma árvore n-ária e permite que o usuário interaja com essa estrutura por meio de comandos comuns do Unix, como `ls`, `cd`, `mkdir`, `rm`, entre outros. As alterações na estrutura de arquivos podem ser salvas em um arquivo de texto.

## Conceito Central: A Estrutura de Árvore

O núcleo do simulador é a representação do sistema de arquivos como uma árvore. Cada nó (`no`) na árvore pode ser um arquivo (`ARQUIVO`) ou uma pasta (`PASTA`). A árvore é implementada usando a representação "primeiro filho, próximo irmão" (first child, next sibling).

  - `no* primeiroFilho`: Ponteiro para o primeiro filho direto do nó.
  - `no* proxIrmao`: Ponteiro para o próximo nó no mesmo nível hierárquico (um "irmão").
  - `no* pai`: Ponteiro para o nó pai, essencial para comandos como `cd ..`.

Essa estrutura permite que um nó (pasta) tenha um número variável de filhos sem a necessidade de um array de ponteiros de tamanho dinâmico.

```c
typedef enum {
    PASTA,
    ARQUIVO
} tipoNo;

typedef struct no {
    char* nome;
    tipoNo tipo;
    struct no* pai;
    struct no* primeiroFilho;
    struct no* proxIrmao;
} no;
```

-----

## Principais Funções e Lógica

A seguir, uma explicação detalhada das funções mais importantes que impulsionam o simulador.

### 1\. Construção da Árvore a Partir de um Arquivo

A inicialização do sistema de arquivos é feita lendo um arquivo de texto onde cada linha representa um caminho completo.

#### `no* construir_arvore_do_arquivo(const char* nome_arquivo)`

**Propósito:** Ler um arquivo de texto contendo caminhos e construir a estrutura de árvore em memória correspondente.

**Como funciona:**

1.  **Leitura do Arquivo:** A função primeiro chama `ler_arquivo` para carregar todo o conteúdo do arquivo de entrada para uma única string na memória.
2.  **Criação da Raiz:** Cria o nó raiz da árvore, que representa o diretório `/`.
3.  **Processamento Linha a Linha:** Itera sobre a string lida, tratando cada linha como um caminho completo (ex: `documentos/trabalhos/relatorio.docx`).
4.  **Delegação:** Para cada linha (caminho), ela chama a função `processar_caminho` para que os nós correspondentes sejam criados e inseridos na árvore.
5.  **Limpeza:** Após o processamento de todas as linhas, a string que continha o conteúdo do arquivo é liberada da memória.

#### `void processar_caminho(no *raiz, char *caminho)`

**Propósito:** Analisar um caminho (ex: `fotos/ferias/praia.jpg`) e criar os nós de pasta e/ou arquivo correspondentes na árvore.

**Como funciona:**

1.  **Tokenização:** Utiliza a função `strtok` para dividir a string do caminho em "tokens" usando `/` como delimitador. Para `fotos/ferias/praia.jpg`, os tokens seriam `fotos`, `ferias` e `praia.jpg`.
2.  **Navegação e Criação:** Começando pelo nó raiz, a função processa cada token:
      * Ela verifica se já existe um filho com o nome do token no nó atual.
      * **Se existe:** Ela navega para esse nó filho e continua o processo com o próximo token.
      * **Se não existe:** Ela cria um novo nó chamando `criar_no`. O tipo do nó (`PASTA` ou `ARQUIVO`) é determinado pela presença de um `.` no nome do token. O novo nó é então adicionado à lista de filhos do nó atual.
3.  **Construção Incremental:** Esse processo garante que a estrutura de diretórios seja construída de forma incremental e que caminhos compartilhados (como `/documentos/pessoal` e `/documentos/trabalho`) reutilizem os mesmos nós pais.

### 2\. O Loop Principal do Terminal

O coração interativo do programa, onde o usuário insere comandos.

#### `void iniciar_terminal(no* raiz, const char* arquivo_entrada)`

**Propósito:** Gerenciar o ciclo de vida da sessão do terminal: exibir o prompt, ler comandos do usuário e processá-los até que o comando `exit` seja inserido.

**Como funciona:**

1.  **Loop Infinito:** Entra em um laço `while(1)`.
2.  **Exibição do Prompt:** A cada iteração, chama `obter_caminho_completo` para obter a string do diretório atual e a exibe no formato ` caminho/atual$  `.
3.  **Leitura do Comando:** Usa `fgets` para ler a linha de comando inserida pelo usuário.
4.  **Comando `exit`:** Se o comando for `exit`, a função aciona a rotina de salvamento (`salvar_arvore_no_arquivo`) e encerra o loop, finalizando o programa.
5.  **Delegação de Comando:** Para qualquer outro comando, ela chama `processar_comando` para executar a ação correspondente.

#### `void processar_comando(char* comando, no** diretorio_atual, no* raiz)`

**Propósito:** Atuar como um "roteador" ou "dispatcher". Ele analisa o comando bruto do usuário e chama a função específica que implementa esse comando.

**Como funciona:**

1.  **Separação:** Usa `strtok` para separar o comando (a primeira palavra, ex: `cd`) de seus argumentos (o resto da linha, ex: `documentos`).
2.  **Seleção:** Utiliza uma cadeia de `if-else if` para comparar a string do comando com os nomes dos comandos disponíveis (`ls`, `cd`, `mkdir`, etc.).
3.  **Execução:** Ao encontrar uma correspondência, chama a função `comando_*` apropriada (ex: `comando_cd`), passando os argumentos e o ponteiro para o diretório atual (`diretorio_atual`). O ponteiro para o diretório atual é passado por referência (`no**`) porque funções como `cd` precisam modificá-lo.

### 3\. Execução de Comandos

Exemplos de como os comandos individuais são implementados.

#### `void comando_cd(char* caminho, no** diretorio_atual, no* raiz)`

**Propósito:** Alterar o diretório de trabalho atual.

**Como funciona:**

  * **Casos Especiais:** Primeiro, verifica casos como `cd ..` (navega para o `pai`), `cd /` (navega para a `raiz`) ou `cd` sem argumentos (também vai para a raiz).
  * **Busca no Diretório:** Para um nome de diretório, ele chama `encontrar_diretorio` para procurar um filho do diretório atual que seja uma pasta com o nome correspondente.
  * **Atualização:** Se um diretório válido for encontrado, o ponteiro `*diretorio_atual` é atualizado para apontar para esse novo nó.
  * **Feedback:** Se o diretório não for encontrado, uma mensagem de erro é exibida, sugerindo nomes similares, se houver (`mostrar_alternativas`).

#### `void comando_mkdir(char* nome_pasta, no* diretorio_atual)`

**Propósito:** Criar uma nova pasta (nó do tipo `PASTA`) no diretório atual.

**Como funciona:**

1.  **Validação:** Realiza várias verificações no nome fornecido: se não é nulo, se não contém caracteres inválidos (`/`, `\`, `*`, etc.) e se já não existe um arquivo ou pasta com o mesmo nome.
2.  **Criação do Nó:** Se a validação for bem-sucedida, chama `criar_no` para alocar memória e inicializar um novo nó com `tipo = PASTA`.
3.  **Inserção na Árvore:** Adiciona o novo nó à lista de filhos do diretório atual. Ele é inserido como `primeiroFilho` se a lista estiver vazia, ou como `proxIrmao` do último filho existente.

### 4\. Salvando as Alterações

Para garantir a persistência dos dados entre as sessões, a estrutura da árvore em memória pode ser salva de volta no arquivo de texto.

#### `void salvar_arvore_no_arquivo(no* raiz, const char* nome_arquivo)`

**Propósito:** Escrever a estrutura da árvore de volta em um arquivo de texto, onde cada nó corresponde a uma linha com seu caminho completo.

**Como funciona:**

1.  **Abertura do Arquivo:** Abre o arquivo de saída em modo de escrita (`"w"`), o que apaga qualquer conteúdo anterior.
2.  **Chamada Recursiva:** Inicia o processo de escrita chamando a função auxiliar `escrever_caminhos_recursivo`, começando pela raiz.

#### `void escrever_caminhos_recursivo(no* no_atual, char* caminho_atual, FILE* arquivo)`

**Propósito:** Percorrer a árvore recursivamente e escrever o caminho completo de cada nó no arquivo.

**Como funciona:**

1.  **Travessia Pre-Order:** A função opera em uma lógica de travessia pré-ordem.
2.  **Construção do Caminho:** Para cada nó visitado, ela constrói a string do seu caminho completo.
3.  **Escrita no Arquivo:** `fprintf` é usado para escrever o caminho completo do nó atual no arquivo, seguido por uma quebra de linha `\n`.
4.  **Recursão:** A função então se chama recursivamente para todos os filhos do nó atual, passando o caminho atualizado para que os filhos possam construir seus próprios caminhos completos.

### Considerações Adicionais

  - **Funções `*_custom`:** O código implementa suas próprias versões de funções de string insensíveis a maiúsculas e minúsculas (`strcasecmp_custom`, `stristr_custom`). Isso é feito para garantir a portabilidade e evitar a dependência da extensão `_POSIX_C_SOURCE` ou de bibliotecas não padrão em todos os compiladores.
  - **Gerenciamento de Memória:** O código utiliza `malloc` e `strdup` para alocar memória dinamicamente. A função `liberar_no_recursivo` é crucial para liberar a memória de uma subárvore (usada em `rmdir`), e todas as alocações são cuidadosamente gerenciadas para evitar vazamentos de memória (memory leaks).
