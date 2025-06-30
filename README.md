# Guia de Comandos e Funções da Interface do Terminal

Este documento serve como uma referência técnica para os comandos disponíveis no simulador de terminal. Para cada comando, é explicado seu propósito, a função em C que o implementa e quais funções auxiliares são invocadas para completar sua execução.

## Funções Principais da Interface

Antes dos comandos individuais, é importante entender as duas funções que gerenciam o ciclo de vida da interface de linha de comando (CLI).

### `void iniciar_terminal(no* raiz, const char* arquivo_entrada)`

Esta é a função que dá início e mantém a sessão do terminal ativa.

* **Lógica:** Opera em um loop infinito (`while (1)`) que executa os seguintes passos:
    1.  Exibe o prompt de comando, que inclui o caminho do diretório atual.
    2.  Lê o comando inserido pelo usuário via `fgets`.
    3.  Verifica se o comando é `exit`. Em caso afirmativo, salva o estado da árvore de arquivos e encerra o loop.
    4.  Para qualquer outro comando, delega a execução para a função `processar_comando`.
* **Funções Auxiliares Utilizadas:**
    * `obter_caminho_completo()`: Para gerar a string do caminho atual exibida no prompt.
    * `fgets()`: Para ler a entrada do usuário de forma segura.
    * `processar_comando()`: Para rotear o comando lido para a função de implementação correta.
    * `salvar_arvore_no_arquivo()`: Chamada especificamente quando o comando `exit` é inserido, para persistir os dados.

### `void processar_comando(char* comando, no** diretorio_atual, no* raiz)`

Atua como um roteador (ou *dispatcher*). Sua única função é analisar a string de comando e chamar a função `comando_*` correspondente.

* **Lógica:**
    1.  Usa `strtok` para separar a primeira palavra (o comando) do restante da string (os argumentos).
    2.  Usa uma série de comparações (`if-else if`) para determinar qual função específica deve ser chamada com base no comando.
* **Funções Auxiliares Utilizadas:**
    * `strtok()`: Para dividir a entrada do usuário em comando e argumentos.
    * `strcmp()`: Para comparar o comando com a lista de comandos disponíveis.

---

## Catálogo de Comandos

A seguir, a lista de todos os comandos implementados, suas funções e dependências.

### `ls`

* **Descrição:** Lista os arquivos e pastas contidos no diretório atual.
* **Função de Implementação:** `void comando_ls(no* diretorio_atual)`
* **Lógica:** Percorre a lista de filhos do `diretorio_atual` (iniciando em `primeiroFilho` e seguindo os ponteiros `proxIrmao`), imprimindo o nome de cada nó e indicando se é um diretório (`[DIR]`) ou arquivo (`[FILE]`).
* **Funções Auxiliares Utilizadas:** Nenhuma função auxiliar complexa é chamada, apenas operações diretas nos ponteiros do nó.

### `cd <dir>`

* **Descrição:** Altera o diretório de trabalho atual para o especificado. Aceita `..` para voltar ao diretório pai e `/` para ir à raiz.
* **Função de Implementação:** `void comando_cd(char* caminho, no** diretorio_atual, no* raiz)`
* **Lógica:** Trata os casos especiais (`..` e `/`) e, para outros caminhos, busca por um subdiretório com o nome correspondente. Se não encontrar, sugere alternativas.
* **Funções Auxiliares Utilizadas:**
    * `encontrar_diretorio()`: Para procurar um subdiretório com o nome especificado no diretório atual.
    * `mostrar_alternativas()`: Chamada se `encontrar_diretorio` falhar, para sugerir pastas com nomes similares.
    * `strcasecmp_custom()`: Usada dentro de `encontrar_diretorio` para a busca de diretório sem diferenciar maiúsculas/minúsculas.

### `pwd`

* **Descrição:** Exibe o caminho completo do diretório de trabalho atual.
* **Função de Implementação:** `void comando_pwd(no* diretorio_atual)`
* **Lógica:** Simplesmente invoca a função que constrói a string do caminho e a imprime.
* **Funções Auxiliares Utilizadas:**
    * `obter_caminho_completo()`: Para gerar a string do caminho completo a partir do nó do diretório atual.

### `tree`

* **Descrição:** Exibe a estrutura de arquivos e pastas de forma hierárquica (árvore) a partir do diretório atual.
* **Função de Implementação:** `void comando_tree(no* diretorio_atual)`
* **Lógica:** Inicia a impressão da árvore, chamando a função recursiva responsável pela exibição.
* **Funções Auxiliares Utilizadas:**
    * `imprimir_arvore()`: Função recursiva que percorre a árvore a partir do `diretorio_atual` e imprime cada nó com a indentação correta.

### `search <nome>`

