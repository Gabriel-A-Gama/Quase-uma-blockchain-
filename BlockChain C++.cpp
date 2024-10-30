#include <iostream>
#include <list>
#include <map>
#include <string>
#include <sstream>
#include <random>
#include <chrono>

using namespace std;


// Contadores globais
int cont = 0;
int n_mensagens = 0;
int cont_block = 0;

// Estrutura de um cliente (processo)
struct client {
    int id;
};

// Estrutura de uma mensagem
struct message_structure {
    client origin;
    client destiny;
    string texto;
    string hash_mensagem;
};

// Estrutura de um bloco
//struct block {
//    string hash_id;
//    list<message_structure> messages;
//    int numero;
//    block* anterior;
//    block* proximo;
//    std::string hash_anterior;
//};

// struct Gama
struct block {
    std::string hash_id;
    std::string hash_anterior;
    std::string hash_proximo;
    std::list<message_structure> messages;
    int numero;
};


//criptografia GAMA=====================================
#undef max
std::string get_current_date() {
    using namespace std::chrono;
    auto now = system_clock::to_time_t(system_clock::now());
    std::tm localTime;
    localtime_s(&localTime, &now);

    char date[11]; // yyyy-mm-dd\0
    std::strftime(date, sizeof(date), "%Y-%m-%d", &localTime);

    return std::string(date);
}

std::string xorEncryptDecrypt(const std::string& data, const std::string& key) {
    std::string output = data;
    size_t keyLength = key.length();

    for (size_t i = 0; i < data.length(); ++i) {
        output[i] = data[i] ^ key[i % keyLength];
    }
    return output;
}

std::string base64_decode(const std::string& in) {
    static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    std::string out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++)
        T[base64_chars[i]] = i;
    int val = 0, valb = -8;
    for (unsigned char c : in) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}
std::string base64_encode(const std::string& in) {
    static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (out.size() % 4) {
        out.push_back('=');
    }
    return out;
}

std::string encript_hash_bloco(const block* bloco) {
   
    std::string current_date = get_current_date();

 
    std::ostringstream oss;
    oss << std::hex << reinterpret_cast<uintptr_t>(bloco);
    std::string endereco_memoria = oss.str();


    std::string hash_inicial = current_date + "|" + endereco_memoria;


    std::string chave = "BLOCKCHAIN_CRIPT_PATRAO";
    std::string hash_encriptado = xorEncryptDecrypt(hash_inicial, chave);


    return base64_encode(hash_encriptado);
}

std::string decript_hash_bloco(const std::string& hash_encriptado_base64) {

    std::string hash_encriptado = base64_decode(hash_encriptado_base64);


    std::string chave = "BLOCKCHAIN_CRIPT_PATRAO";


    std::string hash_original = xorEncryptDecrypt(hash_encriptado, chave);


    size_t separator_pos = hash_original.find('|');
    if (separator_pos == std::string::npos) {

        return "Erro: Hash invalido";
    }

    std::string data_descriptografada = hash_original.substr(0, separator_pos);
    std::string endereco_memoria = hash_original.substr(separator_pos + 1);

   
    if (data_descriptografada != get_current_date()) {
        return "Erro: Data de hash invalida";
    }
    return endereco_memoria;
}
//criptografia GAMA=====================================






// Funções auxiliares
string hash_mensagem();
//void create_block(const list<message_structure>& lista, block* block_head);
void create_block(const std::list<message_structure>& lista, block*& block_head);
string hash_block(const list<message_structure>& lista);
void bloco_genesis_cabecao();
void exibir_mensagens_block(int n, block block_head);

