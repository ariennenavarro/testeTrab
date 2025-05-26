# Trabalho Prático de Grafos com C++ e Visualização em Python

Este projeto realiza a leitura, processamento e análise de grafos a partir de arquivos `.dat`, utilizando C++ para o processamento e Python para visualização dos resultados.

## 📁 Estrutura do Projeto

```
 📂 projeto/ 
├── dados  
|    └── MCGRP                          # Arquivos .dat para teste
|    |    └── BHW1.dat
|    |    └── BHW2.dat
|    |    └── ...
|    └── padrao_solucoes                # Arquivos .dat para referência
|    |    └── padrao_escrita.dat
|    |    └── sol-BHW1.dat
|    └── reference_values.csv           # CSV com todos os valores de referência
├── output                              # Saída para solucoes, executável e CSV
|    └── solucoes 
|    └── solucoes_individuais                              
|    └── codigo.exe 
|    └── resultados.csv
├── codigo.cpp                          # Código principal em C++
├── visualizacao.ipynb                  # Visualização do código principal
└── README.md                           # Explicações 
```

---

## 🧠 Funcionalidades

### 📍 Etapa 1

#### 🧩 Parte C++ (`codigo.cpp`)

O programa realiza as seguintes operações:

- Leitura de arquivos `.dat` com definição de grafos
- Construção da matriz de adjacência
- Identificação de vértices, arestas e arcos (requeridos e opcionais)
- Cálculo e exportação para CSV das seguintes métricas:

| Métrica                      | Descrição |
|-----------------------------|-----------|
| Número total de vértices    | Total de nós no grafo |
| Número de arestas e arcos   | Quantidade total de conexões |
| Densidade                   | Grau de conectividade do grafo |
| Componentes conexos         | Número de subgrafos conexos |
| Grau mínimo e máximo        | Grau mínimo/máximo entre os vértices |
| Caminho médio               | Média dos menores caminhos entre pares de vértices |
| Diâmetro                    | Maior menor caminho possível entre dois nós |
| Intermediação               | Centralidade de intermediação normalizada dos nós |

> Todos os dados são salvos no arquivo `resultados.csv`.

#### 📊 Parte Python (`visualizacao.ipynb`)

Este script realiza:

- Leitura do arquivo `resultados.csv` gerado pelo C++
- Visualização gráfica das principais métricas do grafo com **matplotlib**
- Geração de um gráfico de barras com:
  - Número de vértices
  - Número de arestas/arcos
  - Grau máximo/mínimo
  - Densidade
  - Componentes conexos
  - Caminho médio
  - Diâmetro

### 📍 Etapa 2

- **Leitura de Instâncias MCGRP**: O código lê arquivos .dat formatados para instâncias do MCGRP, extraindo informações como capacidade dos veículos, depósito, número de vértices e detalhes sobre serviços requeridos (nós, arestas, arcos) e não requeridos.
- **Construção do Grafo**: Com os dados extraídos, o grafo é construído utilizando uma matriz de adjacência que representa os custos diretos entre os nós.
- **Cálculo de Caminhos Mínimos**: Utiliza o algoritmo de Floyd-Warshall para calcular distâncias e predecessores entre todos os pares de vértices, essenciais para diversas métricas do problema.
- **Heurística do Vizinho Mais Próximo (VM)**:
    - Algoritmo construtivo que parte de uma solução vazia e itera até atender todas as restrições.
    - Cada rota inicia no depósito com capacidade máxima.
    - O serviço não atendido mais próximo e viável (dentro da capacidade) é selecionado.
    - Custos de deslocamento e de serviço são acumulados, e a demanda é descontada da capacidade.
    - O veículo avança para o final do serviço atendido e repete o processo até que nenhum outro serviço possa ser incluído.
    - Caso um serviço seja visitado mais de uma vez, sua demanda e custo de serviço são contados apenas uma vez.
    - A rota termina com o retorno ao depósito, garantindo que a capacidade não seja excedida e cada serviço seja atendido por exatamente uma rota.
- **Geração de Saída**: As soluções geradas (custo total, número de rotas, tempo de CPU e tempo de referência) são salvas em arquivos `.dat` na pasta `solucoes_individuais` (ou solucoes, se alterado no main), seguindo o padrão `sol-<nome_instancia>.dat`.
- **Medição de Tempo**: O tempo de execução é medido em ciclos de CPU com `__rdtsc()` e comparado com valores de referência extraídos de `reference_values.csv`.

---

## 🛠️ Requisitos

### C++

- Compilador compatível com C++11 ou superior (ex: `g++`, MSVC).
- As bibliotecas padrão `iostream`, `fstream`, `sstream`, `vector`, `set`, `string`, `iomanip`, `algorithm` são utilizadas.
- A biblioteca `<x86intrin.h>` é utilizada para a função `__rdtsc()` para medição de ciclos de CPU. Em sistemas Windows, `direct.h` é incluído.
- O grupo pode utilizar estruturas de dados da standard library, mas funções diretamente relacionadas a grafos de frameworks como networkx ou igraph não são permitidas[cite: 35, 36].

### Python

- Python 3.x
- Bibliotecas:
```bash
pip install pandas matplotlib
```

## 👨‍💻 Autores

- Arienne Alves Navarro
- Thales Rodrigues Resende