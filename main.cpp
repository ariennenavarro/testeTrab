#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <string>
#include <iomanip>
#include <tuple>
#include <ctime> 
#include <chrono>
#include <x86intrin.h>
using namespace std;

const int INF = 1e9;

struct Service {
    string id_original; // Para guardar "N4", "E7", "A8", etc. para a saída
    int id_numerico_sequencial; // NOVO MEMBRO
    enum Type { NODE, EDGE, ARC } type; // Apenas uma declaração de 'type'
    int u, v;           // Nós de início e fim (u=v para nós)
    int demand;         // Demanda do serviço
    int traversal_cost; // Custo de travessia (0 para nós)
    int service_cost;   // Custo de serviço
    bool served = false;// Para controlar se já foi atendido
};

// Estrutura para guardar uma visita na rota
struct RotaVisita {
    char tipo;          // 'D' para Depósito, 'S' para Serviço
    string id_servico;  // "0" para Depósito, ou o ID original (N4, E7, etc.)
    int u;              // Nó de origem/serviço
    int v;              // Nó de destino/serviço (u=v para nós e depósito)
};

// Estrutura para guardar uma rota completa
struct Rota {
    int id_rota;
    int demanda_total = 0;
    int custo_total = 0;
    vector<RotaVisita> visitas;
};

class Grafo {
private:
    int numVertices = 0;
    vector<vector<int>> matrizAdj;
    set<int> verticesRequeridos;
    vector<pair<int, int>> arestas;
    vector<pair<int, int>> arcos;
    set<pair<int, int>> arestasRequeridas;
    set<pair<int, int>> arcosRequeridos;
    vector<vector<int>> dist;
    vector<vector<int>> pred;
  
    int vehicleCapacity = 0;         // Capacidade (Q)
    int depotNode = 0;               // Nó do Depósito (v0)
    vector<vector<int>> costs;       // Matriz de Custos de Travessia
    vector<Service> requiredServices;// Vetor para guardar todos os serviços requeridos

    string limparEspacos(const string& s) {
        size_t inicio = s.find_first_not_of(" \t\r");
        size_t fim = s.find_last_not_of(" \t\r");
        return (inicio == string::npos || fim == string::npos) ? "" : s.substr(inicio, fim - inicio + 1);
    }

