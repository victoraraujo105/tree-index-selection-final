#ifndef B_PLUS_TREE
#define B_PLUS_TREE

#include <fstream>
#include <sstream>
#include <array>
#include <iomanip>
#include <iostream>
#include <cstdio>
#include <string>
#include <initializer_list>

#include "file.hpp"
#include "node.hpp"
#include "consts.hpp"
#include "tabela.hpp"
#include "Stats.hpp"


// class Tabela;
// class TupleIterator;
template <size_t N> class Operador;

class BPlusTree;

class NodeStorage
{
private:
    const std::string directory;
    size_t num_nodes, root_id;

    friend BPlusTree;

    std::string get_directory(const size_t& id)
    {
        return directory + std::to_string(id);
    }

    void save_node(const size_t& id, const Node& node)
    {
        std::fstream node_file("path");
        node_file << node;
    }

    void load_node(const size_t& id, Node& out)
    {
        std::ifstream node_file(directory + get_directory(id));
        // node_file >> out;
    }
public:
    NodeStorage(const std::string& directory): directory(directory), num_nodes(1), root_id(0)
    {

    }


};

class BPlusTree
{
private:
    File<std::fstream> indices;
    const Tabela& table;
    std::string directory, codes_directory, search_key;
    size_t depth, unique_keys;
    Node root;
    
    template <size_t N> friend class Operador;

    size_t get_code(const std::string& search_key);

    std::string search_dir(const std::string& search_key);

    TablePageSystem newResultPage(const std::string& search_key);

    
    std::array<Node, 3> buffer;     // São mantidos na memória 3 nós além da raiz.
                                    // São essenciais pois abstraem a manipulação do arquivo de índices,
                                    // sem a qual o código ficaria consideravelmente mais complicado e lento,
                                    // pois durante a inserção de uma chave na árvore, são utilizados até 3 nós:
                                    // o nó que recebe a chave, seu pai e um nó de "overflow".
                                    // Ao mesmo tempo, o uso de memória é irrisório: 3*((MAX_CHILDREN + 3)*sizeof(size_t) + sizeof(bool)),
                                    // o que não passa de 243 bytes para MAX_CHILDREN = 10.

                                    // TODO: Não carregar mais informações do que o necessário, sobrescrever diretamente campos no arquivo de índices.

    // Atualiza o cabeçalho do arquivo de índices
    // com o endereço da raiz e profundidade atuais.
    void update_header();

    // Insere todos os registros do arquivo de dados na árvore.
    void insert_all();

    // Insere uma chave associada ao registro add
    // Retorna falso sse a chave já pertence a árvore
    bool insert(const Key&k, const size_t& add = 0);
    

    // Carrega no buffer especificado (por padrão no primeiro)
    // a folha correta para a chave especificada.
    // A chave não necessariamente pertence a essa folha.
    // Caso não pertença, não pertence a nenhuma outra folha
    // e pode ser inserida nesta sem violar as propriedades da árvore.
    // As propriedades da árvore B+ garantem que essa folha seja única.
    size_t set_leaf(const Key& k, size_t node_id = 0);

    // Carrega no buffer especificado (por padrão no primeiro)
    // a primeira folha que possa conter uma chave com o
    // search_key especificado. É o ponto de partida
    // da busca por todas as chaves com um dado ano de colheita,
    // visto que as chaves formam uma lista encadeada ordenada.
    // As propriedades da árvore B+ garantem que essa folha seja única.
    std::array<size_t, 2> set_leaf_weak(const size_t& code, const size_t& node_id = 0);

