# NS-2-Selfish-Behavior-Protocols
The node type is used as data source and relays the control options, this type of node is maintained at all times.

You can read the full article clicking the link 
* [OLSR Fuzzy Cost (OLSR-FC): an extension to OLSR protocol based on fuzzy logic and applied to avoid selfish nodes](https://github.com/dioxfile/NS-2-Selfish-Behavior-Protocols/raw/master/86380-376997-1-PB.pdf)</br>
Made during master's dissertation of Diogenes Antonio Marques, teacher at the State University of Mato Grosso - Barra do Bugres - MT - Brazil 

---
<p>
  <pre><code>
--------------------------------------------------------------
A LGORITHM 1
Selfish Behavior of the AODV,DSDV,DSragent,OLSR, Node.
--------------------------------------------------------------
Input: Data Package or Control.
Output: Routing, Disposal, or Package Processing.
1: if (Data Package) then
2:  if (Data Packet was Originated in the Current Node) then
3:    Forwards(Package);
4:  else
5:    Drop(Package);
6:  end if
7: else
8:  Pass the package to the OLSR for processing;
9: end if
  </code></pre>
</p>

Ad-hoc mobile networks (MANETs) are those whose nodes have mobility, energy restriction and which function simultaneously as end systems and router. One of the main problems encountered in MANETs is the occurrence of selfish nodes.

According to [Wankhade](https://pdfs.semanticscholar.org/41e2/a539eb1e96cc6a150e92c8614705cd1c9b2f.pdf) one of the main problems of MANETs networks is the behavior of selfish nodes, which can be defined by the attitudes of obtaining network advantages (eg, flow, watts, among others), especially in packet routing, which consumes a significant amount of energy and not all nodes are willing to spend that precious resource by forwarding packets that are not theirs.





---
# Definition of Selfish Nodes:

The problem of the selfish knot according to [Bakakhouya](https://ieeexplore.ieee.org/abstract/document/4756492) is that it participates in routing functions by agreeing to forward packets on behalf of other nodes, but instead silently discards packets in an attempt to save energy and bandwidth.

* There are two types of us selfish:

<ol>
  <li>which correctly participates in the routing functions, but does not redirect data packets;</li>
  <li>and what does not participate, discarding control messages.</li>
</ol>

> The selfish knot of type 2 has no great effect on the packet delivery rate. However, type 1 is the most damaging to the network by degrading the packet delivery rate.




























# Como usar?
