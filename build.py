#!/usr/bin/env python
#                      _____       .__         
#   ____  ____   _____/ ____\_____ |  | ___.__.
# _/ ___\/  _ \ /    \   __\____ \|  |<   |  |
# \  \__(  <_> )   |  \  |  |  |_> >  |_\___  |
#  \___  >____/|___|  /__|  |   __/|____/ ____|
#      \/           \/      |__|        \/
# launcher generated using:
# 
# python tools/confply/confply.py --launcher build.py

import sys
sys.path.append("tools/confply")
from confply import launcher

# fill this with your commands
aliases = {
    #'mycommand':'path/to/command.py'
    "rc_sample":"src/application/rc_sample/build.py",
    "net_physics_sample":"src/application/net_physics_sample/build.py"
}

if __name__ == "__main__":
    launcher(sys.argv[1:], aliases)
