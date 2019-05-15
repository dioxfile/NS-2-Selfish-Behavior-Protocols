# NS-2-Selfish-Behavior-Protocols

# Definição de Nós Egoístas: 
O tipo de nó egoísta utilizado é aquele que descarta pacotes de dados e retransmite pacotes de controle, este tipo de nós egoísta mantém este comportamento todo o tempo.

As quantidades de nós egoístas foram selecionadas aleatoriamente no intervalo [0, 49]. O comportamento egoísta foi implementado na função recv do OLSR, Algoritmo 1. Além disso, para contabilizar as perdas de pacotes por egoísmo foi implementado no NS-2 um evento de descarte denominado SEL, que significa SELFISH. Por exemplo, se um nó descartar um pacote por egoísmoo NS-2 gravará no arquivo de trace o evento como: D 19.868681666 _11_ RTR SEL 3445 cbr 1020 [...] 48:0 53:0 [...]. Os significados de cada um dos campos do trecho do arquivo de trace são apresentados a seguir: D: evento de descarte; 19.868681666: tempo em que o evento ocorreu; _11_: nó onde o evento ocorreu; RTR: camada em que o evento ocorreu; SEL: o tipo de evento, descarte por egoísmo; 3445: id do pacote que foi descartado; cbr: tipo de tráfego, Constant Bit Rate; 1020: tamanho do pacote; [...]: evento suprimido (supressão nossa); 48:0: nó que originou o pacote e porta de origem; 13:0: nó destino e porta de destino.

---
<p>
  <pre><code>
--------------------------------------------------------------
A LGORITHM 1
Comportamento Egoísta do Nó OLSR.
--------------------------------------------------------------
Input: Pacote de Dados ou Controle.
Output: Encaminhamento, Descarte ou Processamento do Pacote.
1: if (Pacote de Dados) then
2:  if (Pacote de Dados foi Originado no nó Atual) then
3:    Encaminha(Pacote);
4:  else
5:    Descarta(Pacote);
6:  end if
7: else
8:  Passe o pacote ao OLSR para processamento;
9: end if
  </code></pre>
</p>

Os resultados apresentados das métricas de desempenho avaliadas foram calculados da média aritmética de 12 fluxos de tráfego. O script utilizado para extração dessas métricas está disponível em [Performance-Network-Metrics-NS-2
](https://github.com/dioxfile/Performance-Network-Metrics-NS-2) (ex., Github). A Tabela 6 apresenta um resumo dos parâmetros usados na simulação.




| Parâmetros da Simulação          | Valor                                |
|:--------------------------------:|:------------------------------------:|
| Área de simulação                | 1000m × 1000m                        |
| Quantidade de nós                | 50                                   |
| Tipo de tráfego                  | CBR UDP                              |
|Tamanho dos pacotes               | 1000 bytes                           |     
|Taxa de transmissão               |16pps (128Kbps)                       |
|Quantidades de fluxo de tráfego   |12                                    |     
|Modelo de propagação do sinal     |TwoRayGround                          |
|Carga total de energia do nó      |100 Watts (W) (ex., 100J/s)           |
|Potência (TX/RX)                  |TX = 1,2W e RX = 0,6W                 |
|Alcance de interferência          |250m (Padrão no NS-2)                 |
|Tipo de MAC                       |IEEE 802.11b                          |
|Modelo de Mobilidade              |Random Waypoint                       |
|Velocidade do nó                  |mínima 5m/s e máxima 15m/s sem pausa  |  
|Quantidade de nós egoístas        |10%, 20%, 30% e 40%                   |    
|Comportamento egoísta dos nós     |Constante                             |
|Tempo de simulação                |50s                                   |
|Willingness OLSR                  |3                                     |
|Algoritmo de Seleção de MPR       |OLSR-FC = RFC 3626; OLSR-ETX,         | 
|                                  |OLSR-ML e OLSR-MD = Ge et al. [3]     |
|Quantidades de execuções          |   10                                 |
|Nível de confiança                | 95%                                  |
|Quantidade de Pacotes Gerados     |7644                                  |













# Como usar?
