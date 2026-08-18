#!/bin/bash

echo "Starting eacrthworm"
startstop >> /opt/earthworm/startstop_log 2>&1 &
[[ $? == 0 ]] && echo "Started earthworm"
[[ $? != 0 ]] && echo "Failed to start earthworm"
#startstop 2>&1 | tee -a /opt/earthworm/run/log/startstop_log &