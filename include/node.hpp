#ifndef NODE_HPP
#define NODE_HPP

#include <array>
#include <string>
#include <sstream>

#include "key.hpp"
#include "internal_node.hpp"
#include "leaf_node.hpp"
#include "consts.hpp"

// Os nós da árvore são armazenados no arquivo de texto
// generated/indices.ssv (space separated values)
// como sequências de campos separados por espaço.

// A primeira linha indica a posição da raiz e a profundidade da árvore.

// Cada nó ocupa uma única linha de tamanho fixo.

// As folhas obedecem à sintaxe:
// L <PARENT_ADD> <LEFT_ADD> <RIGHT_ADD> <NUM_OF_KEYS> <K_1> <ADD_1> ... <K_<NUM_OF_KEYS>> <ADD_<NUM_OF_KEYS>>  

// Onde PARENT_ADD, LEFT_ADD e RIGHT_ADD são, respectivamente,
// os endereços (posição do cursor no começo da linha correspondente) 
// dos nós pai, esquerdo e direito dessa folha.

// Por convenção, na ausência de algum desses "parentes" (pai, antecessor ou sucessor)
// coloca-se 0 nesses campos. Logo, tem-se por consequência imediata que
// o único nó com pai nulo é a raiz da árvore.

// Já os campos <K_i>, <ADD_i>, denotam, respectivamente, uma chave armazenada
// e o endereço do registro associado no arquivo de dados vinhos.csv

// Os demais nós, aqui chamados de internos, seguem, por sua vez, a seguinte sintaxe:
// N <PARENT_ADD> <NUM_OF_KEYS> <ADD_0> <K_1> <ADD_1> ... <K_<NUM_OF_KEYS>> <ADD_<NUM_OF_KEYS>> 

// Onde <K_i>, <ADD_i-1> e <ADD_i>, representam, respectivamente
// a i-ésima chave e os endereços do nó de chaves estritamente menores
// e do de chaves maiores ou iguais a K_i.

// As chaves dos nós são mantidas em ordem crescente.

// Inicialmente, a árvore possui como único nó e raiz a seguinte linha:
// L 0 0 0 0

// Apelidos para as classes base do nó genérico.
using IN = InternalNode<MAX_CHILDREN - 1, MAX_CHILDREN>;
using LN = LeafNode<MAX_CHILDREN - 1>;

struct Node: public IN, public LN
{
    bool leaf;  // flag que indica se o nó é folha ou não
    size_t parent, l, r;    // endereços do pai deste nó e (caso seja folha) dos nós antecessor e sucessor
    size_t m;   // quantidade atual de chaves, que naturalmente pode variar ao longo das inserções e remoções da árvore
    size_t pos;    // endereço do nó no arquivo de índices
    std::array<Key, MAX_CHILDREN - 1> keys; // chaves, mantidas em ordem crescente
    std::array<size_t, MAX_CHILDREN> ptrs;  // ponteiros (endereços de filhos, para nós internos ou endereços de registros, para folhas)

    // Construtor padrão monta uma folha vazia com ponteiros nulos.
    // Caso não especificada, a posição do nó será nula.
    Node(const size_t& pos = 0): IN(m, keys, ptrs), LN(m, keys, l, r), m(0), parent(0), l(0), r(0), leaf(true), pos(pos)
    {
        std::fill(ptrs.begin(), ptrs.end(), 0); // preenche vetor de ponteiros com zeros.
    }

    // Sobrecarga do operator de atribuição.
    // Tão somente copia o nó atribuído a este atributo por atributo.
    // Os ponteiros l e r são copiados apenas de folhas.
    Node& operator = (const Node& n)
    {
        leaf = n.leaf;
        parent = n.parent;
        if (leaf)
        {
            l = n.l;
            r = n.r;
        }
        m = n.m;
        pos = n.pos;
        std::copy(n.keys.begin(), n.keys.end(), keys.begin());
        std::copy(n.ptrs.begin(), n.ptrs.end(), ptrs.begin());
        return *this;
    }

    bool full() { return m == MAX_CHILDREN - 1; }   // indica se o nó está cheio

    // As seguintes funções devem ser chamadas apenas em nós não vazios (m > 0).
    const Key& min() const { return keys[0]; }  // menor chave do nó
    const Key& max() const { return keys[m == 0? 0: (m-1)]; }   // maior chave do nó

    // Insere uma chave e seu ponteiro correspondente neste nó.
    // Caso este nó seja partido, a metade direita será armazenada
    // no parâmetro de referência overflow_sibling.
    // Retorna verdadeiro sse o nó foi inserido sem overflow.
    bool insert(const Key& k, const size_t& ptr, Node& overflow_sibling)
    {
        overflow_sibling.leaf = leaf;
        if (leaf) return LN::insert(k, overflow_sibling);
        return IN::insert(k, ptr, overflow_sibling);
    }

    // Lê atributos de nó de uma linha com campos separados por espaços
    // seguindo a formatação descrita no início deste arquivo.
    inline Node& operator << (const std::string line)
    {
        std::stringstream ss(line);
        return *this << ss;
    }

    // Lê atributos de nó de uma strinstream com campos separados por espaços
    // seguindo a formatação descrita no início deste arquivo.
    inline Node& operator << (std::stringstream& ss)
    {
        char type;
        ss >> type;

        leaf = type == 'L';

        ss >> parent;

        if (leaf)
            (LN) *this << ss;
        else
            (IN) *this << ss;

        return *this;
    }

    // Insere os campos da stringstream nos atributos correspondentes do nó.
    inline friend std::stringstream& operator >> (std::stringstream& ss, Node& node)
    {
        node << ss;
        return ss;
    }

    // Insere no fluxo de saída a representação textual do nó segundo
    // a formatação detalhada no início deste arquivo.
    template <typename OS>
    inline friend OS& operator << (OS& os, const Node& node)
    {
        std::stringstream ss;

        if (node.leaf) ss << "L " << node.parent << ' ' << ((LN) node);
        else ss << "N " << node.parent << ' ' << ((IN) node);

        os << std::left << std::setw(LINE_WIDTH) << std::setfill(' ') << ss.str() << '\n';
        return os;
    }
};

#endif