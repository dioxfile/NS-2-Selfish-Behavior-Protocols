#!/bin/dash
FILE_TRAFFIC="$1"
rm t.txt -v
rm TX_RX_Nodes.txt -v
cat $FILE_TRAFFIC | egrep "connecting" | awk -F" " '{print $2}' > t.txt
cat $FILE_TRAFFIC | egrep "connecting" | awk -F" " '{print $5}' >> t.txt
sort -u t.txt > TX_RX_Nodes.txt
