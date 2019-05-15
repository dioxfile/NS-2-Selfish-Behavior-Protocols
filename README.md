# NS-2-Selfish-Behavior-Protocols

# Definição de Nós Egoístas: 
O tipo de nó egoísta utilizado é aquele que descarta pacotes de dados e retransmite pacotes de controle, este tipo de nós egoísta mantém este comportamento todo o tempo.




| Parâmetros da Simulação          | Valor                                |
| ---------------------------------|-------------------------------------:|
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



# Como usar?
