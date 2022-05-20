#ifndef LEAF_NODE
#define LEAF_NODE

#include <cstdio>
#include <array>
#include <sstream>
#include "key.hpp"
#include "misc.hpp"

template <size_t NK> // <NK> e <NP> são, resp., os números máximos de chaves e de ponteiros do nó;
                    // Embora o número máximo de ponts. seja exatamente um a mais que o de chaves,
                    // a escolha independente desses limites foi adotada por uma questão de compatibilidade com a superclasse Node.
struct LeafNode
{
    size_t& m;
    std::array<Key, NK>& keys;

    size_t &l, &r;

    LeafNode(size_t& m, std::array<Key, NK>& keys, size_t& l, size_t& r)
        : m(m), keys(keys), l(l), r(r)
    {}

    LeafNode<NK>& operator << (std::stringstream& ss)
    {
        ss >> l >> r >> m;

        for (int i = 0; i < m; i++)
        {
            ss >> keys[i];
        }

        return *this;
    }

    template <size_t _NK>
    inline friend std::stringstream& operator >> (std::stringstream& ss, LeafNode<_NK>& node)
    {
        node << ss;
        return ss;
    }

    // retorna posição apropriada à inserção da chave k,
    // de modo que todas e apenas as chaves menores que k antecedam a posição escolhida.
    // Logo, se nenhuma chave dessa folha for menor que k, k deve ser inserida na primeira posição;
    // já no caso em que k é maior que todas as chaves, a posição retornada é m (supõe-se que a folha não está cheia).
    size_t find_pos(const Key& k)
    {
        if (m == 0) return 0;
        auto p = binary_search(k, keys, m);
        if (keys[p] < k) p++;
        return p;
    }

    // Atribui ao parâmetro de referência p a posição da primeira chave com campo de busca ano_colheita, caso exista.
    // Retorna verdadeiro sse há alguma chave com tal campo de busca na folha.   
    bool get_pos_weak(const size_t& ano_colheita, size_t& p)
    {
        if (m == 0) return false;

        p = search_key(ano_colheita, keys, m);
    
        while (p > 0 && Key::weak_comparator(keys[p], ano_colheita) == 0) p--;
        if (Key::weak_comparator(keys[p], ano_colheita) < 0) p++;
        return (p < m && Key::weak_comparator(keys[p], ano_colheita) == 0);
    }

    // Atribui ao parâmetro de referência p a posição da primeira chave com campo de busca superior a ano_colheita, caso exista.
    // Retorna verdadeiro sse há alguma chave que satisfaça tal propriedade na folha.   
    bool get_pos_next(const size_t& ano_colheita, size_t& p)
    {
        // Caso a folha esteja vazia ou a última chave (máxima, pois estão ordenadas)
        // seja menor ou igual a ano_colheita, então nenhuma chave na folha satisfaz a propriedade.
        // Portanto, a posição da próxima chave é m (após a última) nesse caso.
        if (m == 0 || Key::weak_comparator(keys[m-1], ano_colheita) <= 0)
        {
            p = m;
            return false;
        }

        p = search_key(ano_colheita, keys, m);
        // Como aqui há pelo menos uma chave maior que ano colheita, não há necessidade de verificar se p < m. 
        while (Key::weak_comparator(keys[p], ano_colheita) <= 0) p++;
        return true;
    }

    // Insere chave e endereço do registro correspondente.
    // Caso o nó atual esteja cheio, ele é particionado ao meio, a chave é inserida
    // no nó apropriado (de modo que a parte direita tenha chaves estritamente maiores)
    // e o nó direito gerado é armazenado num parâmetro de referência.
    // Retorna verdadeiro sse a chave foi inserida e não houve particionamento.
    // Em particular, quando a chave já está na folha (e não é inserida), o atributo m (número de chaves)
    // do nó de overflow recebe 0, para distinguir-se do caso em que houve particionamento.
    bool insert(const Key& k, LeafNode<NK>& overflow_sibling)
    {
        auto p = find_pos(k);
        if (p < m && keys[p] == k)
        {
            overflow_sibling.m = 0;
            return false;
        }

        if (m == NK)
        {
            split(overflow_sibling);

            //supõe-se m > 0
            if (k < keys[m-1]) insert(k, overflow_sibling);
            else overflow_sibling.insert(k, *this);

            return false;
        }

        // Deslocamos as chaves (e seus ponteiros correspondentes)
        // à direita da posição correta da chave a ser inserida
        // de modo a abrir espaço para a inserção.
        // A ordenação das chaves é preservada.
        for (size_t i = m; i > p; i--)
        {
            keys[i] = keys[i-1];
        }

        keys[p] = k;
        m++;

        return true;
    }

    // Particiona nó ao meio, coloca metade direita em <sibling>.
    void split(LeafNode<NK>& sibling)
    {
        m /= 2;

        sibling.m = NK - m;
        std::copy(keys.begin() + m, keys.end(), sibling.keys.begin());
    }

private:
    // Remove chave na posição p sem atestar validade do índice.
    // Logo, se não usada com cuidado (por isso o acesso privado)
    // pode acarretar underflow (quando p == 0).
    void _remove(const size_t& p)
    {
        m--;
        for (int i = p; i < m; i++)
        {
            keys[i] = keys[i+1];
        }
    }

public:
    // Remove chave na posição p atestando previamente a validade do índice.
    void remove(const size_t& p)
    {
        if (m == 0) return;
        _remove(p);
    }

    // Remove chave k, caso exista.
    // O valor booleano retornado indica se a chave foi removida.
    bool remove(const Key& k)
    {
        auto p = find_pos(k);
        if (m == 0 || keys[p] != k) return false;
        _remove(p);
        return true;
    }

private:
    // Remove <removed> chaves a partir do índice first.
    // Não verifica validade dos parâmetros, pois é de uso interno apenas.
    void _remove_weak(const size_t& first, const size_t& removed)
    {
        m -= removed;

        for (int i = first; i < m; i++)
        {
            keys[i] = keys[i + removed];
        }
    }

public:
    // Remove as chaves entre os índices first e last (exclusive).
    // Garante que esses índices estejam dentro da sequência de chaves.
    void remove_weak(const size_t& first, const size_t& last)
    {
        if (first >= last) return;
        first = m < first? m: first;
        last = m < last? m: last;

        auto removed = last - first;

        _remove_weak(first, removed);
    }

    // Remove todas as chaves iguais a ano_colheita.
    // Retorna o número de chaves removidas.
    size_t remove_weak(const size_t& ano_colheita)
    {
        size_t first;
        get_pos_weak(ano_colheita, first);
        if (m == 0 || Key::weak_comparator(keys[first], ano_colheita) != 0) return 0;
        size_t last = first + 1;
        while (last < m && Key::weak_comparator(keys[last], ano_colheita) == 0) last++;

        size_t removed = last - first;
        _remove_weak(first, removed);
        return removed;
    }

    // Escreve campos do nó formatados em fluxo de saída.
    template <typename OS>
    friend OS& operator << (OS& os, const LeafNode<NK>& node)
    {
        os << node.l << ' ' << node.r << ' ' << node.m << ' ';
        int i;
        for (i = 0; i < node.m; i++)
        {
            os << node.keys[i] << ' ';
        }

        return os;
    }
};

#endif