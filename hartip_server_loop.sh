#!/bin/bash

cd /opt/plcnext/hartip

while true; do
	./arm_hartip_server >> hartip-server.log
	echo "crashed: $(date)" >> hartip-server.log
done
