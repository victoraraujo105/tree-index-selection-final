#include "include/b_plus_tree.hpp"
#include "include/tabela.hpp"

size_t BPlusTree::get_code(const std::string& search_key)
{
    std::string code_file_path = codes_directory /*+ "k-"*/ + search_key;
    std::fstream code_file(code_file_path);
    size_t code;
    if (!code_file)
    {
        code_file.clear();
        code_file.open(code_file_path, std::ios::in | std::ios::out | std::ios::trunc);
        code_file << unique_keys << '\n';
        // std::fstream key_file(codes_directory + std::to_string(unique_keys), std::ios::in | std::ios::out | std::ios::trunc);
        // key_file << search_key << '\n';
        code = unique_keys++;
    } else {
        code_file >> code;
    }
    
    return code;
}

std::string BPlusTree::search_dir(const std::string& search_key) { return table.result_directory + this->search_key + "=" + search_key + "/"; };

TablePageSystem BPlusTree::newResultPage(const std::string& search_key)
{
    return PageSystem(search_dir(search_key), table.newTuple);
}

BPlusTree::BPlusTree(const Tabela& table, const std::string& search_key):
    table(table),
    directory(table.directory + "trees/" + search_key + "/"),
    codes_directory(directory + "codes/"),
    search_key(search_key),
    root(HEADER_WIDTH+1),
    depth(0),
    unique_keys(0)
{
    std::filesystem::create_directories(codes_directory);
    indices.open(directory + "tree", std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
    update_header();    // escreve cabeçalho
    indices << root;    // escreve raiz
    insert_all();
}

// BPlusTree::BPlusTree(BPlusTree&& tree):
//     indices(std::move(tree.indices)),
//     table(std::move(table)),
//     directory(std::move(directory)),
//     search_key(std::move(search_key)),
//     depth(std::move(depth)),
//     root(std::move(root))
// { }

void BPlusTree::update_header()
{
    std::stringstream header;
    header << root.pos << ' ' << depth;
    indices.rewind();
    indices << std::left << std::setw(HEADER_WIDTH) << std::setfill(' ') << header.str() << '\n';
}

void BPlusTree::insert_all()
{

    // não basta apenas a chave de busca, pois desejamos uma único registro por chave, sem repetições,
    // a fim de evitar a violação das invariantes da árvore B+.

    // percorremos o arquivo de dados, linha a linha, a partir do primeiro registro
    // inserindo na árvore uma chave única para cada registro
    for (auto iter = table.get_tuple_iterator(); !iter.reached_end(); iter++)
    {
        insert(Key(get_code(iter[search_key]), iter.cur_page_index));
    }
}

bool BPlusTree::insert(const Key&k, const size_t& add)
{
    set_leaf(k);    // faz com que o nó auxiliar buffer[0] seja o nó correto para a inserção de k
                    // a propriedades da árvore B+ garantem que esse nó correto seja único

    auto& cur_node = buffer[0]; // nó atual, no qual tentaremos inserir a chave
    auto& overflow_node = buffer[1];    // nó de overflow, será criado caso o nó atual esteja lotado
    auto& parent_node = buffer[2];      // nó pai do atual, no qual devemos tentar inserir o nó de overflow criado

    // tentamos inserir a chave no nó atual
    if (!cur_node.insert(k, add, overflow_node))
    {
        // aqui entramos caso o nó não tenha sido inserido sem overflow
        if (overflow_node.m == 0)
        {
            // esse é o caso em que a chave já está na folha
            // não fazemos nada
            return false;
        }

        // neste ponto, é certo que a folha foi partida e o nó de overflow criado

        // indices.flush();
        indices.append();   // posicionamos o cursor no fim do arquivo de índices, pois é lá que inseriremos o nó de overflow
        overflow_node.pos = indices.tellp();    // a posição do nó de overflow é a do cursor

        // o nó de overflow vai a direita do nó partido, portanto, precisamos atualizar os ponteiros esquerdo e direito de acordo
        overflow_node.l = cur_node.pos;     // o nó esquerdo ao de overflow é o atual
        overflow_node.r = cur_node.r;       // o nó à direita, o antigo sucessor do atual
        cur_node.r = overflow_node.pos;     // o nó de overflow é o novo sucessor do atual

        // se o nó de overflow possui sucessor, devemos atualizar o ponteiro esquerdo desse nó
        if (overflow_node.r != 0)
        {
            // TODO: não há necessidade aqui de carregar (parse) todos os campos do nó direito ao de overflow,
            // pois só queremos sobrescrever o valor no campo l.
            // um modo de fazer isso de modo mais eficiente é fixando o comprimento dos campos e
            // sobrescrever diretamente o campo que desejamos modificar
            
            
            auto& right = parent_node;
            set_node(overflow_node.r, right);   // carregamos o sucessor no nó right
            right.l = overflow_node.pos;    // atualizamos o ponteiro esquerdo
            indices.seekp(right.pos);   // posicionamos o cursor no endereço do nó direito
            indices << right;           // a fim de sobrescrever e atualizar a linha correspondente no arquivo de índices
        }

        bool inserted;  // indica se o último nó de overflow criado foi inserido na árvore
                        // isso só é falso quando particionamos a raiz, visto que ela não possui pai
                        // e, portanto, para inserir o nó de overflow, devemos criar uma nova raiz
        Key min = overflow_node.min();  // tomamos como divisor de intervalo a menor chave do nó de overflow
                                        // pois devemos garantir que os nós à esquerda sejam estritamente menores
                                        // e os sucessores maiores ou iguais

        // se o nó atual não é a raiz, devemos tentar inserir o nó de overflow no seu pai
        if (cur_node.pos != root.pos)
        {
            // podemos atualizar nó atual no arquivo de índices, pois seus campos já não serão alterados 
            indices.seekp(cur_node.pos);
            indices << cur_node;
            // indices.flush();

            parent_node.pos = cur_node.parent;  // precisamos da posição do pai para carregá-lo

            // vamos inserindo os eventuais nós de overflow gerados a cada iteração
            // até que atinjamos a raiz ou encontremos um nó com espaço disponível
            while (true)
            {
                set_node(parent_node.pos, parent_node); // carregamos o nó pai

                // TODO: remover os flushes pra ver o que acontece, testar com insert all
                // indices.flush();
                indices.append();   // vamos ao fim do arquivo
                cur_node = overflow_node;   // tomamos como nó atual o nó de overflow
                cur_node.pos = indices.tellp(); //
                overflow_node.pos = cur_node.pos + LINE_WIDTH + 1;

                inserted = parent_node.insert(min, cur_node.pos, overflow_node);

                cur_node.parent = parent_node.pos;
                indices << cur_node;

                if (parent_node.m < parent_node.keys.size()) min = parent_node.keys[parent_node.m];

                if (!inserted)
                {
                    auto& child = cur_node;

                    for (int i = 0; i <= overflow_node.m; i++)
                    {
                        set_node(overflow_node.ptrs[i], child);
                        child.parent = overflow_node.pos;
                        indices.clear();
                        indices.seekp(child.pos);
                        indices << child;
                    }

                    indices.clear();
                    indices.seekp(overflow_node.pos);
                
                    if (parent_node.pos == root.pos)
                    {
                        cur_node = parent_node;
                        break;
                    }
                }

                indices.seekp(parent_node.pos);
                indices << parent_node;
                // indices.flush();

                if (inserted) break;

                parent_node.pos = parent_node.parent;
            }

            if (parent_node.pos == root.pos) root = parent_node;
        }

        if (!inserted)
        {
            auto& left = cur_node;
            auto& right = overflow_node;
            auto& new_root = root;

            new_root.pos = right.pos + (LINE_WIDTH + 1);
            left.parent = right.parent = new_root.pos;

            depth++;

            new_root.leaf = false;
            new_root.parent = 0;
            new_root.m = 1;
            new_root.keys[0] = min;
            new_root.ptrs[0] = left.pos; new_root.ptrs[1] = right.pos;

            // indices.flush();
            indices.seekp(right.pos);
            indices << right;
            indices << new_root;
            // indices.flush();
            indices.seekp(left.pos);
            indices << left;
            // indices.flush();
            update_header();
            // indices.flush();
        }
    } else {
        indices.seekp(cur_node.pos);
        // indices.flush();
        indices << cur_node;
        // indices.flush();
        if (cur_node.pos == root.pos) root = cur_node;
    }

    return true;
}

TupleIterator BPlusTree::get_tuple_iterator(const std::string& search_key)
{
    select(search_key);
    TupleIterator iter(search_dir(search_key), table.newTuple);

    if (iter.reached_end())
    {
        select(search_key);
        iter.load_page();
    }
    
    return iter;
}

TupleIterator BPlusTree::operator [] (const std::string& search_key) { return get_tuple_iterator(search_key); }

size_t BPlusTree::select(const std::string& search_key)
{
    size_t code = get_code(search_key);
    set_leaf_weak(code);
    auto& cur_node = buffer[0];
    size_t p, count = 0;
    if (((LN) cur_node).get_pos_weak(code, p))
    {
        PageSystem result_page = newResultPage(search_key);
        PageSystem table_page = table.get_page();
        while (true)
        {
            if (p < cur_node.m)
            {
                if (cur_node.keys[p].search_key != code) break;
                table_page.load_page(cur_node.keys[p].page);
                table_page.load_tuples();
    
                for (size_t i = 0; i < table_page.buffer_page.occupancy; i++)
                {
                    const auto& cur_tuple = table_page[i]; 
                    if (cur_tuple[this->search_key] == search_key)
                        result_page << cur_tuple;
                }
                count++;
                p++;
                continue;
            }
            
            if (cur_node.r == 0) break;
            set_node(cur_node.r, cur_node);
            p = 0;
        }
        result_page.update_header();
        result_page.save_page();
    }

    return count;
}

size_t BPlusTree::set_leaf(const Key& k, size_t node_id)
{
    auto& cur_node = buffer[node_id];
    cur_node = root;

    while (!cur_node.leaf)
    {
        set_node(((IN) cur_node).get_ptr(k), cur_node);
    }

    return cur_node.pos;
}

std::array<size_t, 2> BPlusTree::set_leaf_weak(const size_t& code, const size_t& node_id)
{
    auto& cur_node = buffer[node_id];
    cur_node = root;
    
    size_t ios = 0;

    while (!cur_node.leaf)
    {
        set_node(((IN) cur_node).get_ptr_weak(code), cur_node);
        ios++;
    }

    while (cur_node.r != 0 && (cur_node.m == 0 || cur_node.max().search_key < code))
    {
        set_node(cur_node.r, cur_node);
        ios++;
    }

    return {cur_node.pos, ios};
}

void BPlusTree::set_node(const size_t& pos, Node& out)
{
    out.pos = pos;
    std::string line;
    indices.getline(pos, line);
    out << line;
}