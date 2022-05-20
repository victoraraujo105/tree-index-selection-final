#ifndef INTERNAL_NODE_HPP
#define INTERNAL_NODE_HPP

#include <cstdio>
#include <array>
#include <sstream>

#include "key.hpp"
#include "misc.hpp"


template <size_t NK, size_t NP> // <NK> e <NP> são, resp., os números máximos de chaves e de filhos do nó;
                    // Embora o número máximo de filhos seja exatamente um a mais que o de chaves,
                    // a escolha independente desses limites foi adotada por uma questão de compatibilidade com a superclasse Node.
struct InternalNode
{
    size_t& m;    // ocupação atual do nó (número de chaves, 0 <= m <= NK)
    std::array<Key, NK>& keys;  // chaves do nó
    std::array<size_t, NP>& ptrs;  // ponteiros (endereço da linha) aos filhos
    
    // construtor
    InternalNode(size_t& m, std::array<Key, NK>& keys, std::array<size_t, NP>& ptrs):
        m(m), keys(keys), ptrs(ptrs)
    {}

    // Lê os atributos de um fluxo de string dispostos segundo a formatação convencionada.
    InternalNode& operator << (std::stringstream& ss)
    {
        ss >> m;

        int i;
        for (i = 0; i < m; i++)
        {
            ss >> ptrs[i];
            ss >> keys[i];
        }

        ss >> ptrs[i];  // há um ponteiro a mais do que chaves

        return *this;
    }

    // Insere atributos a partir de campos formatados de fluxo de string.
    template <size_t _NK, size_t _NP>
    inline friend std::stringstream& operator >> (std::stringstream& ss, InternalNode<_NK, _NP>& node)
    {
        node << ss;
        return ss;
    }

    // Retorna ponteiro ao filho correspondente a uma chave especificada.
    size_t get_ptr(const Key& k)
    {
        if (m == 0) return 0;
        auto p = binary_search(k, keys, m);

        if (keys[p] <= k) p++;
        return ptrs[p];
    }

    // Retorna ponteiro ao primeiro filho que possa conter a chave de busca, ignorando a chave primária desambiguadora.
    // Considere o seguinte trecho de nó interno como exemplo:
    // N1 (1985, 6) N2 (1986, 2) N3 (1986, 5) N4 (1986, 9) N5
    // Logo, a chamada get_ptr_weak(1986) retornará o ponteiro N2,
    // pois este admite chaves K, tais que (1985, 6) <= K < (1986, 2)
    // e portanto pode existir alguma chave, digamos (1986, 1),
    // com ano_colheita igual a 1986 neste nó.
    size_t get_ptr_weak(const size_t& ano_colheita)
    {
        if (m == 0) return 0;

        auto p = search_key(ano_colheita, keys, m);
        while (p > 0 && Key::weak_comparator(keys[p], ano_colheita) >= 0) p--;
        if (Key::weak_comparator(keys[p], ano_colheita) < 0) p++;
        return ptrs[p];
    }

    // Retorna ponteiro ao primeiro filho que admita chave de busca superior à especificada. 
    size_t get_ptr_next(const size_t& ano_colheita)
    {
        if (m == 0) return 0;

        auto p = search_key(ano_colheita, keys, m);
        while (p < m && Key::weak_comparator(keys[p], ano_colheita) <= 0) p++;

        return ptrs[p];
    }

    // Insere chave e filho com chaves maiores ou iguais à chave inserida (à direita).
    // Caso o nó atual esteja cheio, ele é particionado ao meio, a chave é inserida
    // no nó apropriado (de modo que a parte direita tenha chaves estritamente maiores)
    // e o nó direito gerado é armazenado num parâmetro de referência.
    // Retorna verdadeiro sse não houve particionamento.
    bool insert(const Key& k, const size_t& ptr, InternalNode<NK, NP>& overflow_sibling)
    {
        auto p = binary_search(k, keys, m);
        if (keys[p] == k) return true;
        
        if (m == NK)
        {
            m /= 2;

            Key removed_key = keys[m];
        
            std::copy(keys.begin() + m + 1, keys.end(), overflow_sibling.keys.begin());
            std::copy(ptrs.begin() + m + 1, ptrs.end(), overflow_sibling.ptrs.begin());
            overflow_sibling.m = NK - m - 1;
            if (k < removed_key) insert(k, ptr, overflow_sibling);
            else overflow_sibling.insert(k, ptr, *this);
            keys[m] = removed_key;  // sempre que um nó interno é dividido, a chave central removida
                                    // é colocada após a última chave "visível" (utilizada nas buscas)
                                    // pois ao inserir o nó de overflow na ávore usamos a chave removida
                                    // como divisora de intervalo
            return false;
        }

        if (keys[p] < k) p++;

        for (int i = m; i > p; i--)
        {
            keys[i] = keys[i-1];
            ptrs[i+1] = ptrs[i];
        }

        keys[p] = k;
        ptrs[p+1] = ptr;

        m++;

        return true;
    }

    // Escreve atributos formatados em fluxo de saída. 
    template <typename OS, size_t _NK, size_t _NP>
    friend OS& operator << (OS& os, const InternalNode<_NK, _NP>& node)
    {
        os << node.m << ' ';
        int i;
        for (i = 0; i < node.m; i++)
        {
            os << node.ptrs[i] << ' ' << node.keys[i] << ' ';
        }
        
        return os << node.ptrs[i];
    }
};

#endif