    private:
    // Encontra o índice do serviço mais próximo e viável
    int findNearestService(int currentLocation, int currentCapacity) {
        int bestServiceIdx = -1;    // Índice do melhor serviço encontrado (-1 se nenhum)
        int minCostToService = INF; // Custo para chegar ao melhor serviço

        // Percorre todos os serviços requeridos
        for (int i = 0; i < requiredServices.size(); ++i) {
            // Verifica se o serviço i AINDA NÃO foi atendido
            if (!requiredServices[i].served) {
                // Pega o nó de início do serviço i
                int serviceStartNode = requiredServices[i].u;
                // Pega o custo do caminho mínimo para chegar lá
                int cost = dist[currentLocation][serviceStartNode];

                // Verifica se:
                // 1. Existe um caminho (custo != INF)
                // 2. A demanda do serviço cabe no veículo (<= currentCapacity)
                // 3. O custo para chegar é o menor encontrado até agora
                if (cost != INF && requiredServices[i].demand <= currentCapacity && cost < minCostToService) {
                    minCostToService = cost; // Atualiza o menor custo
                    bestServiceIdx = i;      // Guarda o índice deste serviço
                }
            }
        }
        // Retorna o índice do melhor serviço, ou -1 se nenhum foi encontrado
        return bestServiceIdx;
    }

public:
    Grafo(const string& nomeArquivo) {
    int contador_id_servico = 1;
    ifstream arquivo(nomeArquivo);
    if (!arquivo.is_open()) {
        cerr << "Erro ao abrir o arquivo: " << nomeArquivo << endl;
        exit(1);
    }
    string linha;
    string secaoAtual = ""; // Para ajudar a controlar onde estamos

    while (getline(arquivo, linha)) {
        linha = limparEspacos(linha);
        if (linha.empty()) continue;

        // --- LENDO OS CABEÇALHOS ---
        if (linha.find("Capacity:") != string::npos) {
            sscanf(linha.c_str(), "Capacity: %d", &vehicleCapacity);
            cout << "Lido - Capacidade: " << vehicleCapacity << endl; // Debug
        } else if (linha.find("Depot Node:") != string::npos) {
            sscanf(linha.c_str(), "Depot Node: %d", &depotNode);
            cout << "Lido - Deposito: " << depotNode << endl; // Debug
        } else if (linha.find("#Nodes:") != string::npos) {
            sscanf(linha.c_str(), "#Nodes: %d", &numVertices);
            cout << "Lido - Vertices: " << numVertices << endl; // Debug
            // AGORA podemos inicializar a matriz de custos!
            costs.assign(numVertices + 1, vector<int>(numVertices + 1, INF));
            for(int i = 1; i <= numVertices; ++i) costs[i][i] = 0;
            // Mantenha sua matrizAdj se quiser, mas 'costs' é essencial agora.
            matrizAdj.assign(numVertices + 1, vector<int>(numVertices + 1, 0));
        }
        // --- IDENTIFICANDO SEÇÕES ---
        else if (linha.find("ReN.") != string::npos) { secaoAtual = "ReN"; continue; } // Pula header
        else if (linha.find("ReE.") != string::npos) { secaoAtual = "ReE"; continue; } // Pula header
        else if (linha.find("ReA.") != string::npos) { secaoAtual = "ReA"; continue; } // Pula header
        else if (linha.find("EDGE") != string::npos) { secaoAtual = "EDGE"; continue; } // Pula header
        else if (linha.find("ARC") != string::npos) { secaoAtual = "ARC"; continue; } // Pula header

        // --- LENDO DADOS DAS SEÇÕES ---
        if (secaoAtual == "ReN" && linha[0] == 'N') {
            char id_str[20];
            int demand_val, scost_val;
            sscanf(linha.c_str(), "%s %d %d", id_str, &demand_val, &scost_val);
            int node_num = atoi(id_str + 1);

            Service s;
            s.id_original = id_str;
            s.id_numerico_sequencial = contador_id_servico++; // Atribui e incrementa
            s.type = Service::NODE;
            s.u = node_num; s.v = node_num;
            s.demand = demand_val;
            s.traversal_cost = 0;
            s.service_cost = scost_val;
            requiredServices.push_back(s);
            verticesRequeridos.insert(node_num); // Pode manter
            cout << "Lido - ReN: " << s.id_original << endl; // Debug
        }
        else if (secaoAtual == "ReE" && linha[0] == 'E') {
            char id_str[20];
            int u_val, v_val, tcost_val, demand_val, scost_val;
            sscanf(linha.c_str(), "%s %d %d %d %d %d", id_str, &u_val, &v_val, &tcost_val, &demand_val, &scost_val);

            Service s;
            s.id_original = id_str;
            s.id_numerico_sequencial = contador_id_servico++;
            s.type = Service::EDGE;
            s.u = u_val; s.v = v_val;
            s.demand = demand_val;
            s.traversal_cost = tcost_val;
            s.service_cost = scost_val;
            requiredServices.push_back(s);
            costs[u_val][v_val] = tcost_val; // Adiciona custo na matriz
            costs[v_val][u_val] = tcost_val; // É aresta!
            matrizAdj[u_val][v_val] = 2; // Pode manter se quiser
            matrizAdj[v_val][u_val] = 2;
            cout << "Lido - ReE: " << s.id_original << endl; // Debug
        }
         else if (secaoAtual == "ReA" && linha[0] == 'A') {
            char id_str[20];
            int u_val, v_val, tcost_val, demand_val, scost_val;
            sscanf(linha.c_str(), "%s %d %d %d %d %d", id_str, &u_val, &v_val, &tcost_val, &demand_val, &scost_val);

            Service s;
            s.id_original = id_str;
            s.id_numerico_sequencial = contador_id_servico++;
            s.type = Service::ARC;
            s.u = u_val; s.v = v_val;
            s.demand = demand_val;
            s.traversal_cost = tcost_val;
            s.service_cost = scost_val;
            requiredServices.push_back(s);
            costs[u_val][v_val] = tcost_val; // Adiciona custo na matriz (só uma direção)
            matrizAdj[u_val][v_val] = 1; // Pode manter se quiser
            cout << "Lido - ReA: " << s.id_original << endl; // Debug
        }
        else if (secaoAtual == "ARC" && linha.rfind("NrA", 0) == 0) { // Verifica se começa com NrA
             char id_str[20];
             int u_val, v_val, tcost_val;
             sscanf(linha.c_str(), "%s %d %d %d", id_str, &u_val, &v_val, &tcost_val);
             // Só adiciona se não houver um custo menor (ou nenhum)
             if (costs[u_val][v_val] == INF) {
                costs[u_val][v_val] = tcost_val;
             }
             cout << "Lido - NrA: " << id_str << endl; // Debug
        }
        // Adicionar lógica para 'EDGE' (NrE) se necessário

        // Se a linha não se encaixar em nada, pode ser que mudou de seção,
        // mas a forma como lidamos com 'continue' e 'getline' deve tratar isso.
        // Se encontrar uma linha que não é cabeçalho nem dado, ela será ignorada.
    }
    arquivo.close();
    cout << "Leitura concluida. Total de servicos: " << requiredServices.size() << endl;
}

