# nodes: 50, max conn: 2, send rate: 0.0078125, seed: 0.75

# 35 connecting to 3 at time 10.0854402538786827
set udp_(0) [new Agent/UDP]
$ns_ attach-agent $node_(35) $udp_(0)
set null_(0) [new Agent/Null]
$ns_ attach-agent $node_(3) $null_(0)
set cbr_(0) [new Application/Traffic/CBR]
$cbr_(0) set packetSize_ 250
$cbr_(0) set rate_ 128.0kb
$cbr_(0) attach-agent $udp_(0)
$ns_ connect $udp_(0) $null_(0)
$ns_ at 10.0854402538786827 "$cbr_(0) start"

# 49 connecting to 16 at time 10.0421436907454131
set udp_(1) [new Agent/UDP]
$ns_ attach-agent $node_(49) $udp_(1)
set null_(1) [new Agent/Null]
$ns_ attach-agent $node_(16) $null_(1)
set cbr_(1) [new Application/Traffic/CBR]
$cbr_(1) set packetSize_ 250
$cbr_(1) set rate_ 128.0kb
$cbr_(1) attach-agent $udp_(1)
$ns_ connect $udp_(1) $null_(1)
$ns_ at 10.0421436907454131 "$cbr_(1) start"
#Total sources/connections: 2/2
