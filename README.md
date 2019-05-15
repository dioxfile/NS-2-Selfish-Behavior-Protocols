# NS-2-Selfish-Behavior-Protocols

# Definição de Nós Egoístas: 
O tipo de nó egoísta utilizado é aquele que descarta pacotes de dados e retransmite pacotes de controle [18], este tipo de nós egoísta mantém este comportamento todo o tempo.


A LGORITHM 1</br>
Comportamento Egoísta do Nó OLSR.</br>
Input: Pacote de Dados ou Controle.</br>
Output: Encaminhamento, Descarte ou Processamento do Pacote.</br>
1: if (Pacote de Dados) then</br>
2:  if (Pacote de Dados foi Originado no nó Atual) then</br>
3:    Encaminha(Pacote);</br>
4:  else</br>
5:    Descarta(Pacote);</br>
6:  end if</br>
7: else</br>
8:  Passe o pacote ao OLSR para processamento;</br>
9: end if</br>

<p>
  Essa é a função sayHello():
  <pre><code>function sayHello() {
    return 'hi!';
  }</code></pre>
</p>



# Como usar?
