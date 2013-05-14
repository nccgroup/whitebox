Whitebox Proto-type

Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Ollie Whitehouse, ollie dot whitehouse at nccgroup dot com

http://www.github.com/nccgroup/whitebox

Released under AGPL see LICENSE for more information

(c) 2008 - 2013 Ollie Whitehouse
(c) 2013 NCC Group Plc

[Protocol]
The protocol implemented between the controller and whitebox is vert simple. 
This is implemented in Engine.cpp. Basically there exis the following commands

SHUTDOWN   - Shuts down Whitebox
REBOOT     - Reboots the Windows machine
<filename> - anything else will be treated as a filename and passed to 
             ShellExecuteEx