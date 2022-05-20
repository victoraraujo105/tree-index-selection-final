#ifndef MISC_HPP
#define MISC_HPP

#include <cstdio>
#include <array>
#include <vector>
#include "key.hpp"

// Define funções utilitárias banais.

// Retorna índice de algum termo da sequência ordenada especificada idêntico à chave de busca, não necessariamente o primeiro. 
// Supõe-se n maior que zero, a fim de evitar underflow com o tipo size_t que é unsigned.
template <typename T, size_t size>
size_t binary_search(const T& key, const std::array<T, size>& seq, const size_t& n = size)
{
    size_t i = 0, j = n - 1, c;

    while (i < j)
    {
        c = (i+j)/2;
        if (seq[c] == key) return c;
        if (seq[c] < key) i = c+1;
        else j = c;
    }

    return i;
}

// Pesquisa por chave apenas por ano de colheita, ignorando a chave primária.
// Não necessariamente retorna índice da primeira ocorrência.
template <size_t size>
size_t search_key(const size_t& ano_colheita, const std::array<Key, size>& keys, const size_t& n = size)
{
    size_t i = 0, j = n - 1, c;

    while (i < j)
    {
        c = (i+j)/2;
        auto comp = Key::weak_comparator(keys[c], ano_colheita);
        if (comp == 0) return c;
        if (comp < 0) i = c+1;
        else j = c;
    }

    return i;
}

#endif