#!/bin/bash
curl 'http://interactive.guim.co.uk/2015/the-counted/v/1460319534281/js/main.js' 2>/dev/null | grep 'var total$1' | sed -e 's:.* = ::' -e 's:;.*::' -e 's:^:V:'