    // Carrega o nó com a posição <pos> no arquivo de índices
    // na variável de referência out.
    // Também define seu atributo pos de acordo.
    void set_node(const size_t& pos, Node& out);

public:
    // Construtor único da árvore com parâmetros opcionais dos caminhos dos arquivos de dados e de índices.
    // Esses parâmetros recebem os valores padrãe abaixo.
    // A árvore sempre começa com uma raiz vazia e, portanto, profundidade zero.
    // A raiz inicial é sempre escrita na segunda linha do arquivo de índices,
    // após o cabeçalho, portanto, no endereço HEADER_WIDTH + 1. 
    // BPlusTree(const std::string& data_path = "vinhos.csv", const std::string& indices_path = "generated/indices.ssv")
    //     : indices(indices_path, std::ios_base::in | std::ios_base::out | std::ios_base::trunc), data(data_path, std::ios_base::in), root(HEADER_WIDTH+1), depth(0)
    // {
    //     update_header();    // escreve cabeçalho
    //     indices << root;    // escreve raiz
    // }

    BPlusTree(const Tabela& table, const std::string& search_key);

    BPlusTree(BPlusTree&& tree);
    
    // retorna profundidade atual da árvore
    size_t get_depth() { return depth; }

    TupleIterator get_tuple_iterator(const std::string& search_key);
    
    TupleIterator operator [] (const std::string& search_key);

    // retorna a quantidade de chaves na árvore com o search_key especificado
    // Caso as entradas sejam obtidas do terminal, também imprime todos os registros correspondentes.
    size_t select(const std::string& search_key);
    
    template <size_t N>
    std::string search_dir(const std::string (&keys)[N], const std::string (&values)[N])
    {
        std::stringstream ss;
        ss << table.result_directory;
        for (size_t i = 0; i < N; i++)
        {
            const auto& cur_key = keys[i];
            const auto& cur_val = values[i];
            if (i != 0) ss << ",";
            ss << cur_key << "=" << cur_val;
        }
        ss << "/";
        return ss.str();
    };

    template <size_t N>
    TablePageSystem newResultPage(const std::string (&keys)[N], const std::string (&values)[N])
    {
        auto page = PageSystem(search_dir(keys, values), table.newTuple);
        // page.clear();
        return page;
    }

    template <size_t N>
    bool matches_restriction(const Tuple& tuple, const std::string (&keys)[N], const std::string (&values)[N])
    {
        for (size_t i = 0; i < N; i++)
        {
            const auto& cur_key = keys[i];
            const auto& cur_val = values[i];
            if (tuple[cur_key] != cur_val) return false;
        }
        return true;
    }

    template <size_t N>
    Stats select(size_t matching_index, const std::string (&keys)[N], const std::string (&values)[N])
    {
        Stats stats(0, 0, 0);
        const auto& search_key = values[matching_index];
        size_t code = get_code(search_key);
        stats.ios++;
        stats.ios += set_leaf_weak(code)[1];
        auto& cur_node = buffer[0];
        size_t p;
        if (((LN) cur_node).get_pos_weak(code, p))
        {
            PageSystem result_page = newResultPage(keys, values);
            PageSystem table_page = table.get_page();
            while (true)
            {
                if (p < cur_node.m)
                {
                    if (cur_node.keys[p].search_key != code) break;
                    table_page.load_page(cur_node.keys[p].page);
                    stats.ios++;
                    table_page.load_tuples();
                    for (size_t i = 0; i < table_page.buffer_page.occupancy; i++)
                    {
                        const auto& tuple = table_page[i];
    
                        if (matches_restriction(tuple, keys, values))
                        {
                            result_page << tuple;
                            stats.tuples++;
                        }
                    }
                    p++;
                    continue;
                }
                
                if (cur_node.r == 0) break;
                set_node(cur_node.r, cur_node);
                stats.ios++;
                p = 0;
            }
            stats.pags = result_page.occupancy;
            result_page.update_header();
            result_page.save_page();
        }

        return stats;
    }

    template <size_t N>
    TupleIterator get_tuple_iterator(size_t matching_index, const std::string (&keys)[N], const std::string (&values)[N])
    {
        select(matching_index, keys, values);
        TupleIterator iter(search_dir(keys, values), table.scheme);

        if (iter.reached_end())
        {
            select(matching_index, keys, values);
            iter.load_page();
        }
        
        return iter;
    }
};

#endif