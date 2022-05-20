#
# nodes: 50, max conn: 8, send rate: 0.00390625, seed: 0.25
#
#
# 0 connecting to 1 at time 8.0978678042524805
#
set udp_(0) [new Agent/UDP]
$ns_ attach-agent $node_(0) $udp_(0)
set null_(0) [new Agent/Null]
$ns_ attach-agent $node_(1) $null_(0)
set cbr_(0) [new Application/Traffic/CBR]
$cbr_(0) set packetSize_ 500
$cbr_(0) set rate_ 256.0kb
$cbr_(0) set random_ 1
$cbr_(0) attach-agent $udp_(0)
$ns_ connect $udp_(0) $null_(0)
$ns_ at 8.0978678042524805 "$cbr_(0) start"
#
# 2 connecting to 3 at time 8.0951477136905989
#
set udp_(1) [new Agent/UDP]
$ns_ attach-agent $node_(2) $udp_(1)
set null_(1) [new Agent/Null]
$ns_ attach-agent $node_(3) $null_(1)
set cbr_(1) [new Application/Traffic/CBR]
$cbr_(1) set packetSize_ 500
$cbr_(1) set rate_ 256.0kb
$cbr_(1) set random_ 1
$cbr_(1) attach-agent $udp_(1)
$ns_ connect $udp_(1) $null_(1)
$ns_ at 8.0951477136905989 "$cbr_(1) start"
#
# 4 connecting to 5 at time 8.0381870033862945
#
set udp_(2) [new Agent/UDP]
$ns_ attach-agent $node_(4) $udp_(2)
set null_(2) [new Agent/Null]
$ns_ attach-agent $node_(5) $null_(2)
set cbr_(2) [new Application/Traffic/CBR]
$cbr_(2) set packetSize_ 500
$cbr_(2) set rate_ 256.0kb
$cbr_(2) set random_ 1
$cbr_(2) attach-agent $udp_(2)
$ns_ connect $udp_(2) $null_(2)
$ns_ at 8.0381870033862945 "$cbr_(2) start"
#
# 6 connecting to 7 at time 8.0356390442399483
#
set udp_(3) [new Agent/UDP]
$ns_ attach-agent $node_(6) $udp_(3)
set null_(3) [new Agent/Null]
$ns_ attach-agent $node_(7) $null_(3)
set cbr_(3) [new Application/Traffic/CBR]
$cbr_(3) set packetSize_ 500
$cbr_(3) set rate_ 256.0kb
$cbr_(3) set random_ 1
$cbr_(3) attach-agent $udp_(3)
$ns_ connect $udp_(3) $null_(3)
$ns_ at 8.0356390442399483 "$cbr_(3) start"
#
# 11 connecting to 12 at time 8.0553242353048748
#
set udp_(4) [new Agent/UDP]
$ns_ attach-agent $node_(11) $udp_(4)
set null_(4) [new Agent/Null]
$ns_ attach-agent $node_(12) $null_(4)
set cbr_(4) [new Application/Traffic/CBR]
$cbr_(4) set packetSize_ 500
$cbr_(4) set rate_ 256.0kb
$cbr_(4) set random_ 1
$cbr_(4) attach-agent $udp_(4)
$ns_ connect $udp_(4) $null_(4)
$ns_ at 8.0553242353048748 "$cbr_(4) start"
#
# 12 connecting to 13 at time 8.0643638100309118
#
set udp_(5) [new Agent/UDP]
$ns_ attach-agent $node_(12) $udp_(5)
set null_(5) [new Agent/Null]
$ns_ attach-agent $node_(13) $null_(5)
set cbr_(5) [new Application/Traffic/CBR]
$cbr_(5) set packetSize_ 500
$cbr_(5) set rate_ 256.0kb
$cbr_(5) set random_ 1
$cbr_(5) attach-agent $udp_(5)
$ns_ connect $udp_(5) $null_(5)
$ns_ at 8.0643638100309118 "$cbr_(5) start"
#
# 14 connecting to 15 at time 8.0646923901814471
#
set udp_(6) [new Agent/UDP]
$ns_ attach-agent $node_(14) $udp_(6)
set null_(6) [new Agent/Null]
$ns_ attach-agent $node_(15) $null_(6)
set cbr_(6) [new Application/Traffic/CBR]
$cbr_(6) set packetSize_ 500
$cbr_(6) set rate_ 256.0kb
$cbr_(6) set random_ 1
$cbr_(6) attach-agent $udp_(6)
$ns_ connect $udp_(6) $null_(6)
$ns_ at 8.0646923901814471 "$cbr_(6) start"
#
# 15 connecting to 16 at time 8.0136353589192204
#
set udp_(7) [new Agent/UDP]
$ns_ attach-agent $node_(15) $udp_(7)
set null_(7) [new Agent/Null]
$ns_ attach-agent $node_(16) $null_(7)
set cbr_(7) [new Application/Traffic/CBR]
$cbr_(7) set packetSize_ 500
$cbr_(7) set rate_ 256.0kb
$cbr_(7) set random_ 1
$cbr_(7) attach-agent $udp_(7)
$ns_ connect $udp_(7) $null_(7)
$ns_ at 8.0136353589192204 "$cbr_(7) start"
#
#Total sources/connections: 8/8
#
