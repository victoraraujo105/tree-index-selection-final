#ifndef CONSTS_HPP
#define CONSTS_HPP

// Macros de parâmetros constantes.

#define GEN_DIR "./generated/"
#define PAGE_SIZE 12

#define FILE 0
#define TERMINAL 1
// O macro INPUT_SRC determina de onde vem a entrada do programa.
// Se INPUT_SRC == FILE, ela será lida de um arquivo <in.txt> na raiz do projeto.
// Se INPUT_SRC == TERMINAL, ela será obtida por interações com o usuário na linha de comando. 
#define INPUT_SRC FILE

#define MAX_CHILDREN 10     // quantidade máxima de filhos de um nó interno.
#define HEADER_WIDTH 100    // Comprimento do cabeçalho do arquivo de índices.
                            // Deve ser longo o suficiente de modo a comportar os campos que serão inseridos no cabeçalho.
#define LINE_WIDTH (100*(MAX_CHILDREN) + 10) // Comprimento das linhas do arquivo de índices que representam os nós da árvore.
                                            // Deve ser grande o suficiente de modo a comportar os campos de qualquer nó.
#endif