#
# nodes: 50, max conn: 10, send rate: 0.0078125, seed: 0.75
#
#
# 2 connecting to 3 at time 10.091066715303374
#
set udp_(0) [new Agent/UDP]
$ns_ attach-agent $node(2) $udp_(0)
set null_(0) [new Agent/Null]
$ns_ attach-agent $node(3) $null_(0)
set cbr_(0) [new Application/Traffic/CBR]
$cbr_(0) set packetSize_ 1000
$cbr_(0) set rate_ 128.0kb
$cbr_(0) set random_ 1
$cbr_(0) set maxpkts_ 1000
$cbr_(0) attach-agent $udp_(0)
$ns_ connect $udp_(0) $null_(0)
$ns_ at 10.091066715303374 "$cbr_(0) start"
#
# 4 connecting to 5 at time 10.057555911018307
#
set udp_(1) [new Agent/UDP]
$ns_ attach-agent $node(4) $udp_(1)
set null_(1) [new Agent/Null]
$ns_ attach-agent $node(5) $null_(1)
set cbr_(1) [new Application/Traffic/CBR]
$cbr_(1) set packetSize_ 1000
$cbr_(1) set rate_ 128.0kb
$cbr_(1) set random_ 1
$cbr_(1) set maxpkts_ 1000
$cbr_(1) attach-agent $udp_(1)
$ns_ connect $udp_(1) $null_(1)
$ns_ at 10.057555911018307 "$cbr_(1) start"
#
# 6 connecting to 7 at time 10.041585770501563
#
set udp_(2) [new Agent/UDP]
$ns_ attach-agent $node(6) $udp_(2)
set null_(2) [new Agent/Null]
$ns_ attach-agent $node(7) $null_(2)
set cbr_(2) [new Application/Traffic/CBR]
$cbr_(2) set packetSize_ 1000
$cbr_(2) set rate_ 128.0kb
$cbr_(2) set random_ 1
$cbr_(2) set maxpkts_ 1000
$cbr_(2) attach-agent $udp_(2)
$ns_ connect $udp_(2) $null_(2)
$ns_ at 10.041585770501563 "$cbr_(2) start"
#
# 7 connecting to 8 at time 10.022502297546017
#
set udp_(3) [new Agent/UDP]
$ns_ attach-agent $node(7) $udp_(3)
set null_(3) [new Agent/Null]
$ns_ attach-agent $node(8) $null_(3)
set cbr_(3) [new Application/Traffic/CBR]
$cbr_(3) set packetSize_ 1000
$cbr_(3) set rate_ 128.0kb
$cbr_(3) set random_ 1
$cbr_(3) set maxpkts_ 1000
$cbr_(3) attach-agent $udp_(3)
$ns_ connect $udp_(3) $null_(3)
$ns_ at 10.022502297546017 "$cbr_(3) start"
#
# 8 connecting to 9 at time 10.036486962035525
#
set udp_(4) [new Agent/UDP]
$ns_ attach-agent $node(8) $udp_(4)
set null_(4) [new Agent/Null]
$ns_ attach-agent $node(9) $null_(4)
set cbr_(4) [new Application/Traffic/CBR]
$cbr_(4) set packetSize_ 1000
$cbr_(4) set rate_ 128.0kb
$cbr_(4) set random_ 1
$cbr_(4) set maxpkts_ 1000
$cbr_(4) attach-agent $udp_(4)
$ns_ connect $udp_(4) $null_(4)
$ns_ at 10.036486962035525 "$cbr_(4) start"
#
# 9 connecting to 10 at time 10.083444275932127
#
set udp_(5) [new Agent/UDP]
$ns_ attach-agent $node(9) $udp_(5)
set null_(5) [new Agent/Null]
$ns_ attach-agent $node(10) $null_(5)
set cbr_(5) [new Application/Traffic/CBR]
$cbr_(5) set packetSize_ 1000
$cbr_(5) set rate_ 128.0kb
$cbr_(5) set random_ 1
$cbr_(5) set maxpkts_ 1000
$cbr_(5) attach-agent $udp_(5)
$ns_ connect $udp_(5) $null_(5)
$ns_ at 10.083444275932127 "$cbr_(5) start"
#
# 10 connecting to 11 at time 10.03376933645167
#
set udp_(6) [new Agent/UDP]
$ns_ attach-agent $node(10) $udp_(6)
set null_(6) [new Agent/Null]
$ns_ attach-agent $node(11) $null_(6)
set cbr_(6) [new Application/Traffic/CBR]
$cbr_(6) set packetSize_ 1000
$cbr_(6) set rate_ 128.0kb
$cbr_(6) set random_ 1
$cbr_(6) set maxpkts_ 1000
$cbr_(6) attach-agent $udp_(6)
$ns_ connect $udp_(6) $null_(6)
$ns_ at 10.03376933645167 "$cbr_(6) start"
#
# 12 connecting to 13 at time 10.01482775761505
#
set udp_(7) [new Agent/UDP]
$ns_ attach-agent $node(12) $udp_(7)
set null_(7) [new Agent/Null]
$ns_ attach-agent $node(13) $null_(7)
set cbr_(7) [new Application/Traffic/CBR]
$cbr_(7) set packetSize_ 1000
$cbr_(7) set rate_ 128.0kb
$cbr_(7) set random_ 1
$cbr_(7) set maxpkts_ 1000
$cbr_(7) attach-agent $udp_(7)
$ns_ connect $udp_(7) $null_(7)
$ns_ at 10.01482775761505 "$cbr_(7) start"
#
# 14 connecting to 15 at time 10.057858717235671
#
set udp_(8) [new Agent/UDP]
$ns_ attach-agent $node(14) $udp_(8)
set null_(8) [new Agent/Null]
$ns_ attach-agent $node(15) $null_(8)
set cbr_(8) [new Application/Traffic/CBR]
$cbr_(8) set packetSize_ 1000
$cbr_(8) set rate_ 128.0kb
$cbr_(8) set random_ 1
$cbr_(8) set maxpkts_ 1000
$cbr_(8) attach-agent $udp_(8)
$ns_ connect $udp_(8) $null_(8)
$ns_ at 10.057858717235671 "$cbr_(8) start"
#
# 17 connecting to 18 at time 10.032996380391063
#
set udp_(9) [new Agent/UDP]
$ns_ attach-agent $node(17) $udp_(9)
set null_(9) [new Agent/Null]
$ns_ attach-agent $node(18) $null_(9)
set cbr_(9) [new Application/Traffic/CBR]
$cbr_(9) set packetSize_ 1000
$cbr_(9) set rate_ 128.0kb
$cbr_(9) set random_ 1
$cbr_(9) set maxpkts_ 1000
$cbr_(9) attach-agent $udp_(9)
$ns_ connect $udp_(9) $null_(9)
$ns_ at 10.032996380391063 "$cbr_(9) start"
#
#Total sources/connections: 10/10
#
