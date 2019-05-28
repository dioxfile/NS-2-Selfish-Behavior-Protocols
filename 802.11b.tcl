
#Script TCL Wireless Mesh/Ad-Hoc Networks
#=================DEFINITIONS=====================================================================================
  global val
  global defaultRNG                                  ;# RNG Global Var
  set val(canal)           Channel/WirelessChannel   ;# Channel(1-11)
  set val(propacacao)      Propagation/TwoRayGround  ;# Radio Propagation
  set val(antena)          Antenna/OmniAntenna       ;# Aerial (omni/straight)
  set val(layer2)          LL                        ;# Link Layer
  set val(drop)            Queue/DropTail/PriQueue   ;# Queue type
  set val(fileSize)          50                      ;# Queue size
  set val(wlan0)           Phy/WirelessPhy           ;# DSSS
  set val(mac)             Mac/802_11                ;# MAC Type
  set val(routP)           AODV                      ;# Routing Protocol
 if { $val(routP) == "DSR" } {                       ;# Only DSR
  set val(drop)            CMUPriQueue		 
  } else {
  set val(drop)            Queue/DropTail/PriQueue   ;# FIFO Drop Queue
  }                                                  
  set val(node)              50                      ;# Node Number
  set val(x)               1000                      ;# Axis X 
  set val(y)               1000                      ;# Axis Y
  set val(trafego)           10                      ;# Traffic Source Quantity
  set val(TX)                 1.2W                   ;# Default NS2 - 0.400 -> 0,000509W/PKT
  set val(RX)                 0.6W                   ;# Default NS2 - 0.300 -> 0.000156W/PKT 
  set val(IniEner)          100.00                   ;# Initial Energy
  set val(ModEner)         EnergyModel               ;# Energy Model
  set val(termina)           50                      ;# Simulation Time
#===============================================================================================================#

# ---------------------BEGIN OLSR EXTENSIONS----------------------------

Agent/OLSR set mpr_algorithm_               1    ;# 1 = RFC 3626, 2 = MPRR1, 3 = MPRR2, 4 = QOLSR, 5 = OLSRD
Agent/OLSR set routing_algorithm_           1    ;# OLSR Routing = 1 e Dijkstra = 2;
Agent/OLSR set link_quality_                1    ;# OLSR -> Hop-count = 1, ETX = 2, ML = 3 e MD = true 
Agent/OLSR set link_delay_                 false ;# If true LD will be used
Agent/OLSR set c_alpha_             	    0.4  ;# Smoothing Factor OLSR-MD only.
Agent/OLSR set willingness_         	    3    ;# Default (as published in RFC 3626)
Agent/OLSR set tc_ival_                     5    ;# Default (as published in RFC 3626)
Agent/OLSR set hello_ival_            	    2    ;# Default (as published in RFC 3626)


#==================NIC IEEE 802.11b=====================================#
#========================== HR-DSSS (IEEE802.11b) ======================#
   $val(mac)       set SlotTime_            0.000020        ;# 20us
   $val(mac)       set SIFS_                0.000010        ;# 10us
   $val(mac)       set DIFS_                0.000050        ;# 50us
   $val(mac)       set CWMin_              31               ;# Min Backoff [0, CW]
   $val(mac)       set CWMax_          	 1023               ;#Max Backoff [0, CW]
   $val(mac)       set PreambleLength_    144               ;# 144 bit
   $val(mac)       set PLCPHeaderLength_   48               ;# 48 bits MAC_Address
   $val(mac)       set PLCPDataRate_        1.0e6           ;# 1Mbps
   $val(mac)       set dataRate_           11.0e6           ;# 11Mbps
   $val(wlan0)     set bandwidth_          11.0e6           ;# Bandwidth
   $val(mac)       set basicRate_           1.0e6           ;# 1Mbps
   $val(wlan0)     set freq_                2.4e9           ;# 2.4 GHz 802.11b.
   $val(wlan0)     set Pt_                 3.3962527e-2     ;# Power TX.
   $val(wlan0)     set RXThresh_            6.309573e-12    ;# RX Threshold. 
   $val(wlan0)     set CSThresh_            6.309573e-12    ;# Carrie Sense Threshold. 
   $val(wlan0)     set RTSThreshold_     3000               ;# Use RTS/CTS for packets larger 3000 bytes 

#begin Simulation
set ns_ [new Simulator]
$defaultRNG seed NEW_SEED

$ns_ color 1 blue
$ns_ color 2 red
$ns_ color 3 green

# Trace File Writing
set ArquivoTrace [open TRACE_Arquivo.tr w]
$ns_ trace-all $ArquivoTrace

# NAM File  Writing
set ArquivoNam [open NAM_Arquivo.nam w]
$ns_ namtrace-all $ArquivoNam
$ns_ namtrace-all-wireless $ArquivoNam $val(x) $val(y)

# Topology
set topologia [new Topography]
$topologia  load_flatgrid $val(x) $val(y)

# "GOD (General Operations Director)"
set god_ [create-god $val(node)]

#Starting Channel 1
set chan_11_ [new $val(canal)]

# Node Setup
$ns_ node-config -adhocRouting $val(routP) \
    -llType $val(layer2) \
		-macType $val(mac) \
		-ifqType $val(drop) \
		-ifqLen $val(fileSize) \
		-antType $val(antena) \
		-propType $val(propacacao) \
		-phyType $val(wlan0) \
		-topoInstance $topologia \
		-channel $chan_11_ \
		-agentTrace ON \
		-routerTrace ON \
		-macTrace ON \
		-movementTrace OFF \
		-energyModel $val(ModEner) \
		-initialEnergy $val(IniEner) \
		-txPower $val(TX) \
		-rxPower $val(RX) \
		-wiredRouting OFF

# Wireless
for {set i 0} {$i < $val(node)} {incr i} {
    set node($i) [$ns_ node]
    $node($i) color green
    $ns_ at 0.0 "$node($i) label WN_$i"
    $node($i) random-motion 0 ;# disable
}

################Starting Random WayPoint Model and Traffic model CBR##############################
puts "Starting Random WayPoint (eg., file mobility.tcl)."
source "mobility.tcl" 
puts "Starting Traffic"
source "mesh_traffic.tcl"

############################################################################
puts "Starting Selfish Nodes (eg., Selfish.tcl)..."
source "Selfish.tcl"
#can be done that way too, for instance:
#$ns_ at 0.0 "[$node(11) set ragent_] egoista_on"
#$ns_ at 0.0 "[$node(3) set ragent_] egoista_on"
#$ns_ at 0.0 "[$node(12) set ragent_] egoista_on"
#$ns_ at 0.0 "[$node(46) set ragent_] egoista_on"
#$ns_ at 0.0 "[$node(5) set ragent_] egoista_on"

# NAM Position 
for {set n 0} {$n < $val(node) } {incr n} {
 $ns_ initial_node_pos $node($n) 20
}

# Stop nodes simulation
for {set n 0} {$n < $val(node) } {incr n} {
 $ns_ at $val(termina).000 "$node($n) reset";
}


proc final {} {
global ns_ ArquivoTrace ArquivoNam val
$ns_ flush-trace
close $ArquivoTrace
close $ArquivoNam
}
exec nam NAM_Arquivo.nam &
exit 0
}

puts "Starting Simulation"
$ns_ at $val(termina).001 "$ns_ nam-end-wireless $val(termina)"
$ns_ at $val(termina).002 "puts \"END SIMULATION...\"; final"
$ns_ at $val(termina).003 "$ns_ halt"
$ns_ run
