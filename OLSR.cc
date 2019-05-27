 /***************************************************************************
  *   Copyright (C) 2004 by Francisco J. Ros                                *
  *   fjrm@dif.um.es                                                        *
  *                                                                         *
  *   Modified by Weverton Cordeiro                                         *
  *   (C) 2007 wevertoncordeiro@gmail.com                                   *
  *                                                                         *
  *   Modified by Diógenes Antonio Marques José                             *
  *   (C) 2014-2017 dioxfile@gmail.com/dioxfile@unemat.br                   *
  *                                                                         *
  *   This program is free software; you can redistribute it and/or modify  *
  *   it under the terms of the GNU General Public License as published by  *
  *   the Free Software Foundation; either version 2 of the License, or     *
  *   (at your option) any later version.                                   *
  *                                                                         *
  *   This program is distributed in the hope that it will be useful,       *
  *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
  *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
  *   GNU General Public License for more details.                          *
  *                                                                         *
  *   You should have received a copy of the GNU General Public License     *
  *   along with this program; if not, write to the                         *
  *   Free Software Foundation, Inc.,                                       *
  *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
  ***************************************************************************/
 
 ///
 /// \file  OLSR.cc
 /// \brief  Implementation of OLSR agent and related classes.
 ///
 /// This is the main file of this software because %OLSR's behaviour is
 /// implemented here.
 ///
 
 #include <olsr/OLSR.h>
 #include <olsr/OLSR_pkt.h>
 #include <olsr/OLSR_printer.h>
 #include <olsr/OLSR_dijkstra.h>
 #include <olsr/OLSR_parameter.h>
 #include <math.h>
 #include <limits.h>
 #include <address.h>
 #include <ip.h>
 #include <cmu-trace.h>
 #include <map>
 
 /// Length (in bytes) of UDP header.
 #define UDP_HDR_LEN  8
 
 ///
 /// \brief Function called by MAC layer when cannot deliver a packet.
 ///
 /// \param p Packet which couldn't be delivered.
 /// \param arg OLSR agent passed for a callback.
 ///
 static void
 olsr_mac_failed_callback(Packet *p, void *arg) {
   ((OLSR*)arg)->mac_failed(p);
 }
 
 class OLSR_parameter parameter_;
 
 
 /********** TCL Hooks **********/
 
 
 int OLSR_pkt::offset_;
 static class OLSRHeaderClass : public PacketHeaderClass {
 public:
   OLSRHeaderClass() : PacketHeaderClass("PacketHeader/OLSR", sizeof(OLSR_pkt)) {
     bind_offset(&OLSR_pkt::offset_);
   }
 } class_rtProtoOLSR_hdr;
 
 static class OLSRClass : public TclClass {
 public:
   OLSRClass() : TclClass("Agent/OLSR") {}
   TclObject* create(int argc, const char*const* argv) {
     // argv has the following structure:
     // <tcl-object> <tcl-object> Agent/OLSR create-shadow <id>
     // e.g: _o17 _o17 Agent/OLSR create-shadow 0
     // argv[4] is the address of the node
     assert(argc == 5);
     return new OLSR((nsaddr_t)Address::instance().str2addr(argv[4]));
   }
 } class_rtProtoOLSR;
 
 ///
 /// \brief Interface with TCL interpreter.
 ///
 /// From your TCL scripts or shell you can invoke commands on this OLSR
 /// routing agent thanks to this function. Currently you can call "start",
 /// "print_rtable", "print_linkset", "print_nbset", "print_nb2hopset",
 /// "print_mprset", "print_mprselset" and "print_topologyset" commands.
 ///
 /// \param argc Number of arguments.
 /// \param argv Arguments.
 /// \return TCL_OK or TCL_ERROR.
 ///
 int
 OLSR::command(int argc, const char*const* argv) {
   if (argc == 2) {
     // Starts all timers
     if (strcasecmp(argv[1], "start") == 0) {
       link_quality_timer_.resched(0.0);
       hello_timer_.resched(0.0);
       tc_timer_.resched(0.0);
       mid_timer_.resched(0.0);
 
       return TCL_OK;
     }
     // Prints routing table
     else if (strcasecmp(argv[1], "print_rtable") == 0) {
       if (logtarget_ != NULL) {
         sprintf(logtarget_->pt_->buffer(), "P %f _%d_ Routing Table",
           CURRENT_TIME, OLSR::node_id(ra_addr()));
         logtarget_->pt_->dump();
         rtable_.print(logtarget_);
       }
       else {
         fprintf(stdout, "%f _%d_ If you want to print this routing table "
           "you must create a trace file in your tcl script",
           CURRENT_TIME, OLSR::node_id(ra_addr()));
       }
       return TCL_OK;
     }
     // Prints link set
     else if (strcasecmp(argv[1], "print_linkset") == 0) {
       if (logtarget_ != NULL) {
         sprintf(logtarget_->pt_->buffer(), "P %f _%d_ Link Set",
           CURRENT_TIME, OLSR::node_id(ra_addr()));
         logtarget_->pt_->dump();
         OLSR_printer::print_linkset(logtarget_, linkset());
       }
       else {
         fprintf(stdout, "%f _%d_ If you want to print this link set "
           "you must create a trace file in your tcl script",
           CURRENT_TIME, OLSR::node_id(ra_addr()));
       }
       return TCL_OK;
     }
     // Prints neighbor set
     else if (strcasecmp(argv[1], "print_nbset") == 0) {
       if (logtarget_ != NULL) {
         sprintf(logtarget_->pt_->buffer(), "P %f _%d_ Neighbor Set",
           CURRENT_TIME, OLSR::node_id(ra_addr()));
         logtarget_->pt_->dump();
         OLSR_printer::print_nbset(logtarget_, nbset());
       }
       else {
         fprintf(stdout, "%f _%d_ If you want to print this neighbor set "
           "you must create a trace file in your tcl script",
           CURRENT_TIME, OLSR::node_id(ra_addr()));
       }
       return TCL_OK;
     }
     // Prints 2-hop neighbor set
     else if (strcasecmp(argv[1], "print_nb2hopset") == 0) {
       if (logtarget_ != NULL) {
         sprintf(logtarget_->pt_->buffer(), "P %f _%d_ Neighbor2hop Set",
           CURRENT_TIME, OLSR::node_id(ra_addr()));
         logtarget_->pt_->dump();
         OLSR_printer::print_nb2hopset(logtarget_, nb2hopset());
       }
       else {
         fprintf(stdout, "%f _%d_ If you want to print this neighbor2hop set "
           "you must create a trace file in your tcl script",
           CURRENT_TIME, OLSR::node_id(ra_addr()));
       }
       return TCL_OK;
     }
     // Prints MPR set
     else if (strcasecmp(argv[1], "print_mprset") == 0) {
       if (logtarget_ != NULL) {
         sprintf(logtarget_->pt_->buffer(), "P %f _%d_ MPR Set",
           CURRENT_TIME, OLSR::node_id(ra_addr()));
         logtarget_->pt_->dump();
         OLSR_printer::print_mprset(logtarget_, mprset());
       }
       else {
         fprintf(stdout, "%f _%d_ If you want to print this mpr set "
           "you must create a trace file in your tcl script",
           CURRENT_TIME, OLSR::node_id(ra_addr()));
       }
       return TCL_OK;
     }
     // Prints MPR selector set
     else if (strcasecmp(argv[1], "print_mprselset") == 0) {
       if (logtarget_ != NULL) {
         sprintf(logtarget_->pt_->buffer(), "P %f _%d_ MPR Selector Set",
           CURRENT_TIME, OLSR::node_id(ra_addr()));
         logtarget_->pt_->dump();
         OLSR_printer::print_mprselset(logtarget_, mprselset());
       }
       else {
         fprintf(stdout, "%f _%d_ If you want to print this mpr selector set "
           "you must create a trace file in your tcl script",
           CURRENT_TIME, OLSR::node_id(ra_addr()));
       }
       return TCL_OK;
     }
     // Prints topology set
     else if (strcasecmp(argv[1], "print_topologyset") == 0) {
       if (logtarget_ != NULL) {
         sprintf(logtarget_->pt_->buffer(), "P %f _%d_ Topology Set",
           CURRENT_TIME, OLSR::node_id(ra_addr()));
         logtarget_->pt_->dump();
         OLSR_printer::print_topologyset(logtarget_, topologyset());
       }
       else {
         fprintf(stdout, "%f _%d_ If you want to print this topology set "
           "you must create a trace file in your tcl script",
           CURRENT_TIME, OLSR::node_id(ra_addr()));
       }
       return TCL_OK;
     }
     
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
   }
   else if (argc == 3) {
     // Obtains the corresponding dmux to carry packets to upper layers
     if (strcmp(argv[1], "port-dmux") == 0) {
           dmux_ = (PortClassifier*)TclObject::lookup(argv[2]);
       if (dmux_ == NULL) {
         fprintf(stderr, "%s: %s lookup of %s failed\n", __FILE__, argv[1], argv[2]);
         return TCL_ERROR;
       }
       return TCL_OK;
         }
     // Obtains the corresponding tracer
     else if (strcmp(argv[1], "log-target") == 0 || strcmp(argv[1], "tracetarget") == 0) {
       logtarget_ = (Trace*)TclObject::lookup(argv[2]);
       if (logtarget_ == NULL)
         return TCL_ERROR;
       return TCL_OK;
     }
   }
   // Pass the command up to the base class
   return Agent::command(argc, argv);
 }
 
 
 /********** Timers **********/
 
 
 ///
 /// \brief Check link timeout.
 /// \param e The event which has expired.
 ///
 void
 OLSR_LinkQualityTimer::expire(Event* e) {
   agent_->link_quality();
   agent_->set_link_quality_timer();
 }
 
 ///
 /// \brief Sends a HELLO message and reschedules the HELLO timer.
 /// \param e The event which has expired.
 ///
 void
 OLSR_HelloTimer::expire(Event* e) {
   agent_->send_hello();
   agent_->set_hello_timer();
 }
 
 ///
 /// \brief Sends a TC message (if there exists any MPR selector) and reschedules the TC timer.
 /// \param e The event which has expired.
 ///
 void
 OLSR_TcTimer::expire(Event* e) {
   if (agent_->mprselset().size() > 0)
     agent_->send_tc();
   agent_->set_tc_timer();
 }
 
 ///
 /// \brief Sends a MID message (if the node has more than one interface) and resets the MID timer.
 /// \warning Currently it does nothing because there is no support for multiple interfaces.
 /// \param e The event which has expired.
 ///
 void
 OLSR_MidTimer::expire(Event* e) {
 #ifdef MULTIPLE_IFACES_SUPPORT
   agent_->send_mid();
   agent_->set_mid_timer();
 #endif
 }
 
 ///
 /// \brief Removes tuple_ if expired. Else timer is rescheduled to expire at tuple_->time().
 ///
 /// The task of actually removing the tuple is left to the OLSR agent.
 ///
 /// \param e The event which has expired.
 ///
 void
 OLSR_DupTupleTimer::expire(Event* e) {
   if (tuple_->time() < CURRENT_TIME) {
     agent_->rm_dup_tuple(tuple_);
     delete tuple_;
     delete this;
   }
   else
     resched(DELAY(tuple_->time()));
 }
 
 ///
 /// \brief Removes tuple_ if expired. Else if symmetric time
 /// has expired then it is assumed a neighbor loss and agent_->nb_loss()
 /// is called. In this case the timer is rescheduled to expire at
 /// tuple_->time(). Otherwise the timer is rescheduled to expire at
 /// the minimum between tuple_->time() and tuple_->sym_time().
 ///
 /// The task of actually removing the tuple is left to the OLSR agent.
 ///
 /// \param e The event which has expired.
 ///
 void
 OLSR_LinkTupleTimer::expire(Event* e) {
   double now = CURRENT_TIME;
 
   if (tuple_->time() < now) {
     agent_->rm_link_tuple(tuple_);
     delete tuple_;
     delete this;
   }
   else if (tuple_->sym_time() < now) {
     if (first_time_)
       first_time_ = false;
     else
       agent_->nb_loss(tuple_);
     resched(DELAY(tuple_->time()));
   }
   else
     resched(DELAY(MIN(tuple_->time(), tuple_->sym_time())));
 }
 
 ///
 /// \brief Removes tuple_ if expired. Else the timer is rescheduled to expire at tuple_->time().
 ///
 /// The task of actually removing the tuple is left to the OLSR agent.
 ///
 /// \param e The event which has expired.
 ///
 void
 OLSR_Nb2hopTupleTimer::expire(Event* e) {
   if (tuple_->time() < CURRENT_TIME) {
     agent_->rm_nb2hop_tuple(tuple_);
     delete tuple_;
     delete this;
   }
   else
     resched(DELAY(tuple_->time()));
 }
 
 ///
 /// \brief Removes tuple_ if expired. Else the timer is rescheduled to expire at tuple_->time().
 ///
 /// The task of actually removing the tuple is left to the OLSR agent.
 ///
 /// \param e The event which has expired.
 ///
 void
 OLSR_MprSelTupleTimer::expire(Event* e) {
   if (tuple_->time() < CURRENT_TIME) {
     agent_->rm_mprsel_tuple(tuple_);
     delete tuple_;
     delete this;
   }
   else
     resched(DELAY(tuple_->time()));
 }
 
 ///
 /// \brief Removes tuple_ if expired. Else the timer is rescheduled to expire at tuple_->time().
 ///
 /// The task of actually removing the tuple is left to the OLSR agent.
 ///
 /// \param e The event which has expired.
 ///
 void
 OLSR_TopologyTupleTimer::expire(Event* e) {
   if (tuple_->time() < CURRENT_TIME) {
     agent_->rm_topology_tuple(tuple_);
     delete tuple_;
     delete this;
   }
   else
     resched(DELAY(tuple_->time()));
 }
 
 ///
 /// \brief Removes tuple_ if expired. Else timer is rescheduled to expire at tuple_->time().
 /// \warning Actually this is never invoked because there is no support for multiple interfaces.
 /// \param e The event which has expired.
 ///
 void
 OLSR_IfaceAssocTupleTimer::expire(Event* e) {
   if (tuple_->time() < CURRENT_TIME) {
     agent_->rm_ifaceassoc_tuple(tuple_);
     delete tuple_;
     delete this;
   }
   else
     resched(DELAY(tuple_->time()));
 }
 
 ///
 /// \brief Sends a control packet which must bear every message in the OLSR agent's buffer.
 ///
 /// The task of actually sending the packet is left to the OLSR agent.
 ///
 /// \param e The event which has expired.
 ///
 void
 OLSR_MsgTimer::expire(Event* e) {
   agent_->send_pkt();
   delete this;
 }
 
 
 /********** OLSR class **********/
 
 
 ///
 /// \brief Creates necessary timers, binds TCL-available variables and do
 /// some more initializations.
 /// \param id Identifier for the OLSR agent. It will be used as the address
 /// of this routing agent.
 ///
 OLSR::OLSR(nsaddr_t id) :  Agent(PT_OLSR), link_quality_timer_ (this),
                                    hello_timer_(this), tc_timer_(this), mid_timer_(this) {
 
   // Enable usage of some of the configuration variables from Tcl.
   //
   // Note: Do NOT change the values of these variables in the constructor
   // after binding them! The desired default values should be set in
   // ns-X.XX/tcl/lib/ns-default.tcl instead.
   bind("willingness_", &willingness_);
   bind("hello_ival_", &hello_ival_);
   bind("tc_ival_", &tc_ival_);
   bind("mid_ival_", &mid_ival_);
   bind("mpr_algorithm_", &parameter_.mpr_algorithm());
   bind("routing_algorithm_", &parameter_.routing_algorithm());
   bind("link_quality_", &parameter_.link_quality());
   bind_bool("fish_eye_", &parameter_.fish_eye());
   bind("tc_redundancy_", &parameter_.tc_redundancy());
   bind_bool("use_mac_", &use_mac_);
 
   /// Link delay extension
   bind_bool("link_delay_", &parameter_.link_delay());
   bind("c_alpha_", &parameter_.c_alpha());
 
   // Fish Eye Routing Algorithm for TC message dispatching...
   tc_msg_ttl_index_ = 0;
 
   tc_msg_ttl_ [0] = 255;
   tc_msg_ttl_ [1] = 3;
   tc_msg_ttl_ [2] = 2;
   tc_msg_ttl_ [3] = 1;
   tc_msg_ttl_ [4] = 2;
   tc_msg_ttl_ [5] = 1;
   tc_msg_ttl_ [6] = 1;
   tc_msg_ttl_ [7] = 3;
   tc_msg_ttl_ [8] = 2;
   tc_msg_ttl_ [9] = 1;
   tc_msg_ttl_ [10] = 2;
   tc_msg_ttl_ [11] = 1;
   tc_msg_ttl_ [12] = 1;
 
   /// Link delay extension
   cap_sn_ = 0;
 
   // Do some initializations
   ra_addr_ = id;
   pkt_seq_ = OLSR_MAX_SEQ_NUM;
   msg_seq_ = OLSR_MAX_SEQ_NUM;
   ansn_ = OLSR_MAX_SEQ_NUM;
   
   //Selfish var starting
   selfish = false;
 }
 
 ///
 /// \brief  This function is called whenever a packet is received. It identifies
 ///    the type of the received packet and process it accordingly.
 ///
 /// If it is an %OLSR packet then it is processed. In other case, if it is a data packet
 /// then it is forwarded.
 ///
 /// \param  p the received packet.
 /// \param  h a handler (not used).
 ///
 void
 OLSR::recv(Packet* p, Handler* h) {
   struct hdr_cmn* ch = HDR_CMN(p);
   struct hdr_ip* ih = HDR_IP(p);
 
   if (ih->saddr() == ra_addr()) {
     // If there exists a loop, must drop the packet
     if (ch->num_forwards() > 0) {
       drop(p, DROP_RTR_ROUTE_LOOP);
       return;
     }
     // else if this is a packet I am originating, must add IP header
     else if (ch->num_forwards() == 0)
       ch->size() += IP_HDR_LEN;
   }
 
   // If it is an OLSR packet, must process it
   if (ch->ptype() == PT_OLSR)
     recv_olsr(p);
   // Otherwise, must forward the packet (unless TTL has reached zero)
   else {
     ih->ttl_--;
     if (ih->ttl_ == 0) {
       drop(p, DROP_RTR_TTL);
       return;
     }
     ///Set node's Behavior selfish - By Diógenes
     if((ih->saddr() != ra_addr()) && selfish == true){
		drop(p, DROP_RTR_SELFISH); //Set as "SEL" in the trace.
		return; 
      }
     forward_data(p);
   }
 }
 
 ///
 /// \brief Verify if a link tuple has reached a timeout in the expected time to receive a new packet
 ///
 void
 OLSR::link_quality () {
   double now = CURRENT_TIME;
   for (linkset_t::iterator it = state_.linkset_.begin(); it != state_.linkset_.end(); it++) {
     OLSR_link_tuple* tuple = *it;
     if (tuple->next_hello() < now)
       tuple->packet_timeout();
   }
 }
 
 ///
 /// \brief Processes an incoming %OLSR packet following RFC 3626 specification.
 /// \param p received packet.
 ///
 void
 OLSR::recv_olsr(Packet* p) {
   struct hdr_ip* ih = HDR_IP(p);
   OLSR_pkt* op = PKT_OLSR(p);
 
   // All routing messages are sent from and to port RT_PORT,
   // so we check it.
   assert(ih->sport() == RT_PORT);
   assert(ih->dport() == RT_PORT);
 
   // If the packet contains no messages must be silently discarded.
   // There could exist a message with an empty body, so the size of
   // the packet would be pkt-hdr-size + msg-hdr-size.
   if (op->pkt_len() < OLSR_PKT_HDR_SIZE + OLSR_MSG_HDR_SIZE) {
     Packet::free(p);
     return;
   }
 
   assert(op->count >= 0 && op->count <= OLSR_MAX_MSGS);
   for (int i = 0; i < op->count; i++) {
     OLSR_msg& msg = op->msg(i);
 
     // If ttl is less than or equal to zero, or
     // the receiver is the same as the originator,
     // the message must be silently dropped
     if (msg.ttl() <= 0 || msg.orig_addr() == ra_addr())
       continue;
 
     // If the message has been processed it must not be
     // processed again
     bool do_forwarding = true;
     OLSR_dup_tuple* duplicated = state_.find_dup_tuple(msg.orig_addr(), msg.msg_seq_num());
     if (duplicated == NULL) {
 
       // Process the message according to its type
       if (msg.msg_type() == OLSR_HELLO_MSG)
         process_hello(msg, ra_addr(), ih->saddr(), op->pkt_seq_num());
       else if (msg.msg_type() == OLSR_TC_MSG)
         process_tc(msg, ih->saddr());
       else if (msg.msg_type() == OLSR_MID_MSG)
         process_mid(msg, ih->saddr());
       else {
         debug("%f: Node %d can not process OLSR packet because does not "
                "implement OLSR type (%x)\n", CURRENT_TIME, OLSR::node_id(ra_addr()),
                msg.msg_type());
       }
     }
     else {
       // If the message has been considered for forwarding, it should
       // not be retransmitted again
       for (addr_list_t::iterator it = duplicated->iface_list().begin();
            it != duplicated->iface_list().end(); it++) {
         if (*it == ra_addr()) {
           do_forwarding = false;
           break;
         }
       }
     }
     if (do_forwarding) {
       // HELLO messages are never forwarded.
       // TC and MID messages are forwarded using the default algorithm.
       // Remaining messages are also forwarded using the default algorithm.
       if (msg.msg_type() != OLSR_HELLO_MSG)
         forward_default(p, msg, duplicated, ra_addr());
     }
 
   }
 
   /// Link delay extension
   if (parameter_.link_delay() && op->sn() > 0) {
     OLSR_link_tuple *link_tuple = state_.find_link_tuple (ih->saddr());
     if (link_tuple)
       link_tuple->link_delay_computation(op);
   }
 
   // After processing all OLSR messages, we must recompute routing table
   switch (parameter_.routing_algorithm()) {
   case OLSR_DIJKSTRA_ALGORITHM:
     rtable_dijkstra_computation();
     break;
 
   default:
   case OLSR_DEFAULT_ALGORITHM:
     rtable_default_computation();
     break;
   }
 
   // Release resources
   Packet::free(p);
 }
 
 ///
 /// \brief Computates MPR set of a node.
 ///
 void
 OLSR::olsr_mpr_computation() {
   // MPR computation should be done for each interface. See section 8.3.1
   // (RFC 3626) for details.
 
   state_.clear_mprset();
 
   nbset_t N; nb2hopset_t N2;
   // N is the subset of neighbors of the node, which are
   // neighbor "of the interface I"
   for (nbset_t::iterator it = nbset().begin(); it != nbset().end(); it++)
     if ((*it)->status() == OLSR_STATUS_SYM) // I think that we need this check
       N.push_back(*it);
 
   // N2 is the set of 2-hop neighbors reachable from "the interface
   // I", excluding:
   // (i)   the nodes only reachable by members of N with willingness WILL_NEVER
   // (ii)  the node performing the computation
   // (iii) all the symmetric neighbors: the nodes for which there exists a symmetric
   //       link to this node on some interface.
   for (nb2hopset_t::iterator it = nb2hopset().begin(); it != nb2hopset().end(); it++) {
     OLSR_nb2hop_tuple* nb2hop_tuple = *it;
     bool ok = true;
     OLSR_nb_tuple* nb_tuple = state_.find_sym_nb_tuple(nb2hop_tuple->nb_main_addr());
     if (nb_tuple == NULL)
       ok = false;
     else {
       nb_tuple = state_.find_nb_tuple(nb2hop_tuple->nb_main_addr(), OLSR_WILL_NEVER);
       if (nb_tuple != NULL)
         ok = false;
       else {
         nb_tuple = state_.find_sym_nb_tuple(nb2hop_tuple->nb2hop_addr());
         if (nb_tuple != NULL)
           ok = false;
       }
     }
 
     if (ok)
       N2.push_back(nb2hop_tuple);
   }
 
   // 1. Start with an MPR set made of all members of N with
   // N_willingness equal to WILL_ALWAYS
   for (nbset_t::iterator it = N.begin(); it != N.end(); it++) {
     OLSR_nb_tuple* nb_tuple = *it;
     if (nb_tuple->willingness() == OLSR_WILL_ALWAYS)
       state_.insert_mpr_addr(nb_tuple->nb_main_addr());
   }
 
   // 2. Calculate D(y), where y is a member of N, for all nodes in N.
   // We will do this later.
 
   // 3. Add to the MPR set those nodes in N, which are the *only*
   // nodes to provide reachability to a node in N2. Remove the
   // nodes from N2 which are now covered by a node in the MPR set.
   mprset_t foundset;
   std::set<nsaddr_t> deleted_addrs;
   // iterate through all 2 hop neighbors we have
   for (nb2hopset_t::iterator it = N2.begin(); it != N2.end(); it++) {
     OLSR_nb2hop_tuple* nb2hop_tuple1 = *it;
     // check if this two hop neighbor has more that one hop neighbor in N
     // it would mean that there is more than one node in N that reaches
     // the current 2 hop node
     mprset_t::iterator pos = foundset.find(nb2hop_tuple1->nb2hop_addr());
     if (pos != foundset.end())
       continue;
 
     bool found = false;
     // find the one hop neighbor that provides reachability to the
     // current two hop neighbor.
     for (nbset_t::iterator it2 = N.begin(); it2 != N.end(); it2++) {
       if ((*it2)->nb_main_addr() == nb2hop_tuple1->nb_main_addr()) {
         found = true;
         break;
       }
     }
     if (!found)
       continue;
 
     found = false;
     // check if there is another one hop neighbor able to provide
     // reachability to the current 2 hop neighbor
     for (nb2hopset_t::iterator it2 = it + 1; it2 != N2.end(); it2++) {
       OLSR_nb2hop_tuple* nb2hop_tuple2 = *it2;
       if (nb2hop_tuple1->nb2hop_addr() == nb2hop_tuple2->nb2hop_addr()) {
         foundset.insert(nb2hop_tuple1->nb2hop_addr());
         found = true;
         break;
       }
     }
     // if there is only one node, add our one hop neighbor to the MPR set
     if (!found) {
       state_.insert_mpr_addr(nb2hop_tuple1->nb_main_addr());
 
       // erase all 2 hop neighbor nodes that are now reached through this
       // newly added MPR
       for (nb2hopset_t::iterator it2 = it + 1; it2 != N2.end(); it2++) {
         OLSR_nb2hop_tuple* nb2hop_tuple2 = *it2;
         if (nb2hop_tuple1->nb_main_addr() == nb2hop_tuple2->nb_main_addr()) {
           deleted_addrs.insert(nb2hop_tuple2->nb2hop_addr());
           it2 = N2.erase(it2);
           it2--;
         }
       }
       it = N2.erase(it);
       it--;
     }
 
     // erase all 2 hop neighbor nodes that are now reached through this
     // newly added MPR. We are now looking for the backup links
     for (std::set<nsaddr_t>::iterator it2 = deleted_addrs.begin();
          it2 != deleted_addrs.end(); it2++) {
       for (nb2hopset_t::iterator it3 = N2.begin(); it3 != N2.end(); it3++) {
         if ((*it3)->nb2hop_addr() == *it2) {
           it3 = N2.erase(it3);
           it3--;
           // I have to reset the external iterator because it
           // may have been invalidated by the latter deletion
           it = N2.begin();
           it--;
         }
       }
     }
     deleted_addrs.clear();
   }
 
   // 4. While there exist nodes in N2 which are not covered by at
   // least one node in the MPR set:
   while (N2.begin() != N2.end()) {
     // 4.1. For each node in N, calculate the reachability, i.e., the
     // number of nodes in N2 which are not yet covered by at
     // least one node in the MPR set, and which are reachable
     // through this 1-hop neighbor
     map<int, std::vector<OLSR_nb_tuple*> > reachability;
     set<int> rs;
     set<int> r_mpr;
     for (nbset_t::iterator it = N.begin(); it != N.end(); it++) {
       OLSR_nb_tuple* nb_tuple = *it;
       int r = 0;
       for (nb2hopset_t::iterator it2 = N2.begin(); it2 != N2.end(); it2++) {
         OLSR_nb2hop_tuple* nb2hop_tuple = *it2;
         if (nb_tuple->nb_main_addr() == nb2hop_tuple->nb_main_addr())
           r++;
       }
       rs.insert(r);
       reachability[r].push_back(nb_tuple);
     }
 
     // 4.2. Select as a MPR the node with highest N_willingness among
     // the nodes in N with non-zero reachability. In case of
     // multiple choice select the node which provides
     // reachability to the maximum number of nodes in N2. In
     // case of multiple nodes providing the same amount of
     // reachability, select the node as MPR whose D(y) is
     // greater. Remove the nodes from N2 which are now covered
     // by a node in the MPR set.
     OLSR_nb_tuple* max = NULL;
     int max_r = 0;
     for (set<int>::iterator it = rs.begin(); it != rs.end(); it++) {
       int r = *it;
       if (r > 0) {
         for (std::vector<OLSR_nb_tuple*>::iterator it2 = reachability[r].begin();
              it2 != reachability[r].end(); it2++) {
           OLSR_nb_tuple* nb_tuple = *it2;
           if (max == NULL || nb_tuple->willingness() > max->willingness()) {
             max = nb_tuple;
             max_r = r;
           }
           else if (nb_tuple->willingness() == max->willingness()) {
             if (r > max_r) {
               max = nb_tuple;
               max_r = r;
             }
             else if (r == max_r) {
               r_mpr.insert(max_r);
               if (degree(nb_tuple) > degree(max)) {
                 max = nb_tuple;
                 max_r = r;
               } else
                  
             }
           }
         }
       }
     }
     if (max != NULL) {
       state_.insert_mpr_addr(max->nb_main_addr());
       std::set<nsaddr_t> nb2hop_addrs;
       for (nb2hopset_t::iterator it = N2.begin(); it != N2.end(); it++) {
         OLSR_nb2hop_tuple* nb2hop_tuple = *it;
         if (nb2hop_tuple->nb_main_addr() == max->nb_main_addr()) {
           nb2hop_addrs.insert(nb2hop_tuple->nb2hop_addr());
           it = N2.erase(it);
           it--;
         }
       }
       for (nb2hopset_t::iterator it = N2.begin(); it != N2.end(); it++) {
         OLSR_nb2hop_tuple* nb2hop_tuple = *it;
         std::set<nsaddr_t>::iterator it2 =
           nb2hop_addrs.find(nb2hop_tuple->nb2hop_addr());
         if (it2 != nb2hop_addrs.end()) {
           it = N2.erase(it);
           it--;
         }
       }
     }
   }
 }
 
 ///
 /// \brief Computates MPR set of a node.
 ///
 void
 OLSR::olsr_r1_mpr_computation() {
   // For further details please refer to paper
   // Quality of Service Routing in Ad Hoc Networks Using OLSR
 
   state_.clear_mprset();
 
   nbset_t N; nb2hopset_t N2;
   // N is the subset of neighbors of the node, which are
   // neighbor "of the interface I" and have willigness different
   // from OLSR_WILL_NEVER
   for (nbset_t::iterator it = nbset().begin(); it != nbset().end(); it++)
     if ((*it)->status() == OLSR_STATUS_SYM) // I think that we need this check
       N.push_back(*it);
 
   // N2 is the set of 2-hop neighbors reachable from "the interface
   // I", excluding:
   // (i)   the nodes only reachable by members of N with willingness WILL_NEVER
   // (ii)  the node performing the computation
   // (iii) all the symmetric neighbors: the nodes for which there exists a symmetric
   //       link to this node on some interface.
   for (nb2hopset_t::iterator it = nb2hopset().begin(); it != nb2hopset().end(); it++) {
     OLSR_nb2hop_tuple* nb2hop_tuple = *it;
     bool ok = true;
     OLSR_nb_tuple* nb_tuple = state_.find_sym_nb_tuple(nb2hop_tuple->nb_main_addr());
     if (nb_tuple == NULL)
       ok = false;
     else {
       nb_tuple = state_.find_nb_tuple(nb2hop_tuple->nb_main_addr(), OLSR_WILL_NEVER);
       if (nb_tuple != NULL)
         ok = false;
       else {
         nb_tuple = state_.find_sym_nb_tuple(nb2hop_tuple->nb2hop_addr());
         if (nb_tuple != NULL)
           ok = false;
       }
     }
 
     if (ok)
       N2.push_back(nb2hop_tuple);
   }
 
   // Start with an MPR set made of all members of N with
   // N_willingness equal to WILL_ALWAYS
   for (nbset_t::iterator it = N.begin(); it != N.end(); it++) {
     OLSR_nb_tuple* nb_tuple = *it;
     if (nb_tuple->willingness() == OLSR_WILL_ALWAYS)
       state_.insert_mpr_addr(nb_tuple->nb_main_addr());
   }
 
   // Add to Mi the nodes in N which are the only nodes to provide reachability
   // to a node in N2. Remove the nodes from N2 which are now covered by
   // a node in the MPR set.
   mprset_t foundset;
   std::set<nsaddr_t> deleted_addrs;
   // iterate through all 2 hop neighbors we have
   for (nb2hopset_t::iterator it = N2.begin(); it != N2.end(); it++) {
     OLSR_nb2hop_tuple* nb2hop_tuple1 = *it;
     // check if this two hop neighbor has more that one hop neighbor in N
     // it would mean that there is more than one node in N that reaches
     // the current 2 hop node
     mprset_t::iterator pos = foundset.find(nb2hop_tuple1->nb2hop_addr());
     if (pos != foundset.end())
       continue;
 
     bool found = false;
     // find the one hop neighbor that provides reachability to the
     // current two hop neighbor.
     for (nbset_t::iterator it2 = N.begin(); it2 != N.end(); it2++) {
       if ((*it2)->nb_main_addr() == nb2hop_tuple1->nb_main_addr()) {
         found = true;
         break;
       }
     }
     if (!found)
       continue;
 
     found = false;
     // check if there is another one hop neighbor able to provide
     // reachability to the current 2 hop neighbor
     for (nb2hopset_t::iterator it2 = it + 1; it2 != N2.end(); it2++) {
       OLSR_nb2hop_tuple* nb2hop_tuple2 = *it2;
       if (nb2hop_tuple1->nb2hop_addr() == nb2hop_tuple2->nb2hop_addr()) {
         foundset.insert(nb2hop_tuple1->nb2hop_addr());
         found = true;
         break;
       }
     }
     // if there is only one node, add our one hop neighbor to the MPR set
     if (!found) {
       state_.insert_mpr_addr(nb2hop_tuple1->nb_main_addr());
 
       // erase all 2 hop neighbor nodes that are now reached through this
       // newly added MPR
       for (nb2hopset_t::iterator it2 = it + 1; it2 != N2.end(); it2++) {
         OLSR_nb2hop_tuple* nb2hop_tuple2 = *it2;
         if (nb2hop_tuple1->nb_main_addr() == nb2hop_tuple2->nb_main_addr()) {
           deleted_addrs.insert(nb2hop_tuple2->nb2hop_addr());
           it2 = N2.erase(it2);
           it2--;
         }
       }
       it = N2.erase(it);
       it--;
     }
 
     // erase all 2 hop neighbor nodes that are now reached through this
     // newly added MPR. We are now looking for the backup links
     for (std::set<nsaddr_t>::iterator it2 = deleted_addrs.begin();
          it2 != deleted_addrs.end(); it2++) {
       for (nb2hopset_t::iterator it3 = N2.begin(); it3 != N2.end(); it3++) {
         if ((*it3)->nb2hop_addr() == *it2) {
           it3 = N2.erase(it3);
           it3--;
           // I have to reset the external iterator because it
           // may have been invalidated by the latter deletion
           it = N2.begin();
           it--;
         }
       }
     }
     deleted_addrs.clear();
   }
 
   // While there exist nodes in N2 which are not covered by at
   // least one node in the MPR set:
   while (N2.begin() != N2.end()) {
     // For each node in N, calculate the reachability, i.e., the
     // number of nodes in N2 that it can reach
     map<int, std::vector<OLSR_nb_tuple*> > reachability;
     set<int> rs;
     for (nbset_t::iterator it = N.begin(); it != N.end(); it++) {
       OLSR_nb_tuple* nb_tuple = *it;
       int r = 0;
       for (nb2hopset_t::iterator it2 = N2.begin(); it2 != N2.end(); it2++) {
         OLSR_nb2hop_tuple* nb2hop_tuple = *it2;
         if (nb_tuple->nb_main_addr() == nb2hop_tuple->nb_main_addr())
           r++;
       }
       rs.insert(r);
       reachability[r].push_back(nb_tuple);
     }
 
     // Select as a MPR the node with highest N_willingness among
     // the nodes in N with non-zero reachability. In case of
     // multiple choice select the node which provides
     // reachability to the maximum number of nodes in N2. In
     // case of multiple choices select the node with best conectivity
     // to the current node. Remove the nodes from N2 which are now covered
     // by a node in the MPR set.
     OLSR_nb_tuple* max = NULL;
     int max_r = 0;
     for (set<int>::iterator it = rs.begin(); it != rs.end(); it++) {
       int r = *it;
       if (r > 0) {
         for (std::vector<OLSR_nb_tuple*>::iterator it2 = reachability[r].begin();
              it2 != reachability[r].end(); it2++) {
           OLSR_nb_tuple* nb_tuple = *it2;
           if (max == NULL || nb_tuple->willingness() > max->willingness()) {
             max = nb_tuple;
             max_r = r;
           }
           else if (nb_tuple->willingness() == max->willingness()) {
             if (r > max_r) {
               max = nb_tuple;
               max_r = r;
             }
             else if (r == max_r) {
               OLSR_link_tuple *nb_link_tuple, *max_link_tuple;
               double now = CURRENT_TIME;
 
               nb_link_tuple = state_.find_sym_link_tuple (nb_tuple->nb_main_addr(), now);
               max_link_tuple = state_.find_sym_link_tuple (max->nb_main_addr(), now);
               if (nb_link_tuple || max_link_tuple)
                 continue;
               if (parameter_.link_delay()) {
                 if (nb_link_tuple->link_delay() < max_link_tuple->link_delay()) {
                   max = nb_tuple;
                   max_r = r;
                 }
               } else {
                 switch (parameter_.link_quality()) {
                 case OLSR_BEHAVIOR_ETX:
                   if (nb_link_tuple->etx() < max_link_tuple->etx()) {
                     max = nb_tuple;
                     max_r = r;
                   }
                   break;
 
                 case OLSR_BEHAVIOR_ML:
                   if (nb_link_tuple->etx() > max_link_tuple->etx()) {
                     max = nb_tuple;
                     max_r = r;
                   }
                   break;
 
                 case OLSR_BEHAVIOR_NONE:
                 default:
                   // max = nb_tuple;
                   // max_r = r;
                   break;
                 }
               }
             }
           }
         }
       }
     }
     if (max != NULL) {
       state_.insert_mpr_addr(max->nb_main_addr());
       std::set<nsaddr_t> nb2hop_addrs;
       for (nb2hopset_t::iterator it = N2.begin(); it != N2.end(); it++) {
         OLSR_nb2hop_tuple* nb2hop_tuple = *it;
         if (nb2hop_tuple->nb_main_addr() == max->nb_main_addr()) {
           nb2hop_addrs.insert(nb2hop_tuple->nb2hop_addr());
           it = N2.erase(it);
           it--;
         }
       }
       for (nb2hopset_t::iterator it = N2.begin(); it != N2.end(); it++) {
         OLSR_nb2hop_tuple* nb2hop_tuple = *it;
         std::set<nsaddr_t>::iterator it2 =
           nb2hop_addrs.find(nb2hop_tuple->nb2hop_addr());
         if (it2 != nb2hop_addrs.end()) {
           it = N2.erase(it);
           it--;
         }
       }
     }
   }
 }
 
 ///
 /// \brief Computates MPR set of a node.
 ///
 void
 OLSR::olsr_r2_mpr_computation() {
   // For further details please refer to paper
   // Quality of Service Routing in Ad Hoc Networks Using OLSR
 
   state_.clear_mprset();
 
   nbset_t N; nb2hopset_t N2;
   // N is the subset of neighbors of the node, which are
   // neighbor "of the interface I"
   for (nbset_t::iterator it = nbset().begin(); it != nbset().end(); it++)
     if ((*it)->status() == OLSR_STATUS_SYM &&
          (*it)->willingness() != OLSR_WILL_NEVER) // I think that we need this check
       N.push_back(*it);
 
   // N2 is the set of 2-hop neighbors reachable from "the interface
   // I", excluding:
   // (i)   the nodes only reachable by members of N with willingness WILL_NEVER
   // (ii)  the node performing the computation
   // (iii) all the symmetric neighbors: the nodes for which there exists a symmetric
   //       link to this node on some interface.
   for (nb2hopset_t::iterator it = nb2hopset().begin(); it != nb2hopset().end(); it++) {
     OLSR_nb2hop_tuple* nb2hop_tuple = *it;
     bool ok = true;
     OLSR_nb_tuple* nb_tuple = state_.find_sym_nb_tuple(nb2hop_tuple->nb_main_addr());
     if (nb_tuple == NULL)
       ok = false;
     else {
       nb_tuple = state_.find_nb_tuple(nb2hop_tuple->nb_main_addr(), OLSR_WILL_NEVER);
       if (nb_tuple != NULL)
         ok = false;
       else {
         nb_tuple = state_.find_sym_nb_tuple(nb2hop_tuple->nb2hop_addr());
         if (nb_tuple != NULL)
           ok = false;
       }
     }
 
     if (ok)
       N2.push_back(nb2hop_tuple);
   }
 
   // While there exist nodes in N2 which are not covered by at
   // least one node in the MPR set:
   while (N2.begin() != N2.end()) {
     // For each node in N, calculate the reachability, i.e., the
     // number of nodes in N2 that it can reach
     map<int, std::vector<OLSR_nb_tuple*> > reachability;
     set<int> rs;
     for (nbset_t::iterator it = N.begin(); it != N.end(); it++) {
       OLSR_nb_tuple* nb_tuple = *it;
       int r = 0;
       for (nb2hopset_t::iterator it2 = N2.begin(); it2 != N2.end(); it2++) {
         OLSR_nb2hop_tuple* nb2hop_tuple = *it2;
         if (nb_tuple->nb_main_addr() == nb2hop_tuple->nb_main_addr())
           r++;
       }
       rs.insert(r);
       reachability[r].push_back(nb_tuple);
     }
 
     // Add to Mi the node in N that has the best link to the current
     // node. In case of tie, select tin N2. Remove the nodes from N2
     // which are now covered by a node in the MPR set.
     OLSR_nb_tuple* max = NULL;
     int max_r = 0;
     for (set<int>::iterator it = rs.begin(); it != rs.end(); it++) {
       int r = *it;
       if (r > 0) {
         for (std::vector<OLSR_nb_tuple*>::iterator it2 = reachability[r].begin();
              it2 != reachability[r].end(); it2++) {
           OLSR_nb_tuple* nb_tuple = *it2;
           if (max == NULL) {
             max = nb_tuple;
             max_r = r;
           }
           else {
             OLSR_link_tuple *nb_link_tuple, *max_link_tuple;
             double now = CURRENT_TIME;
 
             nb_link_tuple = state_.find_sym_link_tuple (nb_tuple->nb_main_addr(), now);
             max_link_tuple = state_.find_sym_link_tuple (max->nb_main_addr(), now);
             if (nb_link_tuple || max_link_tuple)
               continue;
             switch (parameter_.link_quality()) {
             case OLSR_BEHAVIOR_ETX:
               if (nb_link_tuple->etx() < max_link_tuple->etx()) {
                 max = nb_tuple;
                 max_r = r;
               }
               else if (nb_link_tuple->etx() == max_link_tuple->etx()) {
                 if (r > max_r) {
                   max = nb_tuple;
                   max_r = r;
                 }
                 else if (r == max_r && degree(nb_tuple) > degree (max)) {
                   max = nb_tuple;
                   max_r = r;
                 }
               }
               break;
 
             case OLSR_BEHAVIOR_ML:
               if (nb_link_tuple->etx() > max_link_tuple->etx()) {
                 max = nb_tuple;
                 max_r = r;
               }
               else if (nb_link_tuple->etx() == max_link_tuple->etx()) {
                 if (r > max_r) {
                   max = nb_tuple;
                   max_r = r;
                 }
                 else if (r == max_r && degree(nb_tuple) > degree (max)) {
                   max = nb_tuple;
                   max_r = r;
                 }
               }
               break;
 
             case OLSR_BEHAVIOR_NONE:
             default:
               if (r > max_r) {
                 max = nb_tuple;
                 max_r = r;
               }
               else if (r == max_r && degree(nb_tuple) > degree (max)) {
                 max = nb_tuple;
                 max_r = r;
               }
               break;
             }
           }
         }
       }
     }
     if (max != NULL) {
       state_.insert_mpr_addr(max->nb_main_addr());
       std::set<nsaddr_t> nb2hop_addrs;
       for (nb2hopset_t::iterator it = N2.begin(); it != N2.end(); it++) {
         OLSR_nb2hop_tuple* nb2hop_tuple = *it;
         if (nb2hop_tuple->nb_main_addr() == max->nb_main_addr()) {
           nb2hop_addrs.insert(nb2hop_tuple->nb2hop_addr());
           it = N2.erase(it);
           it--;
         }
       }
       for (nb2hopset_t::iterator it = N2.begin(); it != N2.end(); it++) {
         OLSR_nb2hop_tuple* nb2hop_tuple = *it;
         std::set<nsaddr_t>::iterator it2 =
           nb2hop_addrs.find(nb2hop_tuple->nb2hop_addr());
         if (it2 != nb2hop_addrs.end()) {
           it = N2.erase(it);
           it--;
         }
       }
     }
   }
 }
 
 ///
 /// \brief Computates MPR set of a node.
 ///
 void
 OLSR::qolsr_mpr_computation() {
   // For further details please refer to paper
   // QoS Routing in OLSR with Several Classes of Services
 
   state_.clear_mprset();
 
   nbset_t N; nb2hopset_t N2;
   // N is the subset of neighbors of the node, which are
   // neighbor "of the interface I"
   for (nbset_t::iterator it = nbset().begin(); it != nbset().end(); it++)
     if ((*it)->status() == OLSR_STATUS_SYM &&
          (*it)->willingness() != OLSR_WILL_NEVER) // I think that we need this check
       N.push_back(*it);
 
   // N2 is the set of 2-hop neighbors reachable from "the interface
   // I", excluding:
   // (i)   the nodes only reachable by members of N with willingness WILL_NEVER
   // (ii)  the node performing the computation
   // (iii) all the symmetric neighbors: the nodes for which there exists a symmetric
   //       link to this node on some interface.
   for (nb2hopset_t::iterator it = nb2hopset().begin(); it != nb2hopset().end(); it++) {
     OLSR_nb2hop_tuple* nb2hop_tuple = *it;
     bool ok = true;
     OLSR_nb_tuple* nb_tuple = state_.find_sym_nb_tuple(nb2hop_tuple->nb_main_addr());
     if (nb_tuple == NULL)
       ok = false;
     else {
       nb_tuple = state_.find_nb_tuple(nb2hop_tuple->nb_main_addr(), OLSR_WILL_NEVER);
       if (nb_tuple != NULL)
         ok = false;
       else {
         nb_tuple = state_.find_sym_nb_tuple(nb2hop_tuple->nb2hop_addr());
         if (nb_tuple != NULL)
           ok = false;
       }
     }
 
     if (ok)
       N2.push_back(nb2hop_tuple);
   }
 
   // While there exist nodes in N2 which are not covered by at
   // least one node in the MPR set:
   while (N2.begin() != N2.end()) {
     // For each node in N, calculate the reachability, i.e., the
     // number of nodes in N2 that it can reach
     map<int, std::vector<OLSR_nb_tuple*> > reachability;
     set<int> rs;
     for (nbset_t::iterator it = N.begin(); it != N.end(); it++) {
       OLSR_nb_tuple* nb_tuple = *it;
       int r = 0;
       for (nb2hopset_t::iterator it2 = N2.begin(); it2 != N2.end(); it2++) {
         OLSR_nb2hop_tuple* nb2hop_tuple = *it2;
         if (nb_tuple->nb_main_addr() == nb2hop_tuple->nb_main_addr())
           r++;
       }
       rs.insert(r);
       reachability[r].push_back(nb_tuple);
     }
 
     // Select a node z from N2
     OLSR_nb2hop_tuple* z = *(N2.begin());
     // Add to Mi, if not yet present, the node in N that provides the
     // shortest-widest path to reach z. In case of tie, select the node
     // that reaches the maximum number of nodes in N2. Remove the nodes from N2
     // which are now covered by a node in the MPR set.
 
     OLSR_nb_tuple* max = NULL;
     int max_r = 0;
 
     // Iterate through all links in nb2hop_set that has the same two hop
     // neighbor as the second point of the link
     for (nb2hopset_t::iterator it = N2.begin(); it != N2.end(); it++) {
       OLSR_nb2hop_tuple* nb2hop_tuple = *it;
       // If the two hop neighbor is not the one we have selected, skip
       if (nb2hop_tuple->nb2hop_addr() != z->nb2hop_addr())
         continue;
       // Compare the one hop neighbor that reaches the two hop neighbor z with
       for (set<int>::iterator it2 = rs.begin(); it2 != rs.end(); it2++) {
         int r = *it2;
         if (r > 0) {
           for (std::vector<OLSR_nb_tuple*>::iterator it3 = reachability[r].begin();
                it3 != reachability[r].end(); it3++) {
             OLSR_nb_tuple* nb_tuple = *it3;
             if (nb2hop_tuple->nb_main_addr() != nb_tuple->nb_main_addr())
               continue;
             if (max == NULL) {
               max = nb_tuple;
               max_r = r;
             }
             else {
               OLSR_link_tuple *nb_link_tuple, *max_link_tuple;
               double now = CURRENT_TIME;
 
               nb_link_tuple = state_.find_sym_link_tuple (nb_tuple->nb_main_addr(), now); /* bug */
               max_link_tuple = state_.find_sym_link_tuple (max->nb_main_addr(), now); /* bug */
 
               double current_total_etx, max_total_etx;
 
               switch (parameter_.link_quality()) {
               case OLSR_BEHAVIOR_ETX:
                 current_total_etx = nb_link_tuple->etx() + nb2hop_tuple->etx();
                 max_total_etx = max_link_tuple->etx() + nb2hop_tuple->etx();
                 if (current_total_etx < max_total_etx) {
                   max = nb_tuple;
                   max_r = r;
                 }
                 else if (current_total_etx == max_total_etx) {
                   if (r > max_r) {
                     max = nb_tuple;
                     max_r = r;
                   }
                   else if (r == max_r && degree(nb_tuple) > degree (max)) {
                     max = nb_tuple;
                     max_r = r;
                   }
                 }
                 break;
 
               case OLSR_BEHAVIOR_ML:
                 current_total_etx = nb_link_tuple->etx() * nb2hop_tuple->etx();
                 max_total_etx = max_link_tuple->etx() * nb2hop_tuple->etx();
                 if (current_total_etx > max_total_etx) {
                   max = nb_tuple;
                   max_r = r;
                 }
                 else if (current_total_etx == max_total_etx) {
                   if (r > max_r) {
                     max = nb_tuple;
                     max_r = r;
                   }
                   else if (r == max_r && degree(nb_tuple) > degree (max)) {
                     max = nb_tuple;
                     max_r = r;
                   }
                 }
                 break;
 
               case OLSR_BEHAVIOR_NONE:
               default:
                 if (r > max_r) {
                   max = nb_tuple;
                   max_r = r;
                 }
                 else if (r == max_r && degree(nb_tuple) > degree (max)) {
                   max = nb_tuple;
                   max_r = r;
                 }
                 break;
               }
             }
           }
         }
       }
     }
 
     if (max != NULL) {
       state_.insert_mpr_addr(max->nb_main_addr());
       std::set<nsaddr_t> nb2hop_addrs;
       for (nb2hopset_t::iterator it = N2.begin(); it != N2.end(); it++) {
         OLSR_nb2hop_tuple* nb2hop_tuple = *it;
         if (nb2hop_tuple->nb_main_addr() == max->nb_main_addr()) {
           nb2hop_addrs.insert(nb2hop_tuple->nb2hop_addr());
           it = N2.erase(it);
           it--;
         }
       }
       for (nb2hopset_t::iterator it = N2.begin(); it != N2.end(); it++) {
         OLSR_nb2hop_tuple* nb2hop_tuple = *it;
         std::set<nsaddr_t>::iterator it2 =
           nb2hop_addrs.find(nb2hop_tuple->nb2hop_addr());
         if (it2 != nb2hop_addrs.end()) {
           it = N2.erase(it);
           it--;
         }
       }
     }
   }
 }
 
 ///
 /// \brief Computates MPR set of a node.
 ///
 void
 OLSR::olsrd_mpr_computation() {
   // MPR computation algorithm according to olsrd project: all nodes will be selected
   // as MPRs, since they have WILLIGNESS different of WILL_NEVER
 
   state_.clear_mprset();
 
   // Build a MPR set made of all members of N with
   // N_willingness different of WILL_NEVER
   for (nbset_t::iterator it = nbset().begin(); it != nbset().end(); it++)
     {
       OLSR_nb_tuple* nb_tuple = *it;
       if (nb_tuple->willingness() != OLSR_WILL_NEVER &&
            nb_tuple->status() == OLSR_STATUS_SYM)
         state_.insert_mpr_addr(nb_tuple->nb_main_addr());
     }
 }
 
 ///
 /// \brief Creates the routing table of the node following RFC 3626 hints.
 ///
 void
 OLSR::rtable_default_computation() {
   // 1. All the entries from the routing table are removed.
   rtable_.clear();
 
   // 2. The new routing entries are added starting with the
   // symmetric neighbors (h=1) as the destination nodes.
 
   // iterate through all 1 hop neighbors we have
   for (nbset_t::iterator it = nbset().begin(); it != nbset().end(); it++) {
     OLSR_nb_tuple* nb_tuple = *it;
     // if the link is still symmetric...
     if (nb_tuple->status() == OLSR_STATUS_SYM) {
       bool nb_main_addr = false;
       OLSR_link_tuple* lt = NULL;
       // check all links in the link set that leads to the current 1 hop neighbor
       // and that have not expired yet...
       for (linkset_t::iterator it2 = linkset().begin(); it2 != linkset().end(); it2++) {
         OLSR_link_tuple* link_tuple = *it2;
         if (get_main_addr(link_tuple->nb_iface_addr()) == nb_tuple->nb_main_addr() &&
              link_tuple->time() >= CURRENT_TIME) {
           lt = link_tuple;
           // add this to the routing table as having cost 1...
           rtable_.add_entry(link_tuple->nb_iface_addr(),
               link_tuple->nb_iface_addr(), link_tuple->local_iface_addr(), 1);
           // iff this link is the link to our neighbor's main interface,
           // record this information...
           if (link_tuple->nb_iface_addr() == nb_tuple->nb_main_addr())
             nb_main_addr = true;
         }
       }
       // also add a route to our neighbor's main address, if not added yet...
       if (!nb_main_addr && lt != NULL) {
         rtable_.add_entry(nb_tuple->nb_main_addr(),
             lt->nb_iface_addr(), lt->local_iface_addr(), 1);
       }
     }
   }
 
   // N2 is the set of 2-hop neighbors reachable from this node, excluding:
   // (i)   the nodes only reachable by members of N with willingness WILL_NEVER
   // (ii)  the node performing the computation
   // (iii) all the symmetric neighbors: the nodes for which there exists a symmetric
   //       link to this node on some interface.
   for (nb2hopset_t::iterator it = nb2hopset().begin(); it != nb2hopset().end(); it++) {
     OLSR_nb2hop_tuple* nb2hop_tuple = *it;
     bool ok = true;
     OLSR_nb_tuple* nb_tuple = state_.find_sym_nb_tuple(nb2hop_tuple->nb_main_addr());
     if (nb_tuple == NULL)
       ok = false;
     else {
       nb_tuple = state_.find_nb_tuple(nb2hop_tuple->nb_main_addr(), OLSR_WILL_NEVER);
       if (nb_tuple != NULL)
         ok = false;
       else {
         nb_tuple = state_.find_sym_nb_tuple(nb2hop_tuple->nb2hop_addr());
         if (nb_tuple != NULL)
           ok = false;
       }
     }
 
     // 3. For each node in N2 create a new entry in the routing table
     if (ok) {
       OLSR_rt_entry* entry = rtable_.lookup(nb2hop_tuple->nb_main_addr());
       assert(entry != NULL);
       // these routes newly added now have cost 2..
       rtable_.add_entry(nb2hop_tuple->nb2hop_addr(),
           entry->next_addr(), entry->iface_addr(), 2);
     }
   }
 
   for (u_int32_t h = 2; ; h++) {
     bool added = false;
 
     // 4.1. For each topology entry in the topology table, if its
     // T_dest_addr does not correspond to R_dest_addr of any
     // route entry in the routing table AND its T_last_addr
     // corresponds to R_dest_addr of a route entry whose R_dist
     // is equal to h, then a new route entry MUST be recorded in
     // the routing table (if it does not already exist)
     for (topologyset_t::iterator it = topologyset().begin();
           it != topologyset().end(); it++) {
       OLSR_topology_tuple* topology_tuple = *it;
       OLSR_rt_entry* entry1 = rtable_.lookup(topology_tuple->dest_addr());
       OLSR_rt_entry* entry2 = rtable_.lookup(topology_tuple->last_addr());
       if (entry1 == NULL && entry2 != NULL && entry2->dist() == h) {
         rtable_.add_entry(topology_tuple->dest_addr(),
                            entry2->next_addr(), entry2->iface_addr(), h+1);
         added = true;
       }
     }
 
     // 5. For each entry in the multiple interface association base
     // where there exists a routing entry such that:
     //  R_dest_addr  == I_main_addr  (of the multiple interface association entry)
     // AND there is no routing entry such that:
     //  R_dest_addr  == I_iface_addr
     // then a route entry is created in the routing table
     for (ifaceassocset_t::iterator it = ifaceassocset().begin();
           it != ifaceassocset().end(); it++) {
       OLSR_iface_assoc_tuple* tuple = *it;
       OLSR_rt_entry* entry1 = rtable_.lookup(tuple->main_addr());
       OLSR_rt_entry* entry2 = rtable_.lookup(tuple->iface_addr());
       if (entry1 != NULL && entry2 == NULL) {
         rtable_.add_entry(tuple->iface_addr(),
             entry1->next_addr(), entry1->iface_addr(), entry1->dist());
         added = true;
       }
     }
 
     if (!added)
       break;
   }
 }
 
 ///
 /// \brief Creates the routing table of the node using dijkstra algorithm
 ///
 void
 OLSR::rtable_dijkstra_computation() {
   // Declare a class that will run the dijkstra algorithm
   Dijkstra * dijkstra = new Dijkstra ();
 
   debug ("Current node %d:\n", ra_addr());
   // Iterate through all out 1 hop neighbors
   for (nbset_t::iterator it = nbset().begin(); it != nbset().end(); it++) {
     OLSR_nb_tuple* nb_tuple = *it;
 
     // Get the best link we have to the current neighbor..
     OLSR_link_tuple* best_link =
       state_.find_best_sym_link_tuple (nb_tuple->nb_main_addr(), CURRENT_TIME);
     // Add this edge to the graph we are building
     if (best_link) {
       debug ("nb_tuple: %d (local) ==> %d , delay %lf, quality %lf\n", best_link->local_iface_addr(),
                nb_tuple->nb_main_addr(), best_link->nb_link_delay(), best_link->etx());
       dijkstra->add_edge (nb_tuple->nb_main_addr(), best_link->local_iface_addr(),
                           best_link->nb_link_delay(), best_link->etx(), true);
     }
   }
 
   // N (set of 1-hop neighbors) is the set of nodes reachable through a symmetric
   // link with willingness different of WILL_NEVER. The vector at each position
   // is a list of the best links connecting the one hop neighbor to a 2 hop neighbor
   // Note: we are not our own two hop neighbor
   map<nsaddr_t, std::vector<OLSR_nb2hop_tuple*> > N;
   set<nsaddr_t> N_index;
   for (nb2hopset_t::iterator it = nb2hopset().begin(); it != nb2hopset().end(); it++) {
     OLSR_nb2hop_tuple* nb2hop_tuple = *it;
     nsaddr_t nb2hop_main_addr = nb2hop_tuple->nb2hop_addr();
     nsaddr_t nb_main_addr = nb2hop_tuple->nb_main_addr();
 
     if (nb2hop_main_addr == ra_addr())
       continue;
     // do we have a symmetric link to the one hop neighbor?
     OLSR_nb_tuple* nb_tuple = state_.find_sym_nb_tuple(nb_main_addr);
     if (nb_tuple == NULL)
       continue;
     // one hop neighbor has willingness different from OLSR_WILL_NEVER?
     nb_tuple = state_.find_nb_tuple(nb_main_addr, OLSR_WILL_NEVER);
     if (nb_tuple != NULL)
       continue;
     // Retrieve the link that connect us to this 2 hop neighbor
     OLSR_nb2hop_tuple* best_link = state_.find_nb2hop_tuple(nb_main_addr, nb2hop_main_addr);
     bool found = false;
     for (std::vector<OLSR_nb2hop_tuple*>::iterator it2 = N[nb_main_addr].begin();
           it2 != N[nb_main_addr].end(); it2++) {
       OLSR_nb2hop_tuple* current_link = *it2;
       if (current_link->nb_main_addr() == nb_main_addr &&
            current_link->nb2hop_addr() == nb2hop_main_addr) {
         found = true;
         break;
       }
     }
     if (!found)
       N[nb_main_addr].push_back(best_link);
     N_index.insert(nb_main_addr);
   }
   // we now have the best link to all of our 2 hop neighbors. Add this information
   // for each 2 hop neighbor to the edge vector...
   for (set<nsaddr_t>::iterator it = N_index.begin(); it != N_index.end(); it++) {
     nsaddr_t nb_main_addr = *it;
 
     for (std::vector<OLSR_nb2hop_tuple*>::iterator it2 = N[nb_main_addr].begin();
           it2 != N[nb_main_addr].end(); it2++) {
       OLSR_nb2hop_tuple* nb2hop_tuple = *it2;
       // Add this edge to the graph we are building. The last hop is our 1 hop
       // neighbor that has the best link to the current two hop neighbor. And
       // nb2hop_addr is not directly connected to this node
       debug ("nb2hop_tuple: %d (local) ==> %d , delay %lf, quality %lf\n", nb_main_addr,
               nb2hop_tuple->nb2hop_addr(), nb2hop_tuple->nb_link_delay(), nb2hop_tuple->etx());
       dijkstra->add_edge (nb2hop_tuple->nb2hop_addr(), nb_main_addr,
                           nb2hop_tuple->nb_link_delay(), nb2hop_tuple->etx(), false);
     }
   }
 
   // here we rely on the fact that in TC messages only the best links are published
   for (topologyset_t::iterator it = topologyset().begin();
         it != topologyset().end(); it++) {
     OLSR_topology_tuple* topology_tuple = *it;
 
     if (topology_tuple->dest_addr() == ra_addr())
       continue;
     // Add this edge to the graph we are building. The last hop is our 1 hop
     // neighbor that has the best link to the current two hop. And dest_addr
     // is not directly connected to this node
     debug ("topology_tuple: %d (local) ==> %d , delay %lf, quality %lf\n", topology_tuple->last_addr(),
              topology_tuple->dest_addr(), topology_tuple->nb_link_delay(), topology_tuple->etx());
     dijkstra->add_edge (topology_tuple->dest_addr(), topology_tuple->last_addr(),
                         topology_tuple->nb_link_delay(), topology_tuple->etx(), false);
   }
 
   // Run the dijkstra algorithm
   dijkstra->run();
 
   // All the entries from the routing table are removed.
   rtable_.clear();
 
   // Now all we have to do is inserting routes according to hop count
   set<nsaddr_t> processed_nodes;
   for (set<nsaddr_t>::iterator it = dijkstra->all_nodes()->begin();
         it != dijkstra->all_nodes()->end(); it++) {
     if (dijkstra->D(*it)->hop_count() == 1) {
       // add route...
       rtable_.add_entry(*it, *it, dijkstra->D(*it)->link().last_node(), 1);
       processed_nodes.insert(*it);
     }
   }
   for (set<nsaddr_t>::iterator it = processed_nodes.begin(); it != processed_nodes.end(); it++)
     dijkstra->all_nodes()->erase(*it);
   processed_nodes.clear();
   for (set<nsaddr_t>::iterator it = dijkstra->all_nodes()->begin();
         it != dijkstra->all_nodes()->end(); it++) {
     if (dijkstra->D(*it)->hop_count() == 2) {
       // add route...
       OLSR_rt_entry* entry = rtable_.lookup(dijkstra->D(*it)->link().last_node());
       assert(entry != NULL);
       rtable_.add_entry(*it, dijkstra->D(*it)->link().last_node(), entry->iface_addr(), 2);
       processed_nodes.insert(*it);
     }
   }
   for (set<nsaddr_t>::iterator it = processed_nodes.begin(); it != processed_nodes.end(); it++)
     dijkstra->all_nodes()->erase(*it);
   processed_nodes.clear();
   for (int i = 3; i <= dijkstra->highest_hop(); i++) {
     for (set<nsaddr_t>::iterator it = dijkstra->all_nodes()->begin();
           it != dijkstra->all_nodes()->end(); it++) {
       if (dijkstra->D(*it)->hop_count() == i) {
         // add route...
         OLSR_rt_entry* entry = rtable_.lookup(dijkstra->D(*it)->link().last_node());
         assert(entry != NULL);
         rtable_.add_entry(*it, dijkstra->D(*it)->link().last_node(), entry->iface_addr(), i);
         processed_nodes.insert(*it);
       }
     }
     for (set<nsaddr_t>::iterator it = processed_nodes.begin(); it != processed_nodes.end(); it++)
       dijkstra->all_nodes()->erase(*it);
     processed_nodes.clear();
   }
 
   // 5. For each entry in the multiple interface association base
   // where there exists a routing entry such that:
   //  R_dest_addr  == I_main_addr  (of the multiple interface association entry)
   // AND there is no routing entry such that:
   //  R_dest_addr  == I_iface_addr
   // then a route entry is created in the routing table
   for (ifaceassocset_t::iterator it = ifaceassocset().begin();
         it != ifaceassocset().end(); it++) {
     OLSR_iface_assoc_tuple* tuple = *it;
     OLSR_rt_entry* entry1 = rtable_.lookup(tuple->main_addr());
     OLSR_rt_entry* entry2 = rtable_.lookup(tuple->iface_addr());
     if (entry1 != NULL && entry2 == NULL) {
       rtable_.add_entry(tuple->iface_addr(),
           entry1->next_addr(), entry1->iface_addr(), entry1->dist());
     }
   }
 
   // destroy the dijkstra class we've created
   delete dijkstra;
 
   rtable_.print_debug(this);
 }
 
 ///
 /// \brief Processes a HELLO message following RFC 3626 specification.
 ///
 /// Link sensing and population of the Neighbor Set, 2-hop Neighbor Set and MPR
 /// Selector Set are performed.
 ///
 /// \param msg the %OLSR message which contains the HELLO message.
 /// \param receiver_iface the address of the interface where the message was received from.
 /// \param sender_iface the address of the interface where the message was sent from.
 ///
 void
 OLSR::process_hello
   (OLSR_msg& msg, nsaddr_t receiver_iface, nsaddr_t sender_iface,
    u_int16_t pkt_seq_num) {
   assert(msg.msg_type() == OLSR_HELLO_MSG);
 
   link_sensing(msg, receiver_iface, sender_iface, pkt_seq_num);
   populate_nbset(msg);
   populate_nb2hopset(msg);
   switch (parameter_.mpr_algorithm()) {
   case OLSR_MPR_R1:
     olsr_r1_mpr_computation();
     break;
 
   case OLSR_MPR_R2:
     olsr_r2_mpr_computation();
     break;
 
   case OLSR_MPR_QOLSR:
     qolsr_mpr_computation();
     break;
 
   case OLSR_MPR_OLSRD:
     olsrd_mpr_computation();
     break;
 
   case OLSR_DEFAULT_MPR:
   default:
     olsr_mpr_computation();
     break;
   }
   populate_mprselset(msg);
 }
 
 ///
 /// \brief Processes a TC message following RFC 3626 specification.
 ///
 /// The Topology Set is updated (if needed) with the information of
 /// the received TC message.
 ///
 /// \param msg the %OLSR message which contains the TC message.
 /// \param sender_iface the address of the interface where the message was sent from.
 ///
 void
 OLSR::process_tc(OLSR_msg& msg, nsaddr_t sender_iface) {
   assert(msg.msg_type() == OLSR_TC_MSG);
   double now = CURRENT_TIME;
   OLSR_tc& tc = msg.tc();
 
   // 1. If the sender interface of this message is not in the symmetric
   // 1-hop neighborhood of this node, the message MUST be discarded.
   OLSR_link_tuple* link_tuple = state_.find_sym_link_tuple(sender_iface, now);
   if (link_tuple == NULL)
     return;
 
   // 2. If there exist some tuple in the topology set where:
   //   T_last_addr == originator address AND
   //   T_seq       >  ANSN,
   // then further processing of this TC message MUST NOT be
   // performed. This might be a message received out of order.
   OLSR_topology_tuple* topology_tuple =
     state_.find_newer_topology_tuple(msg.orig_addr(), tc.ansn());
   if (topology_tuple != NULL)
     return;
 
   // 3. All tuples in the topology set where:
   //  T_last_addr == originator address AND
   //  T_seq       <  ANSN
   // MUST be removed from the topology set.
   state_.erase_older_topology_tuples(msg.orig_addr(), tc.ansn());
 
   // 4. For each of the advertised neighbor main address received in
   // the TC message:
   for (int i = 0; i < tc.count; i++) {
     assert(i >= 0 && i < OLSR_MAX_ADDRS);
     nsaddr_t addr = tc.nb_main_addr(i).iface_address();
     // 4.1. If there exist some tuple in the topology set where:
     //   T_dest_addr == advertised neighbor main address, AND
     //   T_last_addr == originator address,
     // then the holding time of that tuple MUST be set to:
     //   T_time      =  current time + validity time.
     OLSR_topology_tuple* topology_tuple =
       state_.find_topology_tuple(addr, msg.orig_addr());
     if (topology_tuple != NULL)
         topology_tuple->time() = now + OLSR::emf_to_seconds(msg.vtime());
     // 4.2. Otherwise, a new tuple MUST be recorded in the topology
     // set where:
     //  T_dest_addr = advertised neighbor main address,
     //  T_last_addr = originator address,
     //  T_seq       = ANSN,
     //  T_time      = current time + validity time.
     else {
       topology_tuple = new OLSR_topology_tuple;
       topology_tuple->dest_addr() = addr;
       topology_tuple->last_addr() = msg.orig_addr();
       topology_tuple->seq() = tc.ansn();
       topology_tuple->time() = now + OLSR::emf_to_seconds(msg.vtime());
       add_topology_tuple(topology_tuple);
       // Schedules topology tuple deletion
       OLSR_TopologyTupleTimer* topology_timer =
         new OLSR_TopologyTupleTimer(this, topology_tuple);
       topology_timer->resched(DELAY(topology_tuple->time()));
     }
     // Update link quality and link delay information
     topology_tuple->update_link_quality(
       tc.nb_main_addr(i).link_quality(),
       tc.nb_main_addr(i).nb_link_quality());
     topology_tuple->update_link_delay(
       tc.nb_main_addr(i).link_delay(),
       tc.nb_main_addr(i).nb_link_delay());
   }
 }
 
 ///
 /// \brief Processes a MID message following RFC 3626 specification.
 ///
 /// The Interface Association Set is updated (if needed) with the information
 /// of the received MID message.
 ///
 /// \param msg the %OLSR message which contains the MID message.
 /// \param sender_iface the address of the interface where the message was sent from.
 ///
 void
 OLSR::process_mid(OLSR_msg& msg, nsaddr_t sender_iface) {
   assert(msg.msg_type() == OLSR_MID_MSG);
   double now = CURRENT_TIME;
   OLSR_mid& mid = msg.mid();
 
   // 1. If the sender interface of this message is not in the symmetric
   // 1-hop neighborhood of this node, the message MUST be discarded.
   OLSR_link_tuple* link_tuple = state_.find_sym_link_tuple(sender_iface, now);
   if (link_tuple == NULL)
     return;
 
   // 2. For each interface address listed in the MID message
   for (int i = 0; i < mid.count; i++) {
     bool updated = false;
     for (ifaceassocset_t::iterator it = ifaceassocset().begin();
       it != ifaceassocset().end(); it++) {
       OLSR_iface_assoc_tuple* tuple = *it;
 
       // 2.1 If there exist some tuple in the interface association
       //     set where:
       //       I_iface_addr == interface address, AND
       //       I_main_addr  == originator address,
       //     then the holding time of that tuple is set to:
       //       I_time       = current time + validity time.
       if (tuple->iface_addr() == mid.iface_addr(i)
         && tuple->main_addr() == msg.orig_addr()) {
         tuple->time() = now + OLSR::emf_to_seconds(msg.vtime());
         updated = true;
       }
     }
 
     // 2.2 Otherwise, a new tuple is recorded in the interface
     //     association set where:
     //       I_iface_addr = interface address,
     //       I_main_addr  = originator address,
     //       I_time       = current time + validity time.
     if (!updated) {
       OLSR_iface_assoc_tuple* tuple = new OLSR_iface_assoc_tuple;
       tuple->iface_addr() = msg.mid().iface_addr(i);
       tuple->main_addr() = msg.orig_addr();
       tuple->time()  = now + OLSR::emf_to_seconds(msg.vtime());
       add_ifaceassoc_tuple(tuple);
       // Schedules iface association tuple deletion
       OLSR_IfaceAssocTupleTimer* ifaceassoc_timer =
         new OLSR_IfaceAssocTupleTimer(this, tuple);
       ifaceassoc_timer->resched(DELAY(tuple->time()));
     }
   }
 }
 
 ///
 /// \brief OLSR's default forwarding algorithm.
 ///
 /// See RFC 3626 for details.
 ///
 /// \param p the %OLSR packet which has been received.
 /// \param msg the %OLSR message which must be forwarded.
 /// \param dup_tuple NULL if the message has never been considered for forwarding,
 /// or a duplicate tuple in other case.
 /// \param local_iface the address of the interface where the message was received from.
 ///
 void
 OLSR::forward_default(Packet* p, OLSR_msg& msg, OLSR_dup_tuple* dup_tuple, nsaddr_t local_iface) {
   double now = CURRENT_TIME;
   struct hdr_ip* ih = HDR_IP(p);
 
   // If the sender interface address is not in the symmetric
   // 1-hop neighborhood the message must not be forwarded
   OLSR_link_tuple* link_tuple = state_.find_sym_link_tuple(ih->saddr(), now);
   if (link_tuple == NULL)
     return;
 
   // If the message has already been considered for forwarding,
   // it must not be retransmitted again
   if (dup_tuple != NULL && dup_tuple->retransmitted()) {
     debug("%f: Node %d does not forward a message received"
             " from %d because it is duplicated\n",
           CURRENT_TIME, OLSR::node_id(ra_addr()),
           OLSR::node_id(dup_tuple->addr()));
     return;
   }
 
   //   If after those steps, the message is not considered for forwarding,
   //   then the processing of this section stops (i.e., steps 4 to 8 are
   //   ignored), otherwise, if it is still considered for forwarding then
   //   the following algorithm is used:
 
   // If the sender interface address is an interface address
   // of a MPR selector of this node and ttl is greater than 1,
   // the message must be retransmitted
   bool retransmitted = false;
   if (msg.ttl() > 1) {
     OLSR_mprsel_tuple* mprsel_tuple =
       state_.find_mprsel_tuple(get_main_addr(ih->saddr()));
     if (mprsel_tuple != NULL) {
       retransmitted = true;
     }
   }
 
   // Update duplicate tuple...
   if (dup_tuple != NULL) {
     dup_tuple->time() = now + OLSR_DUP_HOLD_TIME;
     dup_tuple->retransmitted() = retransmitted;
     dup_tuple->iface_list().push_back(local_iface);
   }
 
   // ...or create a new one
   else {
     OLSR_dup_tuple* new_dup = new OLSR_dup_tuple;
     new_dup->addr() = msg.orig_addr();
     new_dup->seq_num() = msg.msg_seq_num();
     new_dup->time() = now + OLSR_DUP_HOLD_TIME;
     new_dup->retransmitted() = retransmitted;
     new_dup->iface_list().push_back(local_iface);
     add_dup_tuple(new_dup);
     // Schedules dup tuple deletion
     OLSR_DupTupleTimer* dup_timer =
       new OLSR_DupTupleTimer(this, new_dup);
     dup_timer->resched(DELAY(new_dup->time()));
   }
   // If, and only if, according to step 4, the message must be
   // retransmitted then:
   if (retransmitted) {
     OLSR_msg& new_msg = msg;
     new_msg.ttl()--;
     new_msg.hop_count()++;
     // We have to introduce a random delay to avoid
     // synchronization with neighbors.
     enque_msg(new_msg, JITTER);
   }
 }
 
 ///
 /// \brief Forwards a data packet to the appropiate next hop indicated by the routing table.
 ///
 /// \param p the packet which must be forwarded.
 ///
 void
 OLSR::forward_data(Packet* p) {
   struct hdr_cmn* ch  = HDR_CMN(p);
   struct hdr_ip* ih  = HDR_IP(p);
 
   if (ch->direction() == hdr_cmn::UP &&
     ((u_int32_t)ih->daddr() == IP_BROADCAST || ih->daddr() == ra_addr())) {
     dmux_->recv(p, 0);
     return;
   }
   else {
     ch->direction() = hdr_cmn::DOWN;
     ch->addr_type() = NS_AF_INET;
     if ((u_int32_t)ih->daddr() == IP_BROADCAST)
       ch->next_hop() = IP_BROADCAST;
     else {
       OLSR_rt_entry* entry = rtable_.lookup(ih->daddr());
       if (entry == NULL) {
         debug("%f: Node %d can not forward a packet destined to %d\n",
           CURRENT_TIME,
           OLSR::node_id(ra_addr()),
           OLSR::node_id(ih->daddr()));
         drop(p, DROP_RTR_NO_ROUTE);
         return;
       }
       else {
         entry = rtable_.find_send_entry(entry);
         assert(entry != NULL);
         ch->next_hop() = entry->next_addr();
         if (use_mac()) {
           ch->xmit_failure_  = olsr_mac_failed_callback;
           ch->xmit_failure_data_  = (void*)this;
         }
       }
     }
 
     Scheduler::instance().schedule(target_, p, 0.0);
   }
 }
 
 ///
 /// \brief Enques an %OLSR message which will be sent with a delay of (0, delay].
 ///
 /// This buffering system is used in order to piggyback several %OLSR messages in
 /// a same %OLSR packet.
 ///
 /// \param msg the %OLSR message which must be sent.
 /// \param delay maximum delay the %OLSR message is going to be buffered.
 ///
 void
 OLSR::enque_msg(OLSR_msg& msg, double delay) {
   assert(delay >= 0);
 
   msgs_.push_back(msg);
   OLSR_MsgTimer* timer = new OLSR_MsgTimer(this);
   timer->resched(delay);
 }
 
 ///
 /// \brief Creates as many %OLSR packets as needed in order to send all buffered
 /// %OLSR messages.
 ///
 /// Maximum number of messages which can be contained in an %OLSR packet is
 /// dictated by OLSR_MAX_MSGS constant.
 ///
 void
 OLSR::send_pkt() {
   int num_msgs = msgs_.size();
   if (num_msgs == 0)
     return;
 
   // Calculates the number of needed packets
   int num_pkts = (num_msgs % OLSR_MAX_MSGS == 0) ? num_msgs / OLSR_MAX_MSGS :
     (num_msgs / OLSR_MAX_MSGS + 1);
 
   for (int i = 0; i < num_pkts; i++) {
     /// Link delay extension
     if (parameter_.link_delay()) {
       // We are going to use link delay extension
       // to define routes to be selected
       Packet* p1 = allocpkt();
       struct hdr_cmn* ch1 = HDR_CMN(p1);
       struct hdr_ip* ih1 = HDR_IP(p1);
       OLSR_pkt* op1 = PKT_OLSR(p1);
 
       // Duplicated packet...
       Packet* p2;
       struct hdr_cmn* ch2;
       struct hdr_ip* ih2;
       OLSR_pkt* op2;
 
       op1->pkt_seq_num() = pkt_seq();
       if (i == 0) {
         op1->pkt_len() = OLSR_CAPPROBE_PACKET_SIZE;
         op1->sn() = cap_sn();
 
         // Allocate room for a duplicated packet...
         p2 = allocpkt();
         ch2 = HDR_CMN(p2);
         ih2 = HDR_IP(p2);
         op2 = PKT_OLSR(p2);
 
         op2->pkt_len() = OLSR_CAPPROBE_PACKET_SIZE;
         // duplicated packet sequence no ...
         op2->pkt_seq_num() = op1->pkt_seq_num();
         // but different cap sequence no
         op2->sn() = cap_sn();
       } else {
         op1->pkt_len() = OLSR_PKT_HDR_SIZE;
         op1->sn() = 0;
       }
 
       int j = 0;
       for (std::vector<OLSR_msg>::iterator it = msgs_.begin(); it != msgs_.end(); it++) {
         if (j == OLSR_MAX_MSGS)
           break;
 
         op1->pkt_body_[j++] = *it;
         op1->count = j;
         if (i != 0)
           op1->pkt_len() += (*it).size();
 
         else /* if (i == 0) */ {
           op2->pkt_body_[j++] = *it;
           op2->count = j;
         }
 
         it = msgs_.erase(it);
         it--;
       }
 
       ch1->ptype() = PT_OLSR;
       ch1->direction() = hdr_cmn::DOWN;
       ch1->size() = IP_HDR_LEN + UDP_HDR_LEN + op1->pkt_len();
       ch1->error() = 0;
       ch1->next_hop() = IP_BROADCAST;
       ch1->addr_type() = NS_AF_INET;
 
       if (i == 0) {
         ch2->ptype() = PT_OLSR;
         ch2->direction() = hdr_cmn::DOWN;
         ch2->size() = IP_HDR_LEN + UDP_HDR_LEN + op2->pkt_len();
         ch2->error() = 0;
         ch2->next_hop() = IP_BROADCAST;
         ch2->addr_type() = NS_AF_INET;
       }
 
       if (use_mac()) {
         ch1->xmit_failure_ = olsr_mac_failed_callback;
         ch1->xmit_failure_data_ = (void*)this;
 
         if (i == 0) {
           ch2->xmit_failure_ = olsr_mac_failed_callback;
           ch2->xmit_failure_data_ = (void*)this;
         }
       }
 
       ih1->saddr() = ra_addr();
       ih1->daddr() = IP_BROADCAST;
       ih1->sport() = RT_PORT;
       ih1->dport() = RT_PORT;
       ih1->ttl() = IP_DEF_TTL;
 
       if (i == 0) {
         ih2->saddr() = ra_addr();
         ih2->daddr() = IP_BROADCAST;
         ih2->sport() = RT_PORT;
         ih2->dport() = RT_PORT;
         ih2->ttl() = IP_DEF_TTL;
       }
 
       // Marking packet timestamp
       op1->send_time() = CURRENT_TIME;
       if (i == 0) op2->send_time() = op1->send_time();
       // Sending packet pair
       Scheduler::instance().schedule(target_, p1, 0.0);
       if (i == 0) Scheduler::instance().schedule(target_, p2, 0.0);
     } else {
       Packet* p = allocpkt();
       struct hdr_cmn* ch = HDR_CMN(p);
       struct hdr_ip* ih = HDR_IP(p);
       OLSR_pkt* op = PKT_OLSR(p);
 
       op->pkt_len() = OLSR_PKT_HDR_SIZE;
       op->pkt_seq_num() = pkt_seq();
 
       int j = 0;
       for (std::vector<OLSR_msg>::iterator it = msgs_.begin(); it != msgs_.end(); it++) {
         if (j == OLSR_MAX_MSGS)
           break;
 
         op->pkt_body_[j++] = *it;
         op->count = j;
         op->pkt_len() += (*it).size();
 
         it = msgs_.erase(it);
         it--;
       }
 
       ch->ptype() = PT_OLSR;
       ch->direction() = hdr_cmn::DOWN;
       ch->size() = IP_HDR_LEN + UDP_HDR_LEN + op->pkt_len();
       ch->error() = 0;
       ch->next_hop() = IP_BROADCAST;
       ch->addr_type() = NS_AF_INET;
 
       if (use_mac()) {
         ch->xmit_failure_  = olsr_mac_failed_callback;
         ch->xmit_failure_data_  = (void*)this;
       }
 
       ih->saddr()  = ra_addr();
       ih->daddr()  = IP_BROADCAST;
       ih->sport()  = RT_PORT;
       ih->dport()  = RT_PORT;
       ih->ttl()  = IP_DEF_TTL;
 
       Scheduler::instance().schedule(target_, p, 0.0);
     }
   }
 }
 
 ///
 /// \brief Creates a new %OLSR HELLO message which is buffered to be sent later on.
 ///
 void
 OLSR::send_hello() {
   OLSR_msg msg;
   double now = CURRENT_TIME;
   msg.msg_type() = OLSR_HELLO_MSG;
   msg.vtime() = OLSR::seconds_to_emf(OLSR_NEIGHB_HOLD_TIME);
   msg.orig_addr() = ra_addr();
   msg.ttl() = 1;
   msg.hop_count() = 0;
   msg.msg_seq_num()  = msg_seq();
 
   msg.hello().reserved() = 0;
   msg.hello().htime() = OLSR::seconds_to_emf(hello_ival());
   msg.hello().willingness()  = willingness();
   msg.hello().count = 0;
 
   map<u_int8_t, int> linkcodes_count;
 
   // For each tuple in the Link Set, where L_local_iface_addr is the
   // interface where the HELLO is to be transmitted, and where L_time >=
   // current time (i.e., not expired), L_neighbor_iface_addr is advertised
   // with:
   for (linkset_t::iterator it = linkset().begin(); it != linkset().end(); it++) {
     OLSR_link_tuple* link_tuple = *it;
     if (link_tuple->local_iface_addr() == ra_addr() && link_tuple->time() >= now) {
       u_int8_t link_type, nb_type, link_code;
       // 1 The Link Type set according to the following:
       //   1.1  if L_SYM_time >= current time (not expired)
       //          Link Type = SYM_LINK
       //   1.2  Otherwise, if L_ASYM_time >= current time (not expired)
       //        AND L_SYM_time  <  current time (expired)
       //          Link Type = ASYM_LINK
       //   1.3  Otherwise, if L_ASYM_time < current time (expired) AND
       //        L_SYM_time  < current time (expired)
       //          Link Type = LOST_LINK
       // Establishes link type
       if (use_mac() && link_tuple->lost_time() >= now)
         link_type = OLSR_LOST_LINK;
       else if (link_tuple->sym_time() >= now)
         link_type = OLSR_SYM_LINK;
       else if (link_tuple->asym_time() >= now)
         link_type = OLSR_ASYM_LINK;
       else
         link_type = OLSR_LOST_LINK;
 
       // 2 The Neighbor Type is set according to the following:
       //   2.1  If the main address, corresponding to
       //        L_neighbor_iface_addr, is included in the MPR set:
       //          Neighbor Type = MPR_NEIGH
       //   2.2  Otherwise, if the main address, corresponding to
       //        L_neighbor_iface_addr, is included in the neighbor set:
       //     2.2.1  if N_status == SYM
       //              Neighbor Type = SYM_NEIGH
       //     2.2.2  Otherwise, if N_status == NOT_SYM
       //              Neighbor Type = NOT_NEIGH
 
       // Establishes neighbor type.
       if (state_.find_mpr_addr(get_main_addr(link_tuple->nb_iface_addr())))
         nb_type = OLSR_MPR_NEIGH;
       else {
         bool ok = false;
         for (nbset_t::iterator nb_it = nbset().begin();
              nb_it != nbset().end(); nb_it++) {
           OLSR_nb_tuple* nb_tuple = *nb_it;
           if (nb_tuple->nb_main_addr() == link_tuple->nb_iface_addr()) {
             if (nb_tuple->status() == OLSR_STATUS_SYM)
               nb_type = OLSR_SYM_NEIGH;
             else if (nb_tuple->status() == OLSR_STATUS_NOT_SYM)
               nb_type = OLSR_NOT_NEIGH;
             else {
               fprintf(stderr, "There is a neighbor tuple with an unknown status!\n");
               exit(1);
             }
             ok = true;
             break;
           }
         }
         if (!ok) {
           fprintf(stderr, "Link tuple has no corresponding Neighbor tuple\n");
           exit(1);
         }
       }
 
       int count = msg.hello().count;
       link_code = (link_type & 0x03) | ((nb_type << 2) & 0x0f);
       map<u_int8_t, int>::iterator pos = linkcodes_count.find(link_code);
       if (pos == linkcodes_count.end()) {
         linkcodes_count[link_code] = count;
         assert(count >= 0 && count < OLSR_MAX_HELLOS);
         msg.hello().hello_msg(count).count = 0;
         msg.hello().hello_msg(count).link_code() = link_code;
         msg.hello().hello_msg(count).reserved() = 0;
         msg.hello().count++;
       }
       else
         count = (*pos).second;
 
       int i = msg.hello().hello_msg(count).count;
       assert(count >= 0 && count < OLSR_MAX_HELLOS);
       assert(i >= 0 && i < OLSR_MAX_ADDRS);
 
       msg.hello().hello_msg(count).nb_iface_addr(i).iface_address() =
         link_tuple->nb_iface_addr();
       // publish link quality and link delay information we have
       // found out and the one we have received from our neighbors
       msg.hello().hello_msg(count).nb_iface_addr(i).link_quality() =
         link_tuple->link_quality();
       msg.hello().hello_msg(count).nb_iface_addr(i).nb_link_quality() =
         link_tuple->nb_link_quality();
       msg.hello().hello_msg(count).nb_iface_addr(i).link_delay() =
         link_tuple->link_delay();
       msg.hello().hello_msg(count).nb_iface_addr(i).nb_link_delay() =
         link_tuple->nb_link_delay(); 
 
       msg.hello().hello_msg(count).count++;
       msg.hello().hello_msg(count).link_msg_size() =
         msg.hello().hello_msg(count).size();
     }
   }
 
   msg.msg_size() = msg.size();
 
   enque_msg(msg, JITTER);
 }
 
 ///
 /// \brief Creates a new %OLSR TC message which is buffered to be sent later on.
 ///
 void
 OLSR::send_tc() {
   OLSR_msg msg;
   msg.msg_type() = OLSR_TC_MSG;
   msg.vtime() = OLSR::seconds_to_emf(OLSR_TOP_HOLD_TIME);
   msg.orig_addr() = ra_addr();
   if (parameter_.fish_eye()) {
     msg.ttl() = tc_msg_ttl_ [tc_msg_ttl_index_];
     tc_msg_ttl_index_ = (tc_msg_ttl_index_ + 1) % (MAX_TC_MSG_TTL);
   } else {
     msg.ttl() = 255;
   }
   msg.hop_count() = 0;
   msg.msg_seq_num()  = msg_seq();
 
   msg.tc().ansn() = ansn_;
   msg.tc().reserved()  = 0;
   msg.tc().count = 0;
 
   // we have to check which mpr selection algorithm is being used
   // prior to adding neighbors to the TC message being generated
   switch (parameter_.mpr_algorithm()) {
   case OLSR_MPR_OLSRD:
     // Report all 1 hop neighbors we have
     for (nbset_t::iterator it = nbset().begin(); it != nbset().end(); it++) {
       OLSR_nb_tuple* nb_tuple = *it;
       int count = msg.tc().count;
       OLSR_link_tuple *link_tuple;
 
       if (nb_tuple->status() == OLSR_STATUS_SYM) {
         assert(count >= 0 && count < OLSR_MAX_ADDRS);
         link_tuple = state_.find_best_sym_link_tuple (nb_tuple->nb_main_addr(), CURRENT_TIME);
         if (link_tuple != NULL) {
           msg.tc().nb_main_addr(count).iface_address() = nb_tuple->nb_main_addr();
 
           // Report link quality and link link delay of the best link
           // that we have to this node.
           msg.tc().nb_main_addr(count).link_quality() = link_tuple->link_quality();
           msg.tc().nb_main_addr(count).nb_link_quality() =
             link_tuple->nb_link_quality();
           msg.tc().nb_main_addr(count).link_delay() = link_tuple->link_delay();
           msg.tc().nb_main_addr(count).nb_link_delay() =
             link_tuple->nb_link_delay();
 
           msg.tc().count++;
         }
       }
     }
     break;
 
   default:
     switch (parameter_.tc_redundancy()) {
     case OLSR_TC_REDUNDANCY_MPR_SEL_SET:
       // publish only nodes in mpr sel set (RFC 3626)
       for (mprselset_t::iterator it = mprselset().begin(); it != mprselset().end(); it++) {
         OLSR_mprsel_tuple* mprsel_tuple = *it;
         int count = msg.tc().count;
         OLSR_link_tuple *link_tuple;
 
         assert(count >= 0 && count < OLSR_MAX_ADDRS);
         link_tuple = state_.find_best_sym_link_tuple (mprsel_tuple->main_addr(), CURRENT_TIME);
         if (link_tuple != NULL) {
           msg.tc().nb_main_addr(count).iface_address() = mprsel_tuple->main_addr();
 
           // Report link quality and link link delay of the best link
           // that we have to this node.
           msg.tc().nb_main_addr(count).link_quality() = link_tuple->link_quality();
           msg.tc().nb_main_addr(count).nb_link_quality() =
             link_tuple->nb_link_quality();
           msg.tc().nb_main_addr(count).link_delay() = link_tuple->link_delay();
           msg.tc().nb_main_addr(count).nb_link_delay() =
             link_tuple->nb_link_delay();
 
           msg.tc().count++;
         }
       }

       break;

     case OLSR_TC_REDUNDANCY_MPR_SEL_SET_PLUS_MPR_SET:
       // publish nodes in mpr sel set plus nodes in mpr set (RFC 3626)
       for (mprselset_t::iterator it = mprselset().begin(); it != mprselset().end(); it++) {
         OLSR_mprsel_tuple* mprsel_tuple = *it;
         int count = msg.tc().count;
         OLSR_link_tuple *link_tuple;
 
         assert(count >= 0 && count < OLSR_MAX_ADDRS);
         link_tuple = state_.find_best_sym_link_tuple (mprsel_tuple->main_addr(), CURRENT_TIME);
         if (link_tuple != NULL) {
           msg.tc().nb_main_addr(count).iface_address() = mprsel_tuple->main_addr();
 
           // Report link quality and link link delay of the best link
           // that we have to this node.
           msg.tc().nb_main_addr(count).link_quality() = link_tuple->link_quality();
           msg.tc().nb_main_addr(count).nb_link_quality() =
             link_tuple->nb_link_quality();
           msg.tc().nb_main_addr(count).link_delay() = link_tuple->link_delay();
           msg.tc().nb_main_addr(count).nb_link_delay() =
             link_tuple->nb_link_delay();
 
           msg.tc().count++;
         }
       }

       for (mprset_t::iterator it = mprset().begin(); it != mprset().end(); it++) {
         nsaddr_t mpr_addr = *it;
         int count = msg.tc().count;
         OLSR_link_tuple *link_tuple;
 
         assert(count >= 0 && count < OLSR_MAX_ADDRS);
         link_tuple = state_.find_best_sym_link_tuple (mpr_addr, CURRENT_TIME);
         if (link_tuple != NULL) {
           msg.tc().nb_main_addr(count).iface_address() = mpr_addr;
 
           // Report link quality and link link delay of the best link
           // that we have to this node.
           msg.tc().nb_main_addr(count).link_quality() = link_tuple->link_quality();
           msg.tc().nb_main_addr(count).nb_link_quality() =
             link_tuple->nb_link_quality();
           msg.tc().nb_main_addr(count).link_delay() = link_tuple->link_delay();
           msg.tc().nb_main_addr(count).nb_link_delay() =
             link_tuple->nb_link_delay();
 
           msg.tc().count++;
         }
       }

       break;

     case OLSR_TC_REDUNDANCY_FULL:
       // publish full neighbor link set (RFC 3626)
       for (nbset_t::iterator it = nbset().begin(); it != nbset().end(); it++) {
         OLSR_nb_tuple* nb_tuple = *it;
         int count = msg.tc().count;
         OLSR_link_tuple *link_tuple;
 
         if (nb_tuple->status() == OLSR_STATUS_SYM) {
           assert(count >= 0 && count < OLSR_MAX_ADDRS);
           link_tuple = state_.find_best_sym_link_tuple (nb_tuple->nb_main_addr(), CURRENT_TIME);
           if (link_tuple != NULL) {
             msg.tc().nb_main_addr(count).iface_address() = nb_tuple->nb_main_addr();
 
             // Report link quality and link link delay of the best link
             // that we have to this node.
             msg.tc().nb_main_addr(count).link_quality() = link_tuple->link_quality();
             msg.tc().nb_main_addr(count).nb_link_quality() =
               link_tuple->nb_link_quality();
             msg.tc().nb_main_addr(count).link_delay() = link_tuple->link_delay();
             msg.tc().nb_main_addr(count).nb_link_delay() =
               link_tuple->nb_link_delay();
 
             msg.tc().count++;
           }
         }
       }

       break;

     case OLSR_TC_REDUNDANCY_MPR_SET:
       // non-OLSR standard: publish mpr set only
       for (mprset_t::iterator it = mprset().begin(); it != mprset().end(); it++) {
         nsaddr_t mpr_addr = *it;
         int count = msg.tc().count;
         OLSR_link_tuple *link_tuple;
 
         assert(count >= 0 && count < OLSR_MAX_ADDRS);
         link_tuple = state_.find_best_sym_link_tuple (mpr_addr, CURRENT_TIME);
         if (link_tuple != NULL) {
           msg.tc().nb_main_addr(count).iface_address() = mpr_addr;
 
           // Report link quality and link link delay of the best link
           // that we have to this node.
           msg.tc().nb_main_addr(count).link_quality() = link_tuple->link_quality();
           msg.tc().nb_main_addr(count).nb_link_quality() =
             link_tuple->nb_link_quality();
           msg.tc().nb_main_addr(count).link_delay() = link_tuple->link_delay();
           msg.tc().nb_main_addr(count).nb_link_delay() =
             link_tuple->nb_link_delay();
 
           msg.tc().count++;
         }
       }

       break;
     }

     break;
   }
 
   msg.msg_size() = msg.size();
   enque_msg(msg, JITTER);
 }
 
 ///
 /// \brief Creates a new %OLSR MID message which is buffered to be sent later on.
 /// \warning This message is never invoked because there is no support for multiple interfaces.
 ///
 void
 OLSR::send_mid() {
   OLSR_msg msg;
   msg.msg_type() = OLSR_MID_MSG;
   msg.vtime() = OLSR::seconds_to_emf(OLSR_MID_HOLD_TIME);
   msg.orig_addr() = ra_addr();
   msg.ttl() = 255;
   msg.hop_count() = 0;
   msg.msg_seq_num()  = msg_seq();
 
   msg.mid().count = 0;
   //foreach iface in this_node do
   //  msg.mid().iface_addr(i) = iface
   //  msg.mid().count++
   //done
 
   msg.msg_size() = msg.size();
   enque_msg(msg, JITTER);
 }
 
 ///
 /// \brief  Updates Link Set according to a new received HELLO message (following RFC 3626
 ///    specification). Neighbor Set is also updated if needed.
 ///
 /// \param msg the OLSR message which contains the HELLO message.
 /// \param receiver_iface the address of the interface where the message was received from.
 /// \param sender_iface the address of the interface where the message was sent from.
 ///
 void
 OLSR::link_sensing
   (OLSR_msg& msg, nsaddr_t receiver_iface, nsaddr_t sender_iface,
    u_int16_t pkt_seq_num) {
   OLSR_hello& hello  = msg.hello();
   double now = CURRENT_TIME;
   bool updated = false;
   bool created = false;
 
   // 1 Upon receiving a HELLO message, if there exists no link tuple
   //   with L_neighbor_iface_addr == Source Address a new tuple is created with
   //           L_neighbor_iface_addr = Source Address
   //           L_local_iface_addr    = Address of the interface
   //                                   which received the
   //                                   HELLO message
   //           L_SYM_time            = current time - 1 (expired)
   //           L_time                = current time + validity time
   OLSR_link_tuple* link_tuple = state_.find_link_tuple(sender_iface);
   if (link_tuple == NULL) {
     // We have to create a new tuple
     link_tuple = new OLSR_link_tuple;
     link_tuple->nb_iface_addr()  = sender_iface;
     link_tuple->local_iface_addr()  = receiver_iface;
     link_tuple->sym_time() = now - 1;
     link_tuple->lost_time() = 0.0;
     link_tuple->time() = now + OLSR::emf_to_seconds(msg.vtime());
     // Init link quality information struct for this link tuple
     link_tuple->link_quality_init (pkt_seq_num, DEFAULT_LOSS_WINDOW_SIZE);
     /// Link delay extension
     link_tuple->link_delay_init ();
     // This call will be also in charge of creating a new tuple in
     // the neighbor set
     add_link_tuple(link_tuple, hello.willingness());
     created = true;
   }
   else
     updated = true;
 
   // Account link quality information for this link
   link_tuple->receive (pkt_seq_num, OLSR::emf_to_seconds(hello.htime()));
 
   // 2    The tuple (existing or new) with:
   //           L_neighbor_iface_addr == Source Address
   //      is then modified as follows:
   //      2.1  L_ASYM_time = current time + validity time;
 
   link_tuple->asym_time() = now + OLSR::emf_to_seconds(msg.vtime());
   assert(hello.count >= 0 && hello.count <= OLSR_MAX_HELLOS);
   for (int i = 0; i < hello.count; i++) {
     OLSR_hello_msg& hello_msg = hello.hello_msg(i);
     int lt = hello_msg.link_code() & 0x03;
     int nt = hello_msg.link_code() >> 2;
 
     // We must not process invalid advertised links
     if ((lt == OLSR_SYM_LINK && nt == OLSR_NOT_NEIGH) ||
       (nt != OLSR_SYM_NEIGH && nt != OLSR_MPR_NEIGH
       && nt != OLSR_NOT_NEIGH))
       continue;
 
     assert(hello_msg.count >= 0 && hello_msg.count <= OLSR_MAX_ADDRS);
     for (int j = 0; j < hello_msg.count; j++) {
 
       //      2.2  if the node finds the address of the interface which
       //           received the HELLO message among the addresses listed in
       //           the link message then the tuple is modified as follows:
       if (hello_msg.nb_iface_addr(j).iface_address() == receiver_iface) {
         //           2.2.1 if Link Type is equal to LOST_LINK then
         //                     L_SYM_time = current time - 1 (i.e., expired)
         if (lt == OLSR_LOST_LINK) {
           link_tuple->sym_time() = now - 1;
           updated = true;
         }
         //           2.2.2 else if Link Type is equal to SYM_LINK or ASYM_LINK
         //                then
         //                     L_SYM_time = current time + validity time,
         //                     L_time     = L_SYM_time + NEIGHB_HOLD_TIME
         else if (lt == OLSR_SYM_LINK || lt == OLSR_ASYM_LINK) {
           link_tuple->sym_time() =
             now + OLSR::emf_to_seconds(msg.vtime());
           link_tuple->time() =
             link_tuple->sym_time() + OLSR_NEIGHB_HOLD_TIME;
           link_tuple->lost_time() = 0.0;
           updated = true;
         }
 
         // Update our neighbor's idea of link quality and link delay
         link_tuple->update_link_quality (hello_msg.nb_iface_addr(j).link_quality());
         link_tuple->update_link_delay (hello_msg.nb_iface_addr(j).link_delay());
 
         break;
       }
     }

   }
 
   //      2.3  L_time = max(L_time, L_ASYM_time)
   link_tuple->time() = MAX(link_tuple->time(), link_tuple->asym_time());
 
   if (updated)
     updated_link_tuple(link_tuple);
   // Schedules link tuple deletion
   if (created && link_tuple != NULL) {
     OLSR_LinkTupleTimer* link_timer =
       new OLSR_LinkTupleTimer(this, link_tuple);
     link_timer->resched(DELAY(MIN(link_tuple->time(), link_tuple->sym_time())));
   }
 }
 
 ///
 /// \brief  Updates the Neighbor Set according to the information contained in a new received
 ///    HELLO message (following RFC 3626).
 ///
 /// \param msg the %OLSR message which contains the HELLO message.
 ///
 void
 OLSR::populate_nbset(OLSR_msg& msg) {
   OLSR_hello& hello = msg.hello();
 
   OLSR_nb_tuple* nb_tuple = state_.find_nb_tuple(msg.orig_addr());
   if (nb_tuple != NULL)
     nb_tuple->willingness() = hello.willingness();
 }
 
 ///
 /// \brief  Updates the 2-hop Neighbor Set according to the information contained in a new
 ///    received HELLO message (following RFC 3626).
 ///
 /// \param msg the %OLSR message which contains the HELLO message.
 ///
 void
 OLSR::populate_nb2hopset(OLSR_msg& msg) {
   OLSR_hello& hello  = msg.hello();
   double now = CURRENT_TIME;
 
   // Upon receiving a HELLO message, the "validity time" MUST be computed
   // from the Vtime field of the message header (see section 3.3.2).
   double validity_time = now + OLSR::emf_to_seconds(msg.vtime());
 
   //  If the Originator Address is the main address of a
   //  L_neighbor_iface_addr from a link tuple included in the Link Set with
   //         L_SYM_time >= current time (not expired)
   //  then the 2-hop Neighbor Set SHOULD be updated as follows:
   for (linkset_t::iterator it_lt = linkset().begin(); it_lt != linkset().end(); it_lt++) {
     OLSR_link_tuple* link_tuple = *it_lt;
 
     if (get_main_addr(link_tuple->nb_iface_addr()) == msg.orig_addr() &&
         link_tuple->sym_time() >= now) {
       assert(hello.count >= 0 && hello.count <= OLSR_MAX_HELLOS);
 
       for (int i = 0; i < hello.count; i++) {
         OLSR_hello_msg& hello_msg = hello.hello_msg(i);
         int nt = hello_msg.link_code() >> 2;
         assert(hello_msg.count >= 0 && hello_msg.count <= OLSR_MAX_ADDRS);
 
         // 1    for each address (henceforth: 2-hop neighbor address), listed
         //      in the HELLO message with Neighbor Type equal to SYM_NEIGH or
         //      MPR_NEIGH:
         if (nt == OLSR_SYM_NEIGH || nt == OLSR_MPR_NEIGH) {
 
           for (int j = 0; j < hello_msg.count; j++) {
             // Weverton Cordeiro: was not verifying 2hop main addr
             nsaddr_t nb2hop_addr = get_main_addr(hello_msg.nb_iface_addr(j).iface_address());
 
             // 1.1  if the main address of the 2-hop neighbor address = main
             // address of the receiving node: silently discard the 2-hop neighbor address.
             if (nb2hop_addr == ra_addr())
               continue;
             // 1.2  Otherwise, a 2-hop tuple is created with:
             // N_neighbor_main_addr =  Originator Address;
             // N_2hop_addr          =  main address of the 2-hop neighbor;
             // N_time               =  current time + validity time.
             OLSR_nb2hop_tuple* nb2hop_tuple =
               state_.find_nb2hop_tuple(msg.orig_addr(), nb2hop_addr);
             if (nb2hop_tuple == NULL) {
               nb2hop_tuple = new OLSR_nb2hop_tuple;
               nb2hop_tuple->nb_main_addr() = msg.orig_addr();
               nb2hop_tuple->nb2hop_addr() = nb2hop_addr;
 
               // Init link quality and link delay information
               nb2hop_tuple->update_link_quality(0.0, 0.0);
               nb2hop_tuple->update_link_delay(1.0, 1.0);
 
               add_nb2hop_tuple(nb2hop_tuple);
               nb2hop_tuple->time() = validity_time;
               // Schedules nb2hop tuple deletion
               OLSR_Nb2hopTupleTimer* nb2hop_timer =
                 new OLSR_Nb2hopTupleTimer(this, nb2hop_tuple);
               nb2hop_timer->resched(DELAY(nb2hop_tuple->time()));
             }
             else
               // This tuple may replace an older similar tuple with same
               // N_neighbor_main_addr and N_2hop_addr values.
               nb2hop_tuple->time() = validity_time;
 
             // Update Link Quality information. Note: we only want information about the best link
             switch (parameter_.link_quality()) {
             case OLSR_BEHAVIOR_ETX:
               if (hello_msg.nb_iface_addr(j).etx() < nb2hop_tuple->etx()) {
                 nb2hop_tuple->update_link_quality(
                   hello_msg.nb_iface_addr(j).link_quality(),
                   hello_msg.nb_iface_addr(j).nb_link_quality());
               }
               break;
 
             case OLSR_BEHAVIOR_ML:
               if (hello_msg.nb_iface_addr(j).etx() > nb2hop_tuple->etx()) {
                 nb2hop_tuple->update_link_quality(
                   hello_msg.nb_iface_addr(j).link_quality(),
                   hello_msg.nb_iface_addr(j).nb_link_quality());
               }
               break;
 
             case OLSR_BEHAVIOR_NONE:
             default:
               //
               break;
             }
             // Update link delay information
             if (hello_msg.nb_iface_addr(j).nb_link_delay() < nb2hop_tuple->nb_link_delay()) {
               nb2hop_tuple->update_link_delay(
                 hello_msg.nb_iface_addr(j).link_delay(),
                 hello_msg.nb_iface_addr(j).nb_link_delay());
             }
           }
         }
         // 2 For each 2-hop node listed in the HELLO message with Neighbor
         //   Type equal to NOT_NEIGH, all 2-hop tuples where:
         else if (nt == OLSR_NOT_NEIGH) {
 
           for (int j = 0; j < hello_msg.count; j++) {
             nsaddr_t nb2hop_addr = get_main_addr(hello_msg.nb_iface_addr(j).iface_address());
 
             state_.erase_nb2hop_tuples(msg.orig_addr(), nb2hop_addr);
           }
         }
       }
       // this hello message was already processed, and processing it for another symmetric
       // link we find in our link set will not make any new changes
       break;
     }
   }
 }
 
 ///
 /// \brief  Updates the MPR Selector Set according to the information contained in a new
 ///    received HELLO message (following RFC 3626).
 ///
 /// \param msg the %OLSR message which contains the HELLO message.
 ///
 void
 OLSR::populate_mprselset(OLSR_msg& msg) {
   double now = CURRENT_TIME;
   OLSR_hello& hello  = msg.hello();
 
   assert(hello.count >= 0 && hello.count <= OLSR_MAX_HELLOS);
   for (int i = 0; i < hello.count; i++) {
     OLSR_hello_msg& hello_msg = hello.hello_msg(i);
     int nt = hello_msg.link_code() >> 2;
     if (nt == OLSR_MPR_NEIGH) {
       assert(hello_msg.count >= 0 && hello_msg.count <= OLSR_MAX_ADDRS);
       for (int j = 0; j < hello_msg.count; j++) {
         if (hello_msg.nb_iface_addr(j).iface_address() == ra_addr()) {
           // We must create a new entry into the mpr selector set
           OLSR_mprsel_tuple* mprsel_tuple =
             state_.find_mprsel_tuple(msg.orig_addr());
           if (mprsel_tuple == NULL) {
             mprsel_tuple = new OLSR_mprsel_tuple;
             mprsel_tuple->main_addr() = msg.orig_addr();
             mprsel_tuple->time() = now + OLSR::emf_to_seconds(msg.vtime());
             add_mprsel_tuple(mprsel_tuple);
             // Schedules mpr selector tuple deletion
             OLSR_MprSelTupleTimer* mprsel_timer =
               new OLSR_MprSelTupleTimer(this, mprsel_tuple);
             mprsel_timer->resched(DELAY(mprsel_tuple->time()));
           }
           else
             mprsel_tuple->time() = now + OLSR::emf_to_seconds(msg.vtime());
         }
       }
     }
   }
 }
 
 ///
 /// \brief  Drops a given packet because it couldn't be delivered to the corresponding
 ///    destination by the MAC layer. This may cause a neighbor loss, and appropiate
 ///    actions are then taken.
 ///
 /// \param p the packet which couldn't be delivered by the MAC layer.
 ///
 void
 OLSR::mac_failed(Packet* p) {
   double now = CURRENT_TIME;
   struct hdr_ip* ih  = HDR_IP(p);
   struct hdr_cmn* ch  = HDR_CMN(p);
 
   debug("%f: Node %d MAC Layer detects a breakage on link to %d\n",
          now, OLSR::node_id(ra_addr()), OLSR::node_id(ch->next_hop()));
 
   if ((u_int32_t)ih->daddr() == IP_BROADCAST) {
     drop(p, DROP_RTR_MAC_CALLBACK);
     return;
   }
 
   OLSR_link_tuple* link_tuple = state_.find_link_tuple(ch->next_hop());
   if (link_tuple != NULL) {
     link_tuple->lost_time()  = now + OLSR_NEIGHB_HOLD_TIME;
     link_tuple->time()  = now + OLSR_NEIGHB_HOLD_TIME;
     link_tuple->mac_failed ();
     nb_loss(link_tuple);
   }
   drop(p, DROP_RTR_MAC_CALLBACK);
 }
 
 ///
 /// \brief Schedule the timer used for checking link timeouts.
 ///
 void
 OLSR::set_link_quality_timer() {
   link_quality_timer_.resched((double)(hello_ival()));
 }
 
 ///
 /// \brief Schedule the timer used for sending HELLO messages.
 ///
 void
 OLSR::set_hello_timer() {
   hello_timer_.resched((double)(hello_ival() - JITTER));
 }
 
 ///
 /// \brief Schedule the timer used for sending TC messages.
 ///
 void
 OLSR::set_tc_timer() {
   tc_timer_.resched((double)(tc_ival() - JITTER));
 }
 
 ///
 /// \brief Schedule the timer used for sending MID messages.
 ///
 void
 OLSR::set_mid_timer() {
   mid_timer_.resched((double)(mid_ival() - JITTER));
 }
 
 ///
 /// \brief Performs all actions needed when a neighbor loss occurs.
 ///
 /// Neighbor Set, 2-hop Neighbor Set, MPR Set and MPR Selector Set are updated.
 ///
 /// \param tuple link tuple with the information of the link to the neighbor which has been lost.
 ///
 void
 OLSR::nb_loss(OLSR_link_tuple* tuple) {
   debug("%f: Node %d detects neighbor %d loss\n", CURRENT_TIME,
     OLSR::node_id(ra_addr()), OLSR::node_id(tuple->nb_iface_addr()));
 
   updated_link_tuple(tuple);
   state_.erase_nb2hop_tuples(get_main_addr(tuple->nb_iface_addr()));
   state_.erase_mprsel_tuples(get_main_addr(tuple->nb_iface_addr()));
 
   switch (parameter_.mpr_algorithm()) {
   case OLSR_MPR_R1:
     olsr_r1_mpr_computation();
     break;
 
   case OLSR_MPR_R2:
     olsr_r2_mpr_computation();
     break;
 
   case OLSR_MPR_QOLSR:
     qolsr_mpr_computation();
     break;
 
   case OLSR_MPR_OLSRD:
     olsrd_mpr_computation();
     break;
 
   case OLSR_DEFAULT_MPR:
   default:
     olsr_mpr_computation();
     break;
   }
   switch (parameter_.routing_algorithm()) {
   case OLSR_DIJKSTRA_ALGORITHM:
     rtable_dijkstra_computation();
     break;
 
   default:
   case OLSR_DEFAULT_ALGORITHM:
     rtable_default_computation();
     break;
   }
 }
 
 ///
 /// \brief Adds a duplicate tuple to the Duplicate Set.
 ///
 /// \param tuple the duplicate tuple to be added.
 ///
 void
 OLSR::add_dup_tuple(OLSR_dup_tuple* tuple) {
   /*debug("%f: Node %d adds dup tuple: addr = %d seq_num = %d\n",
     CURRENT_TIME,
     OLSR::node_id(ra_addr()),
     OLSR::node_id(tuple->addr()),
     tuple->seq_num());*/
 
   state_.insert_dup_tuple(tuple);
 }
 
 ///
 /// \brief Removes a duplicate tuple from the Duplicate Set.
 ///
 /// \param tuple the duplicate tuple to be removed.
 ///
 void
 OLSR::rm_dup_tuple(OLSR_dup_tuple* tuple) {
   /*debug("%f: Node %d removes dup tuple: addr = %d seq_num = %d\n",
     CURRENT_TIME,
     OLSR::node_id(ra_addr()),
     OLSR::node_id(tuple->addr()),
     tuple->seq_num());*/
 
   state_.erase_dup_tuple(tuple);
 }
 
 ///
 /// \brief Adds a link tuple to the Link Set (and an associated neighbor tuple to the Neighbor Set).
 ///
 /// \param tuple the link tuple to be added.
 /// \param willingness willingness of the node which is going to be inserted in the Neighbor Set.
 ///
 void
 OLSR::add_link_tuple(OLSR_link_tuple* tuple, u_int8_t  willingness) {
   double now = CURRENT_TIME;
 
   debug("%f: Node %d adds link tuple: nb_addr = %d\n", now,
     OLSR::node_id(ra_addr()), OLSR::node_id(tuple->nb_iface_addr()));
 
   state_.insert_link_tuple(tuple);
   // Creates associated neighbor tuple
   OLSR_nb_tuple* nb_tuple = new OLSR_nb_tuple;
   nb_tuple->nb_main_addr() = get_main_addr(tuple->nb_iface_addr());
   nb_tuple->willingness() = willingness;
   if (tuple->sym_time() >= now)
     nb_tuple->status() = OLSR_STATUS_SYM;
   else
     nb_tuple->status() = OLSR_STATUS_NOT_SYM;
   add_nb_tuple(nb_tuple);
 }
 
 ///
 /// \brief Removes a link tuple from the Link Set.
 ///
 /// \param tuple the link tuple to be removed.
 ///
 void
 OLSR::rm_link_tuple(OLSR_link_tuple* tuple) {
   nsaddr_t nb_addr  = get_main_addr(tuple->nb_iface_addr());
   double now = CURRENT_TIME;
 
   debug("%f: Node %d removes link tuple: nb_addr = %d\n", now,
     OLSR::node_id(ra_addr()), OLSR::node_id(tuple->nb_iface_addr()));
   // Prints this here cause we are not actually calling rm_nb_tuple() (efficiency stuff)
   debug("%f: Node %d removes neighbor tuple: nb_addr = %d\n", now,
     OLSR::node_id(ra_addr()), OLSR::node_id(nb_addr));
 
   state_.erase_link_tuple(tuple);
 
   OLSR_nb_tuple* nb_tuple = state_.find_nb_tuple(nb_addr);
   state_.erase_nb_tuple(nb_tuple);
   delete nb_tuple;
 }
 
 ///
 /// \brief  This function is invoked when a link tuple is updated. Its aim is to
 ///    also update the corresponding neighbor tuple if it is needed.
 ///
 /// \param tuple the link tuple which has been updated.
 ///
 void
 OLSR::updated_link_tuple(OLSR_link_tuple* tuple) {
   double now = CURRENT_TIME;
 
   // Each time a link tuple changes, the associated neighbor tuple must be recomputed
   OLSR_nb_tuple* nb_tuple =
     state_.find_nb_tuple(get_main_addr(tuple->nb_iface_addr()));
   if (nb_tuple != NULL) {
     if (use_mac() && tuple->lost_time() >= now)
       nb_tuple->status() = OLSR_STATUS_NOT_SYM;
     else if (tuple->sym_time() >= now)
       nb_tuple->status() = OLSR_STATUS_SYM;
     else
       nb_tuple->status() = OLSR_STATUS_NOT_SYM;
   }
 
   debug("%f: Node %d has updated link tuple: nb_addr = %d status = %s\n", now,
     OLSR::node_id(ra_addr()), OLSR::node_id(tuple->nb_iface_addr()),
     ((nb_tuple->status() == OLSR_STATUS_SYM) ? "sym" : "not_sym"));
 }
 
 ///
 /// \brief Adds a neighbor tuple to the Neighbor Set.
 ///
 /// \param tuple the neighbor tuple to be added.
 ///
 void
 OLSR::add_nb_tuple(OLSR_nb_tuple* tuple) {
   debug("%f: Node %d adds neighbor tuple: nb_addr = %d status = %s\n",
     CURRENT_TIME, OLSR::node_id(ra_addr()), OLSR::node_id(tuple->nb_main_addr()),
     ((tuple->status() == OLSR_STATUS_SYM) ? "sym" : "not_sym"));
 
   state_.insert_nb_tuple(tuple);
 }
 
 ///
 /// \brief Removes a neighbor tuple from the Neighbor Set.
 ///
 /// \param tuple the neighbor tuple to be removed.
 ///
 void
 OLSR::rm_nb_tuple(OLSR_nb_tuple* tuple) {
   debug("%f: Node %d removes neighbor tuple: nb_addr = %d status = %s\n",
     CURRENT_TIME, OLSR::node_id(ra_addr()), OLSR::node_id(tuple->nb_main_addr()),
     ((tuple->status() == OLSR_STATUS_SYM) ? "sym" : "not_sym"));
 
   state_.erase_nb_tuple(tuple);
 }
 
 ///
 /// \brief Adds a 2-hop neighbor tuple to the 2-hop Neighbor Set.
 ///
 /// \param tuple the 2-hop neighbor tuple to be added.
 ///
 void
 OLSR::add_nb2hop_tuple(OLSR_nb2hop_tuple* tuple) {
   debug("%f: Node %d adds 2-hop neighbor tuple: nb_addr = %d nb2hop_addr = %d\n",
     CURRENT_TIME, OLSR::node_id(ra_addr()), OLSR::node_id(tuple->nb_main_addr()),
     OLSR::node_id(tuple->nb2hop_addr()));
   state_.insert_nb2hop_tuple(tuple);
 }
 
 ///
 /// \brief Removes a 2-hop neighbor tuple from the 2-hop Neighbor Set.
 ///
 /// \param tuple the 2-hop neighbor tuple to be removed.
 ///
 void
 OLSR::rm_nb2hop_tuple(OLSR_nb2hop_tuple* tuple) {
   debug("%f: Node %d removes 2-hop neighbor tuple: nb_addr = %d nb2hop_addr = %d\n",
     CURRENT_TIME, OLSR::node_id(ra_addr()), OLSR::node_id(tuple->nb_main_addr()),
     OLSR::node_id(tuple->nb2hop_addr()));
 
   state_.erase_nb2hop_tuple(tuple);
 }
 
 ///
 /// \brief Adds an MPR selector tuple to the MPR Selector Set.
 ///
 /// Advertised Neighbor Sequence Number (ANSN) is also updated.
 ///
 /// \param tuple the MPR selector tuple to be added.
 ///
 void
 OLSR::add_mprsel_tuple(OLSR_mprsel_tuple* tuple) {
   debug("%f: Node %d adds MPR selector tuple: nb_addr = %d\n", CURRENT_TIME,
     OLSR::node_id(ra_addr()), OLSR::node_id(tuple->main_addr()));
 
   state_.insert_mprsel_tuple(tuple);
   ansn_ = (ansn_ + 1)%(OLSR_MAX_SEQ_NUM + 1);
 }
 
 ///
 /// \brief Removes an MPR selector tuple from the MPR Selector Set.
 ///
 /// Advertised Neighbor Sequence Number (ANSN) is also updated.
 ///
 /// \param tuple the MPR selector tuple to be removed.
 ///
 void
 OLSR::rm_mprsel_tuple(OLSR_mprsel_tuple* tuple) {
   debug("%f: Node %d removes MPR selector tuple: nb_addr = %d\n", CURRENT_TIME,
     OLSR::node_id(ra_addr()), OLSR::node_id(tuple->main_addr()));
 
   state_.erase_mprsel_tuple(tuple);
   ansn_ = (ansn_ + 1)%(OLSR_MAX_SEQ_NUM + 1);
 }
 
 ///
 /// \brief Adds a topology tuple to the Topology Set.
 ///
 /// \param tuple the topology tuple to be added.
 ///
 void
 OLSR::add_topology_tuple(OLSR_topology_tuple* tuple) {
   debug("%f: Node %d adds topology tuple: dest_addr = %d last_addr = %d seq = %d\n",
     CURRENT_TIME, OLSR::node_id(ra_addr()), OLSR::node_id(tuple->dest_addr()),
     OLSR::node_id(tuple->last_addr()), tuple->seq());
 
   state_.insert_topology_tuple(tuple);
 }
 
 ///
 /// \brief Removes a topology tuple from the Topology Set.
 ///
 /// \param tuple the topology tuple to be removed.
 ///
 void
 OLSR::rm_topology_tuple(OLSR_topology_tuple* tuple) {
   debug("%f: Node %d removes topology tuple: dest_addr = %d last_addr = %d seq = %d\n",
     CURRENT_TIME, OLSR::node_id(ra_addr()), OLSR::node_id(tuple->dest_addr()),
     OLSR::node_id(tuple->last_addr()), tuple->seq());
 
   state_.erase_topology_tuple(tuple);
 }
 
 ///
 /// \brief Adds an interface association tuple to the Interface Association Set.
 ///
 /// \param tuple the interface association tuple to be added.
 ///
 void
 OLSR::add_ifaceassoc_tuple(OLSR_iface_assoc_tuple* tuple) {
   debug("%f: Node %d adds iface association tuple: main_addr = %d iface_addr = %d\n",
     CURRENT_TIME, OLSR::node_id(ra_addr()), OLSR::node_id(tuple->main_addr()),
     OLSR::node_id(tuple->iface_addr()));
 
   state_.insert_ifaceassoc_tuple(tuple);
 }
 
 ///
 /// \brief Removes an interface association tuple from the Interface Association Set.
 ///
 /// \param tuple the interface association tuple to be removed.
 ///
 void
 OLSR::rm_ifaceassoc_tuple(OLSR_iface_assoc_tuple* tuple) {
   debug("%f: Node %d removes iface association tuple: main_addr = %d iface_addr = %d\n",
     CURRENT_TIME, OLSR::node_id(ra_addr()), OLSR::node_id(tuple->main_addr()),
     OLSR::node_id(tuple->iface_addr()));
 
   state_.erase_ifaceassoc_tuple(tuple);
 }
 
 ///
 /// \brief Gets the main address associated with a given interface address.
 ///
 /// \param iface_addr the interface address.
 /// \return the corresponding main address.
 ///
 nsaddr_t
 OLSR::get_main_addr(nsaddr_t iface_addr) {
   OLSR_iface_assoc_tuple* tuple = state_.find_ifaceassoc_tuple(iface_addr);
 
   if (tuple != NULL)
     return tuple->main_addr();
   return iface_addr;
 }
 
 ///
 /// \brief Determines which sequence number is bigger (as it is defined in RFC 3626).
 ///
 /// \param s1 a sequence number.
 /// \param s2 a sequence number.
 /// \return true if s1 > s2, false in other case.
 ///
 bool
 OLSR::seq_num_bigger_than(u_int16_t s1, u_int16_t s2) {
   return (s1 > s2 && s1-s2 <= OLSR_MAX_SEQ_NUM/2)
     || (s2 > s1 && s2-s1 > OLSR_MAX_SEQ_NUM/2);
 }
 
 ///
 /// \brief This auxiliary function (defined in RFC 3626) is used for calculating the MPR Set.
 ///
 /// \param tuple the neighbor tuple which has the main address of the node we are going to calculate its degree to.
 /// \return the degree of the node.
 ///
 int
 OLSR::degree(OLSR_nb_tuple* tuple) {
   int degree = 0;
   for (nb2hopset_t::iterator it = nb2hopset().begin(); it != nb2hopset().end(); it++) {
     OLSR_nb2hop_tuple* nb2hop_tuple = *it;
     if (nb2hop_tuple->nb_main_addr() == tuple->nb_main_addr()) {
       OLSR_nb_tuple* nb_tuple =
         state_.find_nb_tuple(nb2hop_tuple->nb_main_addr());
       if (nb_tuple == NULL)
         degree++;
     }
   }
   return degree;
 }
 
 ///
 /// \brief Converts a decimal number of seconds to the mantissa/exponent format.
 ///
 /// \param seconds decimal number of seconds we want to convert.
 /// \return the number of seconds in mantissa/exponent format.
 ///
 u_int8_t
 OLSR::seconds_to_emf(double seconds) {
   // This implementation has been taken from unik-olsrd-0.4.5 (mantissa.c),
   // licensed under the GNU Public License (GPL)
 
   int a, b = 0;
    while (seconds/OLSR_C >= pow((double)2, (double)b))
     b++;
   b--;
 
   if (b < 0) {
     a = 1;
     b = 0;
   }
   else if (b > 15) {
     a = 15;
     b = 15;
   }
   else {
     a = (int)(16*((double)seconds/(OLSR_C*(double)pow(2, b))-1));
     while (a >= 16) {
       a -= 16;
       b++;
     }
   }
 
   return (u_int8_t)(a*16+b);
 }
 
 ///
 /// \brief Converts a number of seconds in the mantissa/exponent format to a decimal number.
 ///
 /// \param olsr_format number of seconds in mantissa/exponent format.
 /// \return the decimal number of seconds.
 ///
 double
 OLSR::emf_to_seconds(u_int8_t olsr_format) {
   // This implementation has been taken from unik-olsrd-0.4.5 (mantissa.c),
   // licensed under the GNU Public License (GPL)
   int a = olsr_format >> 4;
   int b = olsr_format - a*16;
   return (double)(OLSR_C*(1+(double)a/16)*(double)pow(2,b));
 }
 
 ///
 /// \brief Returns the identifier of a node given the address of the attached OLSR agent.
 ///
 /// \param addr the address of the OLSR routing agent.
 /// \return the identifier of the node.
 ///
 int
 OLSR::node_id(nsaddr_t addr) {
   // Preventing a bad use for this function
         if ((u_int32_t)addr == IP_BROADCAST)
     return addr;
   // Getting node id
   Node* node = Node::get_node_by_address(addr);
   assert(node != NULL);
   return node->nodeid();
 }