// Cria um hash para cada mensagem
string hash_mensagem() {
    static const char caracteres[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(0, sizeof(caracteres) - 2);

    string hash;
    for (int i = 0; i < 6; ++i) {
        hash += caracteres[distrib(gen)];
    }
    return hash;
}

// Envia a mensagem entre clientes
void send_message(client* c1, client* c2, list<message_structure>& lista, block* block_head) {
    string texto = "mensagem numero " + to_string(cont);
    cont++;

    message_structure mensagem;
    mensagem.origin = *c1;
    mensagem.destiny = *c2;
    mensagem.texto = texto;
    mensagem.hash_mensagem = hash_mensagem();
    n_mensagens += 1;

    cout << c1->id << " enviou para " << c2->id << " a mensagem: " << mensagem.texto << endl;

    lista.push_back(mensagem);

    if (n_mensagens == 5) {
        create_block(lista, block_head);
        n_mensagens = 0;
        lista.clear();
    }
}

// Cria bloco quando atinge 6 mensagens
//void create_block(const list<message_structure>& lista, block* block_head) {
//    block novo_bloco;
//
//    if (block_head == nullptr) {
//        bloco_genesis_cabecao();
//        return;
//    }
//
//    if (block_head->proximo == nullptr) {
//        block_head->proximo = &novo_bloco;
//        novo_bloco.anterior = block_head;
//        novo_bloco.proximo = nullptr;
//    }
//    else {
//        block* bloco_atual = block_head;
//        while (bloco_atual->proximo != nullptr) {
//            bloco_atual = bloco_atual->proximo;
//        }
//        bloco_atual->proximo = &novo_bloco;
//        novo_bloco.anterior = bloco_atual;
//        novo_bloco.proximo = nullptr;
//    }
//
//    novo_bloco.hash_id = hash_block(lista);
//    novo_bloco.messages = lista;
//    novo_bloco.numero = cont_block;
//    cont_block++;
//
//    cout << "Bloco criado com hash_id: " << novo_bloco.hash_id << endl;
//}

void create_block(const std::list<message_structure>& lista, block*& block_head) {
    block* novo_bloco = new block;

    novo_bloco->messages = lista;
    novo_bloco->numero = cont_block;
    novo_bloco->hash_id = hash_block(lista);
    cont_block++;

    if (block_head == nullptr) {
        bloco_genesis_cabecao();
        block_head = novo_bloco;
        return;
    }

    std::string hash_anterior = encript_hash_bloco(block_head);
    novo_bloco->hash_anterior = hash_anterior;
    block_head->hash_proximo = encript_hash_bloco(novo_bloco); 
    block_head = novo_bloco;

    std::cout << "Bloco criado com hash_id: " << novo_bloco->hash_id << std::endl;
}


block* get_block_by_hash(const std::string& hash_encriptado, const std::map<std::string, block*>& blockchain) {
    std::string endereco_str = decript_hash_bloco(hash_encriptado);
    uintptr_t endereco = std::stoul(endereco_str, nullptr, 16);
    return reinterpret_cast<block*>(endereco);
}





// Primeiro bloco
//void bloco_genesis_cabecao() {
//    block bloco_genesis;
//    bloco_genesis.hash_id = "000000";
//    bloco_genesis.numero = 0;
//    bloco_genesis.anterior = nullptr;
//    bloco_genesis.proximo = nullptr;
//
//    cout << "Bloco Genesis criado com hash_id: " << bloco_genesis.hash_id << endl;
//}
void bloco_genesis_cabecao() {
    block bloco_genesis;
    bloco_genesis.hash_id = "000000";
    bloco_genesis.numero = 0;
    bloco_genesis.hash_anterior = "";
    bloco_genesis.hash_proximo = "";

    cout << "Bloco Genesis criado com hash_id: " << bloco_genesis.hash_id << endl;
}

 //Exibe as mensagens de um bloco
void exibir_mensagens_block(int n, block block_head) {
    bool eh_ou_nao_eh = false;
    block bloco_atual = block_head;

    while (!eh_ou_nao_eh) {
        if (bloco_atual.numero == n) {
            for (const auto& msg : bloco_atual.messages) {
                cout << "Mensagem: " << msg.texto << " (de " << msg.origin.id << " para " << msg.destiny.id << ")" << endl;
            }
            eh_ou_nao_eh = true;
        }
    }
}

// Cria o hash do bloco
string hash_block(const list<message_structure>& lista) {
    string hash_criptografado;

    for (const message_structure& msg : lista) {
        string hash_parcial = msg.hash_mensagem.substr(0, 3);
        hash_criptografado += hash_parcial;
    }

    return hash_criptografado;
}

// Função principal
int main() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(0, 9);

    client cliente;
    cliente.id = distrib(gen);
    std::cout << "ola mundo" << std::endl;
    std::cin.get();


    return 0;
}