#ifndef FILE_HPP
#define FILE_HPP

#include <string>
// Wrapper para classes de fluxo de arquivos da biblioteca padrão (fstream, ifstream, ostream).
// Estende essas classes genericamente com métodos úteis para o programa.
template <class F>
class File: public F
{
public:
    // Construtores devem ser redefinidos, pois não são herdados:
    File(): F() {}
    File(const std::string& filename): F(filename) {}
    File(const std::string& filename, std::ios_base::openmode mode): F(filename, mode) {}

    // Posiciona (retrocede) cursores de leitura e gravação no início do arquivo.
    void rewind()
    {
        this->clear();  // limpa flags de erro, necessário quando se chega ao final do arquivo, pois nesse caso a flag eofbit é ligada
        this->seekg(0, std::ios_base::beg); // posiciona cursor de leitura no começo
        this->seekp(0, std::ios_base::beg); // retrocede cursor de gravação
    }

    // Posiciona cursores de leitura e gravação no final do arquivo.
    void append()
    {
        this->clear();
        this->seekg(0, std::ios_base::end);
        this->seekp(0, std::ios_base::end);
    }

    // Lê arquivo a partir da linha especificada numa string até encontrar separador (não incluso, por padrão é a quebra de linha).
    void getline(const size_t& pos, std::string& line, const char& sep = '\n')
    {
        this->clear();
        this->seekg(pos);
        std::getline(*this, line, sep);
    }
};
#endif