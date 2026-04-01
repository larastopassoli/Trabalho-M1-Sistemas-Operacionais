#include <iostream>
#include <fstream>  // leitura e escrita de arquivos
#include <vector>
#include <string>
#include <thread>   // uso de threads
#include <cctype>   // toupper()

using namespace std;

// Guarda uma posição da matriz (linha e coluna)
struct Posicao {
    int linha;
    int coluna;
};

// Guarda o resultado da busca de uma palavra
struct Resultado {
    string palavra;             // palavra buscada
    bool encontrada = false;    // indica se foi encontrada
    int linhaInicial = -1;      // linha onde começa
    int colunaInicial = -1;     // coluna onde começa
    string direcao = "";        // direção encontrada
    vector<Posicao> posicoes;   // posições das letras da palavra
};

// Representa uma direção de busca (movimento na matriz)
struct Direcao {
    int dl;       // deslocamento da linha
    int dc;       // deslocamento da coluna
    string nome;  // nome da direção
};

// Lista com as 8 direções possíveis
const vector<Direcao> direcoes = {
    {0, 1, "direita"},
    {0, -1, "esquerda"},
    {1, 0, "abaixo"},
    {-1, 0, "cima"},
    {1, 1, "baixo/direita"},
    {1, -1, "baixo/esquerda"},
    {-1, 1, "cima/direita"},
    {-1, -1, "cima/esquerda"}
};

// Lê o arquivo e separa matriz e palavras
bool lerArquivoEntrada(const string& nomeArquivo, int& linhas, int& colunas,
                       vector<string>& matriz, vector<string>& palavras) {
    ifstream entrada(nomeArquivo);

    // Verifica se conseguiu abrir o arquivo
    if (!entrada.is_open()) {
        return false;
    }

    // Lê o tamanho da matriz
    entrada >> linhas >> colunas;
    entrada.ignore();

    matriz.clear();
    palavras.clear();

    string linha;

    // Lê cada linha da matriz
    for (int i = 0; i < linhas; i++) {
        getline(entrada, linha);

        // Remove '\r' (problema comum em arquivos Windows)
        if (!linha.empty() && linha.back() == '\r') {
            linha.pop_back();
        }

        // Verifica se a linha tem o tamanho correto
        if ((int)linha.size() != colunas) {
            return false;
        }

        matriz.push_back(linha);
    }

    // Lê as palavras restantes do arquivo
    while (getline(entrada, linha)) {

        // Remove '\r' se existir
        if (!linha.empty() && linha.back() == '\r') {
            linha.pop_back();
        }

        // Ignora linhas vazias
        if (!linha.empty()) {
            palavras.push_back(linha);
        }
    }

    entrada.close();
    return true;
}

// Verifica se uma palavra existe a partir de uma posição e direção
bool buscarNaDirecao(const vector<string>& matriz, int linhas, int colunas,
                     const string& palavra, int lin, int col,
                     const Direcao& dir, vector<Posicao>& posicoesEncontradas) {
    
    // Limpa posições anteriores
    posicoesEncontradas.clear();

    // Percorre cada letra da palavra
    for (int i = 0; i < (int)palavra.size(); i++) {
        
        // Calcula a próxima posição na matriz
        int novaLin = lin + i * dir.dl;
        int novaCol = col + i * dir.dc;

        // Verifica se saiu da matriz
        if (novaLin < 0 || novaLin >= linhas || novaCol < 0 || novaCol >= colunas) {
            return false;
        }

        // Verifica se a letra da matriz é igual à da palavra
        if (matriz[novaLin][novaCol] != palavra[i]) {
            return false;
        }

        // Guarda a posição válida
        posicoesEncontradas.push_back({novaLin, novaCol});
    }

    // Se chegou aqui, todas as letras coincidem → palavra encontrada
    return true;
}