* **Descrição:** Busca por arquivos ou pastas cujo nome contenha o termo de busca, a partir da raiz (`/`). A busca não diferencia maiúsculas de minúsculas.
* **Função de Implementação:** `void comando_search(char* nome, no* raiz)`
* **Lógica:** Prepara a busca e chama a função recursiva que percorre toda a árvore.
* **Funções Auxiliares Utilizadas:**
    * `buscar_recursivo()`: Função recursiva que efetivamente percorre toda a árvore, verificando cada nó.
    * `stristr_custom()`: Usada por `buscar_recursivo` para verificar se o nome do nó contém o termo de busca de forma insensível a maiúsculas/minúsculas.
    * `obter_caminho_completo()`: Usada por `buscar_recursivo` para imprimir o caminho completo de cada resultado encontrado.

### `mkdir <pasta>`

* **Descrição:** Cria uma nova pasta no diretório atual.
* **Função de Implementação:** `void comando_mkdir(char* nome_pasta, no* diretorio_atual)`
* **Lógica:** Valida o nome da pasta (contra caracteres inválidos e nomes existentes) e, se válido, cria um novo nó do tipo `PASTA` e o anexa à lista de filhos do diretório atual.
* **Funções Auxiliares Utilizadas:**
    * `strcasecmp_custom()`: Para verificar se já existe um arquivo ou pasta com o mesmo nome.
    * `criar_no()`: Para alocar e inicializar a estrutura do novo nó.

### `touch <arquivo>`

* **Descrição:** Cria um novo arquivo vazio no diretório atual.
* **Função de Implementação:** `void comando_touch(char* nome_arquivo, no* diretorio_atual)`
* **Lógica:** Semelhante ao `mkdir`, mas cria um nó do tipo `ARQUIVO`. Valida o nome e verifica se já existe um item com o mesmo nome antes da criação.
* **Funções Auxiliares Utilizadas:**
    * `strcasecmp_custom()`: Para verificar se já existe um arquivo ou pasta com o mesmo nome.
    * `criar_no()`: Para alocar e inicializar a estrutura do novo nó.

### `rm <arquivo>`

* **Descrição:** Remove um arquivo do diretório atual. Não funciona para pastas.
* **Função de Implementação:** `void comando_rm(char* nome_arquivo, no* diretorio_atual)`
* **Lógica:** Procura por um filho com o nome especificado. Se encontrar e for do tipo `ARQUIVO`, ele ajusta os ponteiros da lista de irmãos para "pular" o nó a ser removido e, em seguida, libera a memória do nó.
* **Funções Auxiliares Utilizadas:**
    * `strcasecmp_custom()`: Para encontrar o arquivo a ser removido.
    * `free()`: Para liberar a memória do nó e de seu nome.

### `rmdir <pasta>`

* **Descrição:** Remove uma pasta (e todo o seu conteúdo) do diretório atual.
* **Função de Implementação:** `void comando_rmdir(char* nome_pasta, no* diretorio_atual)`
* **Lógica:** Procura por um filho com o nome especificado. Se encontrar e for do tipo `PASTA`, remove o nó da lista de irmãos e chama uma função recursiva para liberar a memória de todos os seus descendentes.
* **Funções Auxiliares Utilizadas:**
    * `strcasecmp_custom()`: Para encontrar a pasta a ser removida.
    * `liberar_no_recursivo()`: Função recursiva crucial que navega por toda a sub-árvore da pasta a ser removida, liberando cada nó de baixo para cima para evitar vazamentos de memória.

### `clear`

* **Descrição:** Limpa a tela do terminal.
* **Função de Implementação:** `void comando_clear()`
* **Lógica:** Imprime códigos de escape ANSI (`\033[2J\033[H`) que são interpretados pela maioria dos emuladores de terminal como um comando para limpar a tela e mover o cursor para o topo.
* **Funções Auxiliares Utilizadas:** Nenhuma.

### `help`

* **Descrição:** Exibe uma lista de todos os comandos disponíveis e uma breve descrição de cada um.
* **Função de Implementação:** `void comando_help()`
* **Lógica:** Imprime um bloco de texto estático com as informações de ajuda.
* **Funções Auxiliares Utilizadas:** Nenhuma.

### `exit`

* **Descrição:** Encerra a sessão do terminal e salva todas as alterações no arquivo de entrada.
* **Função de Implementação:** A lógica é tratada diretamente dentro de `iniciar_terminal`.
* **Lógica:** Ao detectar o comando "exit", o loop principal em `iniciar_terminal` é interrompido. Antes de sair, a função de salvamento é chamada.
* **Funções Auxiliares Utilizadas:**
    * `salvar_arvore_no_arquivo()`: Para escrever a estrutura de árvore atual de volta para o arquivo, garantindo a persistência dos dados.
