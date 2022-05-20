#ifndef OPERADOR_HPP
#define OPERADOR_HPP

#include "b_plus_tree.hpp"
#include "csv.hpp"
#include "stats.hpp"

template <size_t N>
class Operador
{
private:
    Tabela& table;
    const std::string keys[N], values[N];
    size_t matching_index;
    Stats stats;
public:
    Operador(Tabela& table, const std::string (&keys)[N], const std::string (&values)[N])
        : table(table), keys(keys), values(values), stats(0, 0, 0)
    { 
        matching_index = 0;
        size_t best_unique = table.indices[keys[0]]->unique_keys;

        for (size_t i = 1; i < N; i++)
        {
            size_t cur_unique = table.indices[keys[i]]->unique_keys;
            if (cur_unique > best_unique) 
            {
                matching_index = i;
                best_unique = cur_unique; 
            }
        }
    }

    void executar()
    {
        stats = table.indices[keys[matching_index]]->select(matching_index, keys, values);
    }

    void salvarTuplasGeradas(const std::string& path)
    {
        auto& matching_key = keys[matching_index];
        BPlusTree& matching_tree = *(table.indices[matching_key]);
        std::fstream out(path, std::ios::in | std::ios::out | std::ios::trunc);
        out << csv(table.scheme) << '\n';
        PageSystem result_page(matching_tree.search_dir(keys, values), table.newTuple);
        result_page.copy_to(out);
        const auto& occ = result_page.occupancy;
        if (occ > 0)
        {
            stats.ios += occ;
            result_page.append();
            stats.tuples += (occ - 1)*PAGE_SIZE + result_page.buffer_page.occupancy;
        }
    }

    inline size_t numPagsGeradas()
    {
        return stats.pags;
    }

    inline size_t numIOExecutados()
    {
        return stats.ios;
    }    

    inline size_t numTuplasGeradas()
    {
        return stats.tuples;
    }
};

#endif // OPERADOR_HPP