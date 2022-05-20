#ifndef TABELA_HPP
#define TABELA_HPP

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <array>
#include <iomanip>
#include <map>
#include <filesystem>
#include <cstdio>
#include <memory>
#include <functional>

// only for debugging:
#include <iostream>

#include "consts.hpp"

class BPlusTree;
template <size_t N> class Operador;

void csv_parser(std::string entry, std::vector<std::string>& fields);

bool isspace(const std::string& str);

using Scheme = std::vector<std::string>;
using Fields = std::map<std::string, std::string>;
using Indices = std::map<std::string, std::shared_ptr<BPlusTree>>; 

class Tuple
{
private:
    const Scheme& scheme;
public:
    Fields fields;
    Tuple(const Scheme& scheme): scheme(scheme) {}

    template <typename IS>
    Tuple& operator << (IS& csv_file)
    {
        std::string cur_line, field;
        std::getline(csv_file, cur_line);
        std::stringstream csv_stream(cur_line);

        for (auto& field_name: scheme)
        {
            std::getline(csv_stream, fields[field_name], ',');
        }
        
        return *this;
    }

    template <typename OS>
    friend OS& operator << (OS& os, const Tuple& tuple)
    {
        std::stringstream ss;
        bool first = true;
        for (const auto& field_name: tuple.scheme)
        {
            if (first)
            {
                first = false;
            } else {
                ss << ",";
            }
            ss << tuple[field_name];
        }
        os << ss.str();
        return os; 
    }

    const std::string& operator [] (const std::string& key) const
    {
        return fields.at(key);
    }

};

template <typename T>
class PageSystem;

class TupleIterator;

// pág reutilizável associada a tabela
template <class T>
class Page
{
private:
    std::vector<T> tuples;
    size_t occupancy;
    std::stringstream content;

    friend PageSystem<T>;
    friend TupleIterator;
    friend BPlusTree;
    template <size_t N>
    friend class Operador;

    Page(const std::function<T()>& constructor = [](){ return T(); }): tuples(PAGE_SIZE, constructor()), occupancy(0) {update_header();}

    inline bool full() { return occupancy == PAGE_SIZE; }

    void append()
    {
        content.clear();
        content.seekp(0, std::ios::end);
    }

    void clear()
    {
        occupancy = 0;
        std::stringstream().swap(content);
        update_header();
    }

    bool load(const std::string& path)
    {
        std::ifstream file(path);
        if (!file) return false;
        std::stringstream().swap(content);
        content << file.rdbuf();
        file.close();
        content >> occupancy;
        return true;
    }

    // assumes the file is a valid page 
    void load_tuples() {
        content.clear();
        content.seekg(21, std::ios::beg);
        for (size_t i = 0; i < occupancy; i++)
        {
            tuples.at(i) << content;
        }
    }

    void update_content()
    {
        clear();
        occupancy = tuples.size();
        update_header();
        for (const auto& tuple: tuples)
        {
            content << tuple;
        }
    }
    void update_header()
    {
        content.clear();
        content.seekp(0, std::ios::beg);
        content << std::left << std::setw(20) << occupancy << '\n';
    }

    bool save(const std::string& path)
    {
        std::ofstream file(path);
        if (!file) return false;
        // std::cout << content.str() << '\n';
        // content.flush();
        // std::cout << content.rdbuf() << '\n';
        file << content.str();
        return true;
    }

    inline T& operator [] (size_t i)
    {
        return tuples.at(i);
    }

    inline const T& operator [] (size_t i) const
    {
        return tuples.at(i);
    }

    // como a operação será tipicamente feita repetidas vezes em sequência,
    // delegamos ao chamador a responsabilidade de colocar o cursor no fim
    // do buffer (chamando append()) bem como a de verificar se há espaço
    // disponível na página
    Page& operator << (const std::string& entry)
    {
        occupancy++;
        content << entry << '\n';
        return *this;
    }

    inline auto begin() const -> decltype(tuples)
    {
        return tuples.begin();
    }

    inline auto end() const -> decltype(tuples)
    {
        return tuples.end();
    }

};

template <typename T>
class PageSystem
{
private:
    const std::string directory, header_directory;
    size_t occupancy, index;
    Page<T> buffer_page;
    std::fstream header;

    friend TupleIterator;
    friend BPlusTree;
    
    template <size_t N>
    friend class Operador;

    inline std::string page_dir(const size_t& i)
    {
        return directory + std::to_string(i);
    }

