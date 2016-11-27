#!/bin/bash

cd `dirname $0`
. ../common/get-directories

curl http://www.icebike.org/real-time-traffic-accident-statistics/ 2>/dev/null | grep count_ | head -1 | sed -e 's:.* = ::' -e 's:;.*::' -e "s:^:V:" -e "s:^:$FROM$BASE:"
