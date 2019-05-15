# NS-2-Selfish-Behavior-Protocols
The mobile ad-hoc networks (MANET) are those whose nodes have mobility, energy restriction
and operate simultaneously as end systems and router. One of the main problems found in MANETs is the
occurrence of selfish nodes, which are those that refuse to route packets for other nodes. To address the issue
of selfish nodes in MANETs and improve the flow of traffic in these networks, this paper proposes an extension
to the OLSR protocol, based on Fuzzy logic, called OLSR Fuzzy Cost (OLSR -FC). Using the NS-2 simulator,
the OLSR-FC proposal was compared to other extensions of OLSR protocol (e.g., OLSR-ETX, OLSR-ML e
OLSR-MD) concerning the performance metrics: packet loss, end-to-end delay, Jitter, power consumption,
routing overhead and throughput. The results showed that OLSR-FC obtains better performance than the
evaluated extensions, avoiding selfish nodes and selecting routes whose links have little packet losses.

# Definition of Egoistic Nodes:
The node type is used as data source and relays the control options, this type of node is maintained at all times.

The quantities of selfish nodes were selected randomly in the interval [0, 49]. Selfish behavior was implemented in the recv function of the OLSR, Algorithm 1. In addition, to account for the losses of packages for selfishness was implemented in the NS-2 a discard event called SEL, which means SELFISH. For example, if a node discards a packet by selfish NS-2 it will write to the trace file the event as: D 19.868681666 _11_ RTR SEL 3445 cbr 1020 [...] 48: 0 53: 0 [...]. The meanings of each of the fields in the trace file excerpt are given below: D: discard event; 19.868681666: time at which the event occurred; _11_: node where the event occurred; RTR: layer in which the event occurred; SEL: the type of event, discarded by selfishness; 3445: id of the package that was dropped; cbr: traffic type, Constant Bit Rate; 1020: package size; [...]: event deleted (deletion from us); 48: 0: node that originated the source packet and port; 13: 0: destination node and destination port.

---
<p>
  <pre><code>
--------------------------------------------------------------
A LGORITHM 1
Selfish Behavior of the OLSR Node.
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

The results of the performance metrics evaluated were calculated from the arithmetic mean of 12 traffic flows. The script used to extract these metrics is available in [Performance-Network-Metrics-NS-2
](https://github.com/dioxfile/Performance-Network-Metrics-NS-2) (ex., Github). Table 6 presents a summary of the parameters used in the simulation.




| Simulation Parameters            | Value                                      |
|:--------------------------------:|:------------------------------------------:|
|Simulation zone                   |1000m × 1000m                               |
|Number of nodes                   |50                                          |
|Type of traffic                   |CBR UDP                                     |
|Package size                      |1000 bytes                                  |     
|Transmission rate                 |16pps (128Kbps)                             |
|Traffic Flow Quantities           |12                                          |     
|Sign propagation model            |TwoRayGround                                |
|Total power load of the node      |100 Watts (W) (ex., 100J/s)                 |
|Potency (TX/RX)                   |TX = 1,2W e RX = 0,6W                       |
|Range of interference             |250m (Padrão no NS-2)                       |
|Type of MAC                       |IEEE 802.11b                                |
|Mobility Model                    |Random Waypoint                             |
|Node Speed                        |minimum 5m/s and maximum 15m/s without pause|  
|Quantity of selfish nodes         |10%, 20%, 30% e 40%                         |    
|Selfish behavior of knots         |Constant                                    |
|Simulation time                   |50s                                         |
|Willingness OLSR                  |3                                           |
|Algorithm of Selection of MPR     |OLSR-FC = RFC 3626; OLSR-ETX, OLSR-ML e OLSR-MD = Ge et al. [3]| 
|                                  |                                            |
|Quantity of executions            |10                                          |
|Quantity of executions            |95%                                         |
|Quantity of Packages Generated    |7644                                        |













# Como usar?
