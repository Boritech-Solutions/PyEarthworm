#!/bin/bash

echo "Sourcing ew_linux"
source "/opt/earthworm/bin/ew_linux.bash"
if [[ $? == 0 ]]; then echo "Sourced ew_linux"; else echo "Failed to source ew_linux"; fi
echo "Exec-ing args $@"
exec "$@"