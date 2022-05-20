#include "include/tabela.hpp"
#include "include/b_plus_tree.hpp"

void csv_parser(std::string entry, std::vector<std::string>& fields)
{
    std::stringstream line_stream(entry);
    std::string field;
    while (getline(line_stream, field, ',')) fields.push_back(std::move(field));
}

bool isspace(const std::string& str)
{
    return str.find_first_not_of(" \t\n\v\f\r") == std::string::npos;
}

Tabela::Tabela(std::string dataset_path):
    directory(GEN_DIR + dataset_path.substr(dataset_path.find_last_of("/\\") + 1,  dataset_path.find_last_of(".")) + "/"),
    data_directory(directory + "data/"),
    result_directory(directory + "results/"),
    dataset(dataset_path),
    num_pages(0)
{
    std::filesystem::create_directories(data_directory);
    std::filesystem::create_directories(result_directory);
    std::ofstream scheme_file(directory + "scheme")/*, pages(directory + "pages")*/;
    
    std::string header;
    std::getline(dataset, header);
    scheme_file << header << '\n';
    // pages << std::left << std::setw(20) << num_pages << '\n';
    // std::cout << "directory: " << directory << ".\n";
    csv_parser(header, scheme);
    // for (auto& field: scheme) std::cout << "field: " << field << '\n';
    // std::cout << scheme.size() << '\n';
    newTuple = [this](){ return Tuple(scheme); };
}

void Tabela::carregarDados()
{
    std::string record;
    TablePageSystem page(data_directory, newTuple);

    while (!dataset.eof())
    {
        std::getline(dataset, record);
        if (isspace(record)) break;
        page << record;
    }

    page.update_header();
    page.save_page();

    for (auto& field_name: scheme)
    {
        indices[field_name] = std::make_shared<BPlusTree>(*this, field_name);
    }
}

TablePageSystem Tabela::get_page(const size_t& index) const
{
    return PageSystem(data_directory, index, newTuple);
}

TupleIterator Tabela::get_tuple_iterator() const
{
    return (*this)[0];
}

TupleIterator Tabela::operator [] (size_t i) const
{
    return TupleIterator(data_directory, newTuple, i);
}
