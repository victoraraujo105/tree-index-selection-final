#ifndef KEY_HPP
#define KEY_HPP
#include <string>

// Chave da estrutura de índices.
struct Key
{
    size_t search_key;    // Chave de busca
    size_t page;    // Chave primária, utilizada para diferenciar entre chaves com mesma chave de busca
                        // é a solução adotada pelo livro para o problema das chaves duplicadas.

    // Construtores:
    Key(const size_t& search_key, const size_t& page):
        search_key(search_key), page(page)
    {}
Key(const size_t& search_key): Key(search_key, 0) {}
    Key(): Key(0) {}

    // Operadores de comparação que definem ordem total sobre essas chaves.
    inline bool operator == (const Key& k) const
    {
        return search_key == k.search_key && page == k.page;
    }

    inline bool operator != (const Key& k) const
    {
        return !(*this == k);
    }

    inline bool operator < (const Key& k) const
    {
        return search_key < k.search_key || (search_key == k.search_key && page < k.page);
    }

    inline bool operator > (const Key& k) const
    {
        return k < *this;
    }

    inline bool operator <= (const Key& k) const
    {
        return !(k < *this);
    }

    inline bool operator >= (const Key& k) const
    {
        return k <= *this;
    }

    // Operador de inserção em fluxos de saída (como arquivos de texto).
    // Formata a chave com campos separados por espaço, de acordo
    // com a sintaxe convencionada para o arquivo de índices.
    template <typename OS>
    inline friend OS& operator << (OS& os, const Key& k)
    {
        return os << k.search_key << ' ' << k.page;
    }

    // Operador de extração de fluxos de entrada
    // (interpreta automaticamente campos textuais como inteiros).
    // Geralmente operado em std::stringstream.
    template <typename IS>
    inline friend IS& operator >> (IS& is, Key& k)
    {
        is >> k.search_key;
        is >> k.page;
        return is;
    }

    // Comparador fraco (em constraste aos comparadores fortes definidos acima).
    // Compara apenas a chave de busca, necessário para as operações de busca do enunciado.
    static inline int weak_comparator(const Key& k, const size_t& search_key)
    {
        if (k.search_key < search_key) return -1;
        if (k.search_key == search_key) return 0;
        return 1;
    }
};
#endif