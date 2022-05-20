# NS-2-Selfish-Behavior-Protocols

Ad-hoc mobile networks (MANETs) are those whose nodes have mobility, energy restriction and which function simultaneously as end systems and router. One of the main problems encountered in MANETs is the occurrence of selfish nodes.

According to [Wankhade](https://pdfs.semanticscholar.org/41e2/a539eb1e96cc6a150e92c8614705cd1c9b2f.pdf) one of the main problems of MANETs networks is the behavior of selfish nodes, which can be defined by the attitudes of obtaining network advantages (e.g., throughput, watts, among others), especially in packet routing, which consumes a significant amount of energy and not all nodes are willing to spend that precious resource by forwarding packets that are not theirs.

1. [Definition of Selfish Nodes:](#Definition-of-Selfish-Nodes)
2. [HOW TO INSTALL?](#HOW-TO-INSTALL)</br>
2.1 [Manually changing:](#Manually-changing)</br>
2.1.1 [AODV](#To-use-the-AODV-protocol-change-the-files)</br>
2.1.2 [DSDV](#To-use-the-DSDV-protocol-change-the-files)</br>
2.1.3 [OLSR](#To-use-the-OLSR-protocol-change-the-files)</br>
2.1.4 [DSR](#To-use-the-DSR-protocol-change-the-files)</br>
3. [HOW TO USE?](#HOW-TO-USE) 

---
# Definition of Selfish Nodes:

The problem of the selfish nodes according to [Bakakhouya](https://ieeexplore.ieee.org/abstract/document/4756492) is that it participates in routing functions by agreeing to forward packets on behalf of other nodes. However, instead of silently drops packets in an attempt to save energy and bandwidth.

There are three types of selfish nodes:

1 - which correctly participates in the routing process. However, it does not forward data packets to other nodes. Algorithm 1 implements this behavior in AODV, DSDV, OLSR, and DSR protocols in NS-2. 

2 - these are nodes with low battery charge on the mobile device, even for maintain their own communication and as a result, after using the network, they turn off their Wireless interface causing route errors and consequently the loss of packets from other nodes.

3 - and what does not participate in the routing process, dropping control messages.

OBS: The selfish node of type 3 has no effect on the packet delivery rate. However, type 1 and type 2 are the ones that do the most damage to the network by degrading the packet delivery rate.

---
<p>
  <pre><code>
--------------------------------------------------------------
ALGORITHM 1
Selfish Behavior Type 1 of the AODV/DSDV/OLSR/DSR Node.
--------------------------------------------------------------
Input: Data Packet or Control Packet.
Output: Packet Routing, Packet Dropping, or  Packet Processing.
1: if (Data Packet) then
2:  if (Data Packet was Originated in the Current Node) then
3:    Forwards(Packet);
4:  else
5:    Drop(Packet);
6:  end if
7: else
8:  Pass the packet to the AODV/DSDV/OLSR/DSR for processing;
9: end if
  </code></pre>
</p>

---
# How to create Type 1 selfish behavior in NS-2?

You have two flavors of how to use it, for instance, you can download ns-allinone-2.34 already modified [NS-2-Selfish-Behavior
](https://github.com/dioxfile/NS-2-Selfish-Behavior/archive/master.zip) or download the ns-allinone-2.34 clean package [ns-allinone-2.34](https://github.com/dioxfile/ns-allinone-2.34/archive/master.zip) and modify the codes manually.

## Manually changing:
Taking into consideration that you already have the ns-allinone-2.34 version compiled, you will have to add the modifications in the files cmu-trace.h; AODV(.h and .cc); DSDV(.h and .cc); OLSR(.h and .cc); and DSR(.h and .cc), respectively.

> You do not need to change all, just modify cmu-trace.h and the protocol that you want to use.

## To create the dropping event by selfishness in NS-2:

* ns-2.34/trace/cmu-trace.h
<p>
  <pre><code>
  ///Between the lines 91-93, add: 
#define DROP_RTR_SELFISH                 "SEL"   // Selfish DROP
</code></pre>
</p>

## To use the AODV protocol change the files:

* ns-2.34/aodv/aodv.cc
<p>
  <pre><code>
///Between the lines 97-98, add: 
///Start Selfish Behavior
    if(strcmp(argv[1], "egoista_on") == 0){
	   selfish = true;
	   return TCL_OK;
    } 
    ///Stop Selfish Behavior
    else if(strcmp(argv[1], "egoista_off") == 0){
	   selfish = false;
	   return TCL_OK;
    }
</code></pre>
</p>    

<p>
  <pre><code>
///After the line 153, add:
///Initializing variable
  selfish = false;
</code></pre>
</p>  

<p>
  <pre><code>
///Between the lines 609-610, add:
///Set node's Behavior selfish - By Diógenes
   if(ih->saddr() != index && selfish == true){
	  drop(p, DROP_RTR_SELFISH); //Set as "SEL" in the trace.
	  return; 
  }
  </code></pre>
</p>  

---
* ns-2.34/aodv/aodv.h

<p>
  <pre><code>
	///After the line 326, add:
	///selfish node
	bool selfish;
 </code></pre>
</p> 	 

---
---
## To use the DSDV protocol change the files:
* ns-2.34/dsdv/dsdv.cc

<p>
  <pre><code>
///Between the lines 1060 e 1061, add:
     ///Set node's Behavior selfish - By Diógenes
      if(src != myaddr_ && selfish == true && cmh->ptype() != PT_MESSAGE){
	drop(p, DROP_RTR_SELFISH); //Set as "SEL" in the trace.
	return; 
      }
 </code></pre>
</p>      
      
<p>
  <pre><code>
///On the line 1106, add:
selfish = false;
 </code></pre>
</p>

<p>
  <pre><code>
///Between the lines 1160 e 1161, add:
	///Start Selfish Behavior
      else if(strcmp(argv[1], "egoista_on") == 0){
	   selfish = true;
	   return TCL_OK;
      }  
      ///Stop Selfish Behavior
      else if(strcmp(argv[1], "egoista_off") == 0){
	   selfish = false;
	   return TCL_OK;
    }
 </code></pre>
</p>

---
* ns-2.34/dsdv/dsdv.h

<p>
  <pre><code>
///After the line 140, add:  
///selfish node
  bool selfish;
</code></pre>
</p>

---
---
## To use the OLSR protocol change the files:
* ns-2.34/olsr/olsr.cc

<p>
  <pre><code>
///Between the lines 216 e 217: add:
     ///Start Selfish Behavior
    if(strcmp(argv[1], "egoista_on") == 0){
	   selfish = true;
	   return TCL_OK;
    }  
    ///Stop Selfish Behavior
    else if(strcmp(argv[1], "egoista_off") == 0){
	   selfish = false;
	   return TCL_OK;
    }
</code></pre>
</p>

<p>
  <pre><code>
///Between the lines 490 e 491, add 
   //Selfish var starting
   selfish = false;
</code></pre>
</p>

<p>
  <pre><code>
///Between the lines 531 e 531, add:
     ///Set node's Behavior selfish - By Diógenes
     if((ih->saddr() != ra_addr()) && selfish == true){
		drop(p, DROP_RTR_SELFISH); //Set as "SEL" in the trace.
		return; 
      }
   </code></pre>
</p>

---
* ns-2.34/olsr/olsr.h
<p>
  <pre><code>
///After the line 366, add:
   /// Sets the behavior selfish node
   bool selfish;
   </code></pre>
</p>

---
---
## To use the DSR protocol change the files:
* ns-2.34/dsr/dsragent.cc

<p>
  <pre><code>
///After the line 390, add:
selfish = false;
   </code></pre>
</p>

<p>
  <pre><code>
///Between the lines 490 e 491, add:
      ///Start Selfish Behavior
      if(strcmp(argv[1], "egoista_on") == 0){
	   selfish = true;
	   return TCL_OK;
      }  
      ///Stop Selfish Behavior
      else if(strcmp(argv[1], "egoista_off") == 0){
	   selfish = false;
	   return TCL_OK;
    }
   </code></pre>
</p>

<p>
  <pre><code>
///Between the lines 673 e 674, add:
	  ///Set node's Behavior selfish - By Diógenes
	  if(p.src != net_id && selfish == true && cmh->ptype() != PT_DSR){
	    drop(packet, DROP_RTR_SELFISH); //Set as "SEL" in the trace.
	    return; 
	  }
   </code></pre>
</p>

---
* ns-2.34/dsr/dsragent.h

<p>
  <pre><code>
///After the line 279, add:
  ///Selfish Node
  bool selfish;
   </code></pre>
</p>

In our tests, we just recompiling with the `make` command was not enough, other commands like` make clean && make distclean` were necessary for the correct operation of the program.

After modifying the files of the desired protocols compile the program with the commands (do with root permission):</br>
`# make clean`</br>
`# make distclean`</br> 
`# ./configure`</br>
`# make`</br>
`# make install`</br>
 
> Note: Files need to be compiled with a version of Gcc below version 5.0, in the tests we used the version "gcc (Ubuntu 4.8.5-4Ubuntu9) 4.8.5" and the version "g++ (GCC) 3.4.6 20060404 (Hed Hat 3.4.6-19.el6)".

## Observations: 
Another possibility is to download the modified files available in this git, for example: cmu-trace.h, aodv.(h,cc), dsdv.(h,cc), olsr.(h,cc) e dsragent.(h,cc). Replace the originals in: ns-2.34/trace/cmu-trace.h, ns-2.34/aodv/aodv.(h,cc), ns-2.34/dsdv/dsdv.(h,cc), ns-2.34/olsr/olsr.(h,cc) e ns-2.34/dsr/dsragent.(h,cc). And then recompile NS-2 as previously described.

# HOW TO USE?
Once NS-2 is installed and all necessary changes have been made to the cmu-trace.h files (e.g., packet discarding event for selfishness), AODV(.h and .cc) or DSDV(.h and .cc) or OLSR(.h and .cc) or DSR(.h and .cc), you now need to follow these steps:

1 - Download the 802_11b.tcl file and change line 14 to the protocol of your choice (AODV, DSDV, OLSR or DSR);

2 - Download the following files in the same folder where 802_11b.tcl is located: mesh_traffic.tcl, mobility.tcl, and Selfish_GENERATOR.cc. 

3 - In the terminal Compile/run the file Selfish_GENERATOR.cc: 
	
	a) $ sudo g++  Selfish_GENERATOR.cc -o Selfish_GENERATOR
	
	b) Now just run it: $ ./Selfish_GENERATOR 5 50. Where 5 is the number of selfish nodes and 50 is the total number of nodes in the simulation (previously configured in 802_11b.tcl).
	
	c) After running Selfish_GENERATOR, if everything went well, a Selfish.tcl file with a content similar to this was created:
	
	$ns_ at 0.0 "[$node_(19) set ragent_] egoista_on"
	$ns_ at 0.0 "$node_(19) color red"
	$ns_ at 0.0 "[$node_(42) set ragent_] egoista_on"
	$ns_ at 0.0 "$node_(42) color red"
	$ns_ at 0.0 "[$node_(1) set ragent_] egoista_on"
	$ns_ at 0.0 "$node_(1) color red"
	$ns_ at 0.0 "[$node_(14) set ragent_] egoista_on"
	$ns_ at 0.0 "$node_(14) color red"
	$ns_ at 0.0 "[$node_(26) set ragent_] egoista_on"
	$ns_ at 0.0 "$node_(26) color red"

4 -Execute, in the terminal, the simulation as follows: `$ ns 802_11b.tcl`, after the simulation is finished open the`trace file 'TRACE_Arquivo.tr'`, and with an editor of your choice check for a dropping event with the `flag SEL`, if yes, everything happened as planned. Ex: 

`D 11.197096623 _19_ RTR  SEL 339 cbr 1020 [13a 13 27 800] [energy 99.580939 ei 0.000 es 0.000 et 0.029 er 0.390] ------- [4:0 5:0 30 19] [6] 3 5`

OBS: make an analysis in the `trace file` to make sure that the simulation has occurred according to the modifications previously recommended and in case of doubts, review the process in detail.
	
## These codes were developed for use in this research: [OLSR-Fuzzy Cost](https://doi.org/10.22456/2175-2745.86380).
<!-- ![all text](https://github.com/dioxfile/NS-2-Selfish-Behavior-Protocols/raw/master/Images/protocol_change.png) -->

# How to create Type 2 selfish behavior in NS-2?
One of the main factors that inhibit cooperation in wireless ad hoc environments is energy savings, as the limitation of power supply undermines the belief that nodes in wireless ad hoc networks will always accept to retransmit packets from other nodes. Therefore, we define Type 2 selfish nodes as a node that goes offline when it has no personal interest in cooperating with other nodes in the system by forwarding their packets. Thus, for Shutting down the nodes in this sandbox uses an NS-2 function that is activated in the file ∼/ns-allinone-2.34/ns-2.34/common/mobilenode.cc, through the comment of line 202 tcl.evalf(" %s reset-state", str);. After commenting out this line, NS-2 must be recompiled. In addition, in order for the mobile nodes to be turned off, it is necessary to activate the NS-2 energy model (EnergyModel) in the simulation tcl script:

• set val(ModEner) EnergyModel : code inserted in the variable header;

• -energyModel $val(ModEner) : code inserted in the mobile node specification.

After that, once the NS-2 energy model is activated, a node can be turned off/on during the simulation with the following commands: $ns_ at 18 "$node_(18) off" (eg, indicates that the node eighteen (18) will be turned off 18 seconds after the start of the simulation) and $ns_ at 43.5 "$node_(18) on" (eg, indicates that node eighteen (18) will be turned back on at the time of 40.5s, 25.5s after it has been turned off).
In this context, a C++ program, Selfish_GEN_ON_OFF.cc, was developed to automate the creation of Type 2 selfish nodes. In this way, it will be possible to generate selfish nodes that are not transmitter/receiver nodes. The program in question also uses parameter passing and its explanation is as follows:
<p>
  <pre><code>
1. "USAGE: ./Selfish_GEN_ON_OFF 10 50 60 20.5 mesh_traffic.tcl !"-> the first two parameters of this program are exactly the same as the type 1 selfish node generator program.
Thus, the third parameter is the simulation duration time, it is necessary because to turn off the nodes it is necessary to know how long the simulation will last. Therefore, the fourth parameter is the amount of time the node will behave as selfish, that is, how long this node will be offline. And finally, the last parameter is the traffic file, which will be used in the simulation.

2. After running Selfish_GEN_ON_OFF, if everything went well, a Selfish_On_Off.tcl file with a content similar to this was created:	

	$ns_ at 18 "$node_(18) off"
	$ns_ at 43.5 "$node_(18) on"
	$ns_ at 0.0 "$node_(18) color red"
	$ns_ at 26 "$node_(19) off"
	$ns_ at 51.5 "$node_(19) on"
	$ns_ at 0.0 "$node_(19) color red"
	$ns_ at 10 "$node_(38) off"
	$ns_ at 35.5 "$node_(38) on"
	$ns_ at 0.0 "$node_(38) color red"
	$ns_ at 46 "$node_(20) off"
	$ns_ at 60 "$node_(20) on"
	$ns_ at 0.0 "$node_(20) color red"
	$ns_ at 47 "$node_(0) off"
	$ns_ at 60 "$node_(0) on"
	$ns_ at 0.0 "$node_(0) color red"
  </code></pre>
<p>

# OBS: when running Selfish_GEN_ON_OFF, SEL_DASH.sh must be in the same folder as Selfish_GEN_ON_OFF.
**[⬆ back to top](#NS-2-Selfish-Behavior-Protocols)**