    void salvarEstatisticas() {
        ofstream resultados("resultados.csv");
        resultados << "Metrica,Valor" << endl;
        
        resultados << "Numero total de vertices," << numVertices << endl;
        resultados << "Numero total de arestas," << contarArestas() << endl;
        resultados << "Numero total de arcos," << contarArcos() << endl;
        resultados << "Numero de vertices requeridos," << verticesRequeridos.size() << endl;
        resultados << "Numero de arestas requeridas," << arestasRequeridas.size() / 2 << endl;
        resultados << "Numero de arcos requeridos," << arcosRequeridos.size() << endl;
        resultados << "Densidade do grafo," << calcularDensidade() << endl;
        resultados << "Componentes conexos," << contarComponentesConexos() << endl;
        resultados << "Grau minimo," << calcularGrauMinimo() << endl;
        resultados << "Grau maximo," << calcularGrauMaximo() << endl;

        resultados.close();
    }

    int contarArestas() {
        int total = 0;
        for (int i = 1; i <= numVertices; ++i) {
            for (int j = i + 1; j <= numVertices; ++j) {
                if (matrizAdj[i][j] == 2) total++;
            }
        }
        return total;
    }

    int contarArcos() {
        int total = 0;
        for (int i = 1; i <= numVertices; ++i) {
            for (int j = 1; j <= numVertices; ++j) {
                if (matrizAdj[i][j] == 1) total++;
            }
        }
        return total;
    }

    double calcularDensidade() {
        int totalConexoes = contarArestas() * 2 + contarArcos();
        double maxConexoes = numVertices * (numVertices - 1);
        return totalConexoes / maxConexoes;
    }

    void dfs(int v, vector<bool>& visitado) {
        visitado[v] = true;
        for (int i = 1; i <= numVertices; ++i) {
            if (!visitado[i] && (matrizAdj[v][i] > 0 || matrizAdj[i][v] > 0)) {
                dfs(i, visitado);
            }
        }
    }

    int contarComponentesConexos() {
        vector<bool> visitado(numVertices + 1, false);
        int componentes = 0;
        for (int i = 1; i <= numVertices; ++i) {
            if (!visitado[i]) {
                dfs(i, visitado);
                componentes++;
            }
        }
        return componentes;
    }

    void calcularCaminhosMinimosComCustos() {
    dist.assign(numVertices + 1, vector<int>(numVertices + 1, INF));
    pred.assign(numVertices + 1, vector<int>(numVertices + 1, -1));

    // Inicializa distâncias a partir da matriz de CUSTOS
    for (int i = 1; i <= numVertices; ++i) {
        for (int j = 1; j <= numVertices; ++j) {
            if (i == j) {
                dist[i][j] = 0;
                pred[i][j] = i;
            } else if (costs[i][j] != INF) { // USA A MATRIZ 'costs'
                dist[i][j] = costs[i][j];    // USA O CUSTO REAL
                pred[i][j] = i;
            }
        }
    }

    // Algoritmo de Floyd-Warshall com custos
    for (int k = 1; k <= numVertices; ++k) {
        for (int i = 1; i <= numVertices; ++i) {
            for (int j = 1; j <= numVertices; ++j) {
                // Previne overflow se dist[i][k] ou dist[k][j] for INF
                if (dist[i][k] != INF && dist[k][j] != INF && 
                    dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    pred[i][j] = pred[k][j]; // Aqui pred[k][j] deve ser usado
                }
            }
        }
    }
    cout << "Calculo de Caminhos Minimos com Custos concluido." << endl;
}