// Função executada por cada thread
// Cada thread recebe uma palavra e tenta encontrá-la na matriz
void procurarPalavra(const vector<string>& matriz, int linhas, int colunas,
                     const string& palavra, Resultado& resultado) {
    
    // Guarda qual palavra está sendo buscada
    resultado.palavra = palavra;

    // Percorre toda a matriz
    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunas; j++) {

            // Otimização: só testa se a primeira letra bater
            if (matriz[i][j] != palavra[0]) {
                continue;
            }

            // Testa a palavra em todas as direções
            for (const auto& dir : direcoes) {
                vector<Posicao> posicoesTemp;

                if (buscarNaDirecao(matriz, linhas, colunas, palavra, i, j, dir, posicoesTemp)) {
                    resultado.encontrada = true;
                    resultado.linhaInicial = i;
                    resultado.colunaInicial = j;
                    resultado.direcao = dir.nome;
                    resultado.posicoes = posicoesTemp;
                    return;
                }
            }
        }
    }
}

// Marca as palavras encontradas em maiúsculo
void marcarPalavrasMaiusculas(vector<string>& matrizSaida, const vector<Resultado>& resultados) {
    for (const auto& resultado : resultados) {
        if (resultado.encontrada) {
            for (const auto& pos : resultado.posicoes) {
                matrizSaida[pos.linha][pos.coluna] =
                    toupper((unsigned char)matrizSaida[pos.linha][pos.coluna]);
            }
        }
    }
}

// Escreve o resultado no arquivo de saída
bool escreverArquivoSaida(const string& nomeArquivo, const vector<string>& matrizSaida,
                          const vector<Resultado>& resultados) {
    ofstream saida(nomeArquivo);

    // Verifica se conseguiu abrir o arquivo
    if (!saida.is_open()) {
        return false;
    }

    // Escreve a matriz final
    for (const auto& linha : matrizSaida) {
        saida << linha << '\n';
    }

    // Escreve o resultado de cada palavra
    for (const auto& resultado : resultados) {
        if (resultado.encontrada) {
            saida << resultado.palavra << " - ("
                  << resultado.linhaInicial + 1 << ","
                  << resultado.colunaInicial + 1 << "): "
                  << resultado.direcao << '\n';
        } else {
            saida << resultado.palavra << " - nao encontrada" << '\n';
        }
    }

    saida.close();
    return true;
}

int main() {
    string arquivoEntrada, arquivoSaida;
    int linhas, colunas;
    vector<string> matriz;
    vector<string> palavras;

    // Solicita os nomes dos arquivos
    cout << "Digite o nome do arquivo de entrada: ";
    cin >> arquivoEntrada;

    cout << "Digite o nome do arquivo de saida: ";
    cin >> arquivoSaida;

    // Lê os dados do arquivo
    if (!lerArquivoEntrada(arquivoEntrada, linhas, colunas, matriz, palavras)) {
        cout << "Erro ao ler o arquivo de entrada.\n";
        return 1;
    }

    // Cria um resultado para cada palavra
    vector<Resultado> resultados(palavras.size());

    // Vetor que guarda as threads
    vector<thread> threads;

    // Cria uma thread para cada palavra
    for (int i = 0; i < (int)palavras.size(); i++) {
        threads.push_back(thread(procurarPalavra,
                                 cref(matriz),
                                 linhas,
                                 colunas,
                                 cref(palavras[i]),
                                 ref(resultados[i])));
    }

    // Espera todas as threads terminarem
    for (auto& t : threads) {
        t.join();
    }

    // Cria cópia da matriz original
    vector<string> matrizSaida = matriz;

    // Marca palavras encontradas
    marcarPalavrasMaiusculas(matrizSaida, resultados);

    // Escreve arquivo final
    if (!escreverArquivoSaida(arquivoSaida, matrizSaida, resultados)) {
        cout << "Erro ao escrever o arquivo de saida.\n";
        return 1;
    }

    cout << "Processamento concluido com sucesso.\n";
    return 0;
}