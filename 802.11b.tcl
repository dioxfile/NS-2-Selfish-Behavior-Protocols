
#Script TCL Wireless Mesh Network Using Wired-Cum-Wireless NS-2
#=================DEFINIÇÕES=====================================================================================
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
  set val(x)               1000;#3410                ;# Axis X 
  set val(y)               1000;#1810                ;# Axis Y
  set val(trafego)           2;#12                   ;# Traffic Source Quantity
  set val(TX)                 1.2W                   ;# Default NS2 - 0.400 -> 0,000509W/PKT
  set val(RX)                 0.6W                   ;# Default NS2 - 0.300 -> 0.000156W/PKT 
  set val(IniEner)          100.00                   ;# Initial Energy
  set val(ModEner)         EnergyModel               ;# Energy Model
  set val(termina)           50                      ;# Simulation Time
#===============================================================================================================#

# ---------------------BEGIN OLSR EXTENSIONS----------------------------

Agent/OLSR set mpr_algorithm_               1    ;# 1 = RFC 3626, 2 = MPRR1, 3 = MPRR2, 4 = QOLSR, 5 = OLSRD
Agent/OLSR set routing_algorithm_           2    ;# OLSR Routing = 1 e Dijkstra = 2;
Agent/OLSR set link_quality_                4    ;# OLSR = Hop-count, ETX = 2, ML = 3 e MD = true 
Agent/OLSR set link_delay_                 false ;# Se true LD métrica será utilizada
Agent/OLSR set c_alpha_             	    0.4  ;#Smoothing Factor OLSR-MD only.
Agent/OLSR set willingness_         	    3    ;# Default (as published in RFC 3626)
Agent/OLSR set tc_ival_                     5    ;# Default (as published in RFC 3626)
Agent/OLSR set hello_ival_            	    2    ;# Default (as published in RFC 3626)


#==================Criando uma NIC para o padrão IEEE 802.11b=====================================#
#===========================HR-DSSS (IEEE802.11b) ================================================#
   $val(mac)       set SlotTime_            0.000020        ;# 20us
   $val(mac)       set SIFS_                0.000010        ;# 10us
   $val(mac)       set DIFS_                0.000050        ;# 50us
   $val(mac)       set CWMin_              31               ;# Limite Mínimo de CW p/ cálculo do Backoff [0, CW]
   $val(mac)       set CWMax_          	 1023               ;# Limite Máximo de CW p/ cálculo do Backoff [0, CW]
   $val(mac)       set PreambleLength_    144               ;# 144 bit
   $val(mac)       set PLCPHeaderLength_   48               ;# 48 bits MAC_Address
   $val(mac)       set PLCPDataRate_        1.0e6           ;# 1Mbps
   $val(mac)       set dataRate_           11.0e6           ;# 11Mbps
   $val(wlan0)     set bandwidth_          11.0e6           ;#Largura de Banda
   $val(mac)       set basicRate_           1.0e6           ;# 1Mbps
   $val(wlan0)     set freq_                2.4e9           ;# frequencia (9.14 GHz 802.11b) 537.417m.
   $val(wlan0)     set Pt_                 3.3962527e-2
;#10.3962527e-2    ;# Potência do transmissor da placa TX 10.3962527e-2 537.417m.
   $val(wlan0)     set RXThresh_            6.309573e-12    ;#200m     ;# Potência da Sensibilidade do Receptor RX 537.417m. 
   $val(wlan0)     set CSThresh_            6.309573e-12    ;#200m      ;# Potência da Sensibilidade da Portadora CarrierSense 537.417
   #$val(wlan0)     set freq_                9.14e+08        ;# frequencia (2.4 GHz 802.11b) 537.417m.
   #$val(wlan0)     set Pt_                  0.281838        ;# Potência do transmissor da placa TX 10.3962527e-2 537.417m.
   #$val(wlan0)     set RXThresh_            1.7098e-11      ;#200m     ;# Potência da Sensibilidade do Receptor RX 537.417m. 
   #$val(wlan0)     set CSThresh_            1.7098e-11      ;#200m      ;# Potência da Sensibilidade da Portadora CarrierSense 537.417
   $val(wlan0)     set RTSThreshold_     3000               ;# Use RTS/CTS for packets larger 3000 bytes 

#Adicionando ou Removendo pacotes na simulação
#remove-all-packet-headers
#remove-packet-header OLSR
#add-packet-header IP TCP

#Início da Simulação
set ns_ [new Simulator]
$defaultRNG seed NEW_SEED

$ns_ color 1 blue
$ns_ color 2 red
$ns_ color 3 green

#Usar o novo formato de trace file
#$ns_ use-newtrace

# Abrindo e Gravando no arquivo trace
set ArquivoTrace [open TRACE_Arquivo.tr w]
$ns_ trace-all $ArquivoTrace

#trace da Vazão (Throughput)
#Configura  o trace da vazãoset tempo [$rng uniform 48 50]
for {set v 0} {$v < $val(trafego)} {incr v} {
set th($v) [open throughput$v.tr w]
}

#Grava vazão global
set geral [open V_Global.dio w]

# Abrindo e Gravando no arquivo NAM
set ArquivoNam [open NAM_Arquivo.nam w]
$ns_ namtrace-all $ArquivoNam
$ns_ namtrace-all-wireless $ArquivoNam $val(x) $val(y)

#Definição da Topologia
set topologia [new Topography]
$topologia  load_flatgrid $val(x) $val(y)

#Armazena Informação de conectividade da Topologia "GOD (General Operations Director)"
set god_ [create-god $val(node)]

#Inicializando o canal 1
set chan_11_ [new $val(canal)]

#Definição Técnica do node IEEE 802.11 DCF Mode
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

#Última linha acima ativa o roteamento para redes cabeadas no AP

#nos wireless
for {set i 0} {$i < $val(node)} {incr i} {
    $ns_ node-config -initialEnergy 100
    set node($i) [$ns_ node]
    $node($i) color green
    $ns_ at 0.0 "$node($i) label WN_$i"
    $node($i) random-motion 0 ;#desativado
}

################Aqui inicia tráfego e padrão de movimentação##############################
#Distribuição dos nodes usando modelo de mobilidade Random WayPoint
puts "Iniciando o padrão de movimentação Random Waypoint se a simulação não for estática..."
source "mobility.tcl" 
puts "Iniciando tráfego"
source "mesh_traffic.tcl"

############################################################################
#puts "Iniciando Selfish Nodes..."
#source "Selfish_10.tcl"
$ns_ at 0.0 "[$node(11) set ragent_] egoista_on"
$ns_ at 0.0 "[$node(3) set ragent_] egoista_on"
$ns_ at 0.0 "[$node(12) set ragent_] egoista_on"
$ns_ at 0.0 "[$node(46) set ragent_] egoista_on"
$ns_ at 0.0 "[$node(5) set ragent_] egoista_on"
#Define a posiçãons inicial do nó no nam
for {set n 0} {$n < $val(node) } {incr n} {
 $ns_ initial_node_pos $node($n) 20
}

# Diz aos nós quando a simulação para no ns
for {set n 0} {$n < $val(node) } {incr n} {
 $ns_ at $val(termina).000 "$node($n) reset";
}


proc final {} {
global ns_ ArquivoTrace ArquivoNam th val geral
$ns_ flush-trace
close $ArquivoTrace
close $ArquivoNam
close $geral
 for {set v 0} {$v < $val(trafego) } {incr v} {
close $th($v)
}
exec nam NAM_Arquivo.nam &
exec xgraph V_Global.dio -geometry 800x400 &
exit 0
}

puts "Iniciando a SIMULAÇÃO (DioxSimulator)..."
#$ns_ at 0.0 "grava"
$ns_ at $val(termina).001 "$ns_ nam-end-wireless $val(termina)"
$ns_ at $val(termina).002 "puts \"FIM DA SIMULAÇÃO...\"; final"
$ns_ at $val(termina).003 "$ns_ halt"
$ns_ run