    void calcularCaminhoMedio() {
        ofstream resultados("resultados.csv", ios::app);
        int soma = 0;
        int contagem = 0;
        for (int i = 1; i <= numVertices; ++i) {
            for (int j = 1; j <= numVertices; ++j) {
                if (i != j && dist[i][j] != INF) {
                    soma += dist[i][j];
                    contagem++;
                }
            }
        }
        if (contagem == 0) {
            // cout << "Nao ha caminhos entre pares de vertices." << endl;
            resultados << "Caminho medio,Nao ha caminhos entre pares de vertices." << endl;
        } else {
            double caminhoMedio = static_cast<double>(soma) / contagem;
            // cout << "Caminho medio: " << fixed << setprecision(2) << caminhoMedio << endl;
            resultados << "Caminho medio," << fixed << setprecision(2) << caminhoMedio << endl;
        }
        resultados.close();
    }

    void calcularDiametro() {
        ofstream resultados("resultados.csv", ios::app);
        int diametro = 0;
        for (int i = 1; i <= numVertices; ++i) {
            for (int j = 1; j <= numVertices; ++j) {
                if (i != j && dist[i][j] != INF) {
                    diametro = max(diametro, dist[i][j]);
                }
            }
        }
        // cout << "Diametro do grafo: " << diametro << endl;
        resultados << "Diametro do grafo," << diametro << endl;
        resultados.close();
    }

    void calcularIntermediacao() {
        ofstream resultados("resultados.csv", ios::app);
        vector<double> intermediacao(numVertices + 1, 0.0);
        
        // itera sobre todos os pares de vertices (s, t)
        for (int s = 1; s <= numVertices; ++s) {
            for (int t = 1; t <= numVertices; ++t) {
                if (s != t && dist[s][t] != INF) { // verifica se ha um caminho minimo entre s e t
                    // reconstroi o caminho de t ate s usando a matriz de predecessores
                    int atual = t;
                    vector<int> caminho;
    
                    while (atual != -1 && atual != s) {
                        caminho.push_back(atual);
                        atual = pred[s][atual];
                    }
    
                    if (atual == s) { // se o caminho foi reconstruido corretamente
                        caminho.push_back(s); // adiciona o no de origem
    
                        // conta os nos intermediarios no caminho
                        for (size_t i = 1; i < caminho.size() - 1; ++i) {
                            intermediacao[caminho[i]] += 1.0;
                        }
                    }
                }
            }
        }
    
        // calcula o numero total de pares de vertices conectados
        double totalPares = (double)(numVertices * (numVertices - 1)) / 2.0;
    
        // normaliza os valores de intermediação
        for (int v = 1; v <= numVertices; ++v) {
            intermediacao[v] /= totalPares;
        }
    
        // imprime os resultados no arquivo
        resultados << "Intermediacao dos vertices (normalizada):" << endl;
        for (int v = 1; v <= numVertices; ++v) {
            resultados << "Vertice " << v << "," << fixed << setprecision(4) << intermediacao[v] << endl;
        }
        resultados.close();
    }

    int calcularGrauMinimo() {
        int min_grau = INF;
        for(int i = 1; i <= numVertices; i++) {
            int grau = 0;
            for(int j = 1; j <= numVertices; j++) {
                if(matrizAdj[i][j] != 0) grau++;
            }
            min_grau = min(min_grau, grau);
        }
        return min_grau;
    }
    
    int calcularGrauMaximo() {
        int max_grau = 0;
        for(int i = 1; i <= numVertices; i++) {
            int grau = 0;
            for(int j = 1; j <= numVertices; j++) {
                if(matrizAdj[i][j] != 0) grau++;
            }
            max_grau = max(max_grau, grau);
        }
        return max_grau;
    }

    int getDepot() { return depotNode; }
    int getCapacity() { return vehicleCapacity; }
    vector<Service>& getRequiredServices_ref() { return requiredServices; } // Retorna referência para modificar
    int getCost(int u, int v) { return costs[u][v]; }
    int getShortestPathCost(int u, int v) { return dist[u][v]; }
    vector<int> getCaminho(int u, int v); // Precisamos criar/adaptar

