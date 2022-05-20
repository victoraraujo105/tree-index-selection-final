#include <iostream>
#include <string>

// VOCE DEVE INCLUIR SEUS HEADERS AQUI, CASO NECESSARIO!!!
#include "include/tabela.hpp"
#include "include/operador.hpp"

using namespace std;

int main() {
    std::filesystem::remove_all(GEN_DIR);
    Tabela vinho {"vinho.csv"}; // cria estrutura necessaria para a tabela
    // Tabela uva {"uva.csv"};
    // Tabela pais {"pais.csv"};

    vinho.carregarDados(); // le os dados do csv e add na estrutura da tabela, caso necessario
    // uva.carregarDados();
    // pais.carregarDados();

    //// DESCOMENTE A PROXIMA LINHA CASO SEU TRABALHO SEJA SELECAO:
    // Operador op {vinho, {"ano_producao", "pais_producao_id"}, {"1996", "0"}};
    Operador op {vinho, {"ano_producao", "pais_producao_id"}, {"1996", "0"}};
    //// significa: SELECT * FROM Vinho WHERE ano_colheita = '1990' AND uva_id = '0'
    //// IMPORTANTE: isso eh so um exemplo, pode ser outra tabela e ter mais ou menos colunas/constantes.
    //// genericamente: Operador(tabela, lista_colunas, lista_constantes): 
    //// significa: SELECT * FROM tabela WHERE col_1 = con_1 AND col_2 = con_2 AND ... AND col_n = con_n

    //// DESCOMENTE A PROXIMA LINHA CASO SEU TRABALHO SEJA PROJECAO:
    // Operador op {vinho, new string[2] {"uva_id", "rotulo"}};
    //// significa: SELECT uva_id, rotulo FROM Vinho
    //// IMPORTANTE: isso eh so um exemplo, pode ser outra tabela e ter mais ou menos colunas.
    //// genericamente: Operador(tabela, lista_colunas_proj):
    //// significa: SELECT col_1, col_2, ..., col_n FROM tabela

    //// DESCOMENTE A PROXIMA LINHA CASO SEU TRABALHO SEJA JUNCAO:
    // Operador op {vinho, uva, "vinho_id", "uva_id"};
    //// significa: SELECT * FROM Vinho V, Uva U WHERE V.vinho_id = U.uva_id
    //// IMPORTANTE: isso eh so um exemplo, pode ser tabelas/colunas distintas.
    //// genericamente: Operador(tabela_1, tabela_2, col_tab_1, col_tab_2):
    //// significa: SELECT * FROM tabela_1, tabela_2 WHERE col_tab_1 = col_tab_2
    op.executar(); // Realiza a operacao desejada

    cout << "#Pags: " << op.numPagsGeradas(); // Retorna a quantidade de paginas geradas pela operacao
    cout << "\n#IOss: " << op.numIOExecutados(); // Retorna a quantidade de IOs geradas pela operacao
    cout << "\n#Tups: " << op.numTuplasGeradas(); // Retorna a quantidade de tuplas geradas pela operacao
    cout << "\n";

    op.salvarTuplasGeradas("selecao_vinho_ano_colheita_1990.csv"); // Retorna as tuplas geradas pela operacao e salva em um csv

    return 0;
}