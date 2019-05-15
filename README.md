# NS-2-Selfish-Behavior-Protocols

# Definição de Nós Egoístas: 
O tipo de nó egoísta utilizado é aquele que descarta pacotes de dados e retransmite pacotes de controle, este tipo de nós egoísta mantém este comportamento todo o tempo.




| Parâmetros da Simulação     | Valor                 |
| ----------------------------|----------------------:|
| col 3 is                    | right-aligned         |
| col 2 is                    | centered              |
| zebra stripes               | are neat              |

-










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