    void construirESalvarSolucaoNN(const string& nomeInstancia) {
        calcularCaminhosMinimosComCustos(); // Garante que os caminhos foram calculados

        unsigned long long inicio_ciclos_cpu, fim_ciclos_cpu;
        inicio_ciclos_cpu = __rdtsc();

        vector<Rota> todasAsRotas; // Vetor para guardar todas as rotas
        int servicosAtendidos = 0;
        int totalServicos = requiredServices.size();
        int routeIdCounter = 1;
        int custoTotalSolucao = 0;

        // Loop principal: continua até todos os serviços serem atendidos
        while (servicosAtendidos < totalServicos) {
            Rota rotaAtual; // Cria uma nova Rota
            rotaAtual.id_rota = routeIdCounter++;
            
            int cargaAtual = vehicleCapacity;
            int localizacaoAtual = depotNode;

            // Adiciona partida do depósito
            rotaAtual.visitas.push_back({'D', "0", depotNode, depotNode}); 

            // Loop da Rota: constrói uma rota até não poder mais
            while (true) {
                // Usa a função privada findNearestService 
                int proximoServicoIdx = findNearestService(localizacaoAtual, cargaAtual);

                if (proximoServicoIdx == -1) break; // Sai do loop da rota

                Service& servico = requiredServices[proximoServicoIdx];

                // Acumula custos
                rotaAtual.custo_total += dist[localizacaoAtual][servico.u];
                rotaAtual.custo_total += servico.traversal_cost;
                rotaAtual.custo_total += servico.service_cost;

                // Acumula demanda e atualiza capacidade
                rotaAtual.demanda_total += servico.demand;
                cargaAtual -= servico.demand;

                // Atualiza estado
                servico.served = true;
                servicosAtendidos++;
                localizacaoAtual = servico.v; 

                // Adiciona a visita do serviço à rota
                rotaAtual.visitas.push_back({'S', to_string(servico.id_numerico_sequencial), servico.u, servico.v});

                if (servicosAtendidos == totalServicos) break;
            }

            // Retorna ao Depósito
            rotaAtual.custo_total += dist[localizacaoAtual][depotNode];
            rotaAtual.visitas.push_back({'D', "0", depotNode, depotNode});

            // Guarda a rota completa e soma seu custo ao total
            todasAsRotas.push_back(rotaAtual); // Guarda a Rota completa
            custoTotalSolucao += rotaAtual.custo_total;
        }

        fim_ciclos_cpu = __rdtsc();
        unsigned long long ciclos_cpu_heuristica = fim_ciclos_cpu - inicio_ciclos_cpu;
      

        // --- SALVANDO A SOLUÇÃO NO ARQUIVO ---
        string nomeArquivoSaida = "sol-" + nomeInstancia + ".dat";
        ofstream arquivoSaida(nomeArquivoSaida);

        if (!arquivoSaida.is_open()) {
            cerr << "Erro ao criar o arquivo de saida: " << nomeArquivoSaida << endl;
            return;
        }

        arquivoSaida << custoTotalSolucao << endl;
        arquivoSaida << todasAsRotas.size() << endl;
        arquivoSaida << "0" << endl;
        arquivoSaida << ciclos_cpu_heuristica << endl;
      


        // Escreve cada rota
        for (const auto& rota : todasAsRotas) {
            arquivoSaida << "0 1 " // Depósito 0 (fixo), Dia 1 (fixo)
                         << rota.id_rota << " "
                         << rota.demanda_total << " "
                         << rota.custo_total << " "
                         << rota.visitas.size();

            // Escreve cada visita da rota
            for (const auto& visita : rota.visitas) {
                arquivoSaida << " (" << visita.tipo << " "
                             << visita.id_servico << ","
                             << visita.u << ","
                             << visita.v << ")";
            }
            arquivoSaida << endl;
        }

        arquivoSaida.close();

        cout << "\n--- Solucao Finalizada e Salva ---" << endl;
        cout << "Arquivo gerado: " << nomeArquivoSaida << endl;
        cout << "Total de Rotas: " << todasAsRotas.size() << endl;
        cout << "Custo Total da Solucao: " << custoTotalSolucao << endl;
        cout << "Ciclos de CPU da Heuristica (estimado com __rdtsc): " << ciclos_cpu_heuristica << endl;
        // cout << "Valor para linha 3 (placeholder): " << clock_linha3_ref << endl;
    }
    
};

int main() {
    Grafo g("C:/Users/arien/OneDrive/Documentos/GitHub/TrabalhoGrafos/testeTrab/BHW1.dat");

    string nomeInstancia = "BHW1"; 
    string nomeArquivoEntrada = nomeInstancia + ".dat";

    /*g.salvarEstatisticas();
    g.calcularCaminhosMinimosComCustos();
    g.calcularCaminhoMedio();
    g.calcularDiametro();
    g.calcularIntermediacao();
    return 0;*/
    g.construirESalvarSolucaoNN(nomeInstancia);
}
