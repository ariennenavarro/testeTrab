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

### 🧩 Parte C++ (`codigo.cpp`)

O programa realiza as seguintes operações:

### Etapa 1

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

### Etapa 2

- **Leitura de Instâncias MCGRP**: O código lê arquivos `.dat` formatados para instâncias do MCGRP, extraindo informações como capacidade do veículo, nó de depósito, número de vértices, e detalhes dos serviços requeridos (nós, arestas, arcos) e não requeridos (arestas e arcos).
- **Construção do Grafo**: A partir dos dados lidos, constrói uma representação do grafo, incluindo a matriz de adjacência e os custos diretos entre os nós.
- **Cálculo de Caminhos Mínimos**: Utiliza o algoritmo de Floyd-Warshall para calcular as distâncias e predecessores de todos os pares de vértices no grafo[cite: 20]. Este é um passo importante, pois muitas métricas do grafo utilizam os resultados da matriz de caminhos mais curtos de múltiplas fontes[cite: 19].
- **Heurística do Vizinho Mais Próximo (VM) para Solução Inicial**:
    - O algoritmo é construtivo, iniciando com uma solução vazia e adicionando iterações até construir uma solução que atenda a todas as restrições do problema[cite: 22].
    - Para cada rota, o veículo parte do depósito com sua capacidade máxima.
    - Ele identifica o serviço não atendido mais próximo e viável (cuja demanda não excede a capacidade atual do veículo).
    - O custo para alcançar o início do serviço é adicionado ao custo total da rota, juntamente com o custo de percurso e o custo de serviço do próprio serviço.
    - A demanda do serviço é subtraída da capacidade atual do veículo, e o serviço é marcado como atendido.
    - O veículo se move para o nó final do serviço atendido e continua buscando o próximo serviço mais próximo e viável a partir de sua localização atual.
    - Caso uma rota passe mais de uma vez por um vértice, ou uma aresta, ou um arco requeridos, o valor de demanda do serviço e seu custo de serviço devem ser contados apenas 1 vez[cite: 24].
    - Este processo se repete até que nenhum outro serviço possa ser adicionado à rota ou todos os serviços tenham sido atendidos.
    - A rota é então finalizada com o retorno do veículo ao depósito.
    - A solução garante que a capacidade dos veículos não seja ultrapassada em cada rota e que cada serviço seja executado por exatamente uma rota[cite: 23, 24].
- **Geração de Saída**: As soluções (custo total, número de rotas, tempo de execução em ciclos de CPU e tempo de referência) são salvas em arquivos `.dat` individuais na pasta `solucoes_individuais` (ou `solucoes` se a parte comentada em `main` for ativada), com o formato `sol-<nome_instancia>.dat`[cite: 29]. As soluções devem seguir um padrão de nomenclatura específico[cite: 29].
- **Medição de Tempo**: O tempo de execução do algoritmo é medido em ciclos de CPU utilizando `__rdtsc()` para análise de desempenho. Para comparação, o código também lê um valor de referência de tempo do arquivo `reference_values.csv`.

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