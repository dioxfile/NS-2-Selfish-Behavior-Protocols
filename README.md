# NS-2-Selfish-Behavior-Protocols

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

---
# HOW TO USE?

You have two choices of how to use it, or you can download ns-allinone-2.34 already modified [here] (link) or download the ns-allinone-2.34 clean package [here] (link) and modify the codes manually.

## Manually changing:
Taking into consideration that you already have the ns-allinone-2.34 version compiled, you will have to add the modifications in the AODV files; DSDV; OLSR; DSR, respectively.

> You do not need to change all, just modify the protocol you want to use.

Moreover, in our tests, just recompiling with the `make` command was not enough, other commands like` make clean && make distclean` were necessary for the correct operation of the program.

To modify the files manually follow the tutorial below.

> Note: Files need to be compiled with a version of Gcc below version 5.0, in our tests we used the version "gcc (Ubuntu 4.8.5-4Ubuntu9) 4.8.5" and the version "g++ (GCC) 3.4.6 20060404 (Hed Hat 3.4.6-19.el6)".

## To use the AODV protocol, change the files:

* ns-2.34/aodv/aodv.cc
<p>
  <pre><code>
///Between the lines 97-98, add: 
///Start Sefish Behavior
    if(strcmp(argv[1], "egoista_on") == 0){
	   selfish = true;
	   return TCL_OK;
    } 
    ///Stop Sefish Behavior
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
## To use the DSDV protocol, change the files:
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
///On the line 1099, add:
selfish = false;
 </code></pre>
</p>

<p>
  <pre><code>
///Between the lines 1050 e 1051, add:
	///Start Sefish Behavior
      else if(strcmp(argv[1], "egoista_on") == 0){
	   selfish = true;
	   return TCL_OK;
      }  
      ///Stop Sefish Behavior
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
## To use the OLSR protocol, change the files:
* ns-2.34/olsr/olsr.cc

<p>
  <pre><code>
///Between the lines 216 e 217: add:
     ///Start Sefish Behavior
    if(strcmp(argv[1], "egoista_on") == 0){
	   selfish = true;
	   return TCL_OK;
    }  
    ///Stop Sefish Behavior
    else if(strcmp(argv[1], "egoista_off") == 0){
	   selfish = false;
	   return TCL_OK;
    }
</code></pre>
</p>

<p>
  <pre><code>
///Between the lines 477 e 478, add 
   //Selfish var starting
   selfish = false;
</code></pre>
</p>

<p>
  <pre><code>
///Between the lines 515 e 516, add:
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
///After the line 361, add:
   /// Sets the behavior selfish node
   bool selfish;
   </code></pre>
</p>

---
---
## To use the DSR protocol, change the files:
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
      ///Start Sefish Behavior
      if(strcmp(argv[1], "egoista_on") == 0){
	   selfish = true;
	   return TCL_OK;
      }  
      ///Stop Sefish Behavior
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