    inline std::string page_dir() { return page_dir(index); }


public:
    PageSystem(const std::string& directory, size_t cur_page = 0, const std::function<T()>& newTuple = [](){return T();}):
        directory(directory), index(cur_page), buffer_page(newTuple), header_directory(directory + "total"), header(header_directory)
    {
        if (header)
        {
            std::string line;
            std::getline(header, line);
            std::stringstream line_stream(line);
            line_stream >> occupancy;
            // if (index >= occupancy)
            // {
            //     std::cout << "Índice de página out of bounds!\n";
            //     index = occupancy - 1;
            // }
            load_page();
        }
        else {
            std::filesystem::create_directories(directory);
            header.clear();
            header.open(header_directory, std::ios::in | std::ios::out | std::ios::trunc);
            occupancy = 1;
            index = 0;
            save_page();
            update_header();
        }
    }

    PageSystem(const std::string& directory, const std::function<T()>& newTuple):
        PageSystem(directory, 0, newTuple)
    {}

    PageSystem& operator << (const std::string& entry)
    {
        if (buffer_page.full())
        {
            occupancy++;
            update_header();
            save_page();
            buffer_page.clear();
            // buffer_page.append();
            index++;
        }

        buffer_page << entry;
        // std::cout << entry << '\n';
        return *this;
    }

    bool load_page(size_t index)
    {
        this->index = index;
        return buffer_page.load(page_dir());
    }

    inline bool load_page()
    {
        return load_page(index);
    }

    inline bool rewind()
    {
        return load_page(0);
    }

    inline bool append()
    {
        return load_page(occupancy - 1);
    }

    void copy_to(std::fstream& out)
    {
        do {
            auto buffer = std::move(buffer_page.content.rdbuf());
            buffer->pubseekoff(21, std::ios::beg);
            out << buffer;
            index++;
        } while (load_page());
    }

    void load_tuples()
    {
        buffer_page.load_tuples();
    }

    // efetuar alterações após edição
    void save_tuples()
    {
        buffer_page.update_content();
    }

    void save_page()
    {
        buffer_page.update_header();
        buffer_page.save(page_dir());
    }

    void update_header()
    {
        header.clear();
        header.seekp(0, std::ios::beg);
        header << std::left << std::setw(20) << occupancy << '\n';
    }

    const Tuple& operator [] (size_t i) const { return buffer_page[i]; }

    inline const size_t& get_occupancy() const { return occupancy; }

    // inline void clear()
    // {
    //     create_file();
    // }
};

using TablePageSystem = PageSystem<Tuple>;

class TupleIterator
{
private:
    size_t cur_tuple_index, cur_page_index;
    TablePageSystem cur_page;

    friend BPlusTree;

    bool load_page()
    {
        cur_tuple_index = 0;
        if (!cur_page.load_page(cur_page_index)) return false;
        cur_page.load_tuples();
        return true;
    }

    bool load_page(const size_t& page_index)
    {
        cur_page_index = page_index;
        return load_page();
    }

public:
    TupleIterator(const std::string& directory, const std::function<Tuple()> newTuple, size_t page = 0):
        cur_page(directory, page, newTuple), cur_page_index(page), cur_tuple_index(0)
    {
        load_page();
    }

    TupleIterator& operator ++ (int)
    {   
        if (++cur_tuple_index >= cur_page.buffer_page.occupancy)
        {
            cur_page_index++;
            load_page();
        }

        return *this;
    }

    const std::string& operator [] (const std::string& key)
    {
        return cur_page[cur_tuple_index][key];
    }

    bool reached_end()
    {
        return cur_page.occupancy == cur_page_index;
    }

    const Tuple& operator * () const
    {
        return cur_page[cur_tuple_index];
    }
};

class Tabela
{
private:
    std::ifstream dataset;
    std::string directory, data_directory, result_directory;
    Scheme scheme;
    Indices indices;
    size_t num_pages;
    std::function<Tuple()> newTuple;

    friend BPlusTree;
    template <size_t N> friend class Operador;

    // Tuple newTuple() const { return Tuple(scheme); }
    // PageSystem newPage(const size_t& index = 0) const { return PageSystem(directory, scheme, index); }

public:
    Tabela(std::string dataset_path);

    void carregarDados();

    TablePageSystem get_page(const size_t& index = 0) const;

    TupleIterator get_tuple_iterator() const;

    TupleIterator operator [] (size_t i) const;

    inline BPlusTree& operator [] (const std::string& field_name) const
    {
        return *indices.at(field_name);
    }

    // inline BPlusTree& operator [] (const std::vector<std::string>& A)
    // {
    //     // for (const auto& a: A) std::cout << a << '\n';
    //     return *indices.at(A[0]);
    // }

};


#endif