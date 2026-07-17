# Packet Monitor/Sniffer
- C (Linux), libwebsockets, vanilla html/css/js

Linux Packet sniffer built with raw packets in raw C that captures and filters them into a html, css, js through Websockets which is implemented using libwebsockets. 

Architecture:
- Two processes connected with a pipe; the packet sniffer and the web socket server
- Sniffer: takes cmd args, opens a raw packet socket after giving capability then drop capability and permission (libcap), parses packets IPv4 UDP and TCP with strict bounds checking, outputs json to web socket
- WebSocket Server: Buffers into a ring, opens local html, cs, js page (checking origin) (libwebsocket)
- Single threaded, event driven

## Demo
<p align="center">
  <img src="photos/packetsniffergif.gif" width="700" alt="Demo">
</p>

## Running for yourself!
Requirements:
- Linux, gcc, cmake, libcap, libwebsockets
```bash
sudo apt install build-essential libcap-dev cmake
```
Build:
```bash
git clone --recurse-submodules https://github.com/IanWorldHi/PacketSnifferMonitor.git
cd PacketSnifferMonitor/libs/libwebsockets
mkdir build && cd build
cmake .. -DLWS_WITHOUT_TESTAPPS=ON -DLWS_WITH_SSL=OFF
make && sudo make install
sudo ldconfig
cd ../../../Code/
gcc sniffer2.c -o sniffer -D_GNU_SOURCE -Wall -Wextra -lcap  
gcc ws1.c -o ws -Wall -Wextra -lwebsockets
```
Run:
```bash
sudo setcap cap_net_raw+p ./sniffer
./sniffer <flags> | ./ws
```
Filtering With Flags:
<table>
<tr><th>Long option</th><th>Has argument</th><th>Short flag</th><th>Description</th></tr>
<tr><td><code>--sip</code></td><td>required</td><td><code>-s</code></td><td>source ip</td></tr>
<tr><td><code>--dip</code></td><td>required</td><td><code>-d</code></td><td>dest ip</td></tr>
<tr><td><code>--sport</code></td><td>required</td><td><code>-p</code></td><td>source port</td></tr>
<tr><td><code>--dport</code></td><td>required</td><td><code>-q</code></td><td>dest port</td></tr>
<tr><td><code>--sif</code></td><td>required</td><td><code>-i</code></td><td>source interface</td></tr>
<tr><td><code>--dif</code></td><td>required</td><td><code>-j</code></td><td>dest interface</td></tr>
<tr><td><code>--logfile</code></td><td>required</td><td><code>-f</code></td><td>log file name</td></tr>
<tr><td><code>--tcp</code></td><td>none</td><td><code>-t</code></td><td>use TCP</td></tr>
<tr><td><code>--udp</code></td><td>none</td><td><code>-u</code></td><td>use UDP</td></tr>
</table>


### Improvements Roadmap
- add IPv6, handling for other protocols, promiscuous mode
- Use PACKET_MMAP, optimized version of af_packet, ring buffer
- Use BPF to filter before copying into memmory
- Switch from lws to raw c for learning and cuztomization
- Clean up: (Change to header - do something with the logfile's more detailed output - fix edge case) 

<!-- 
(Rough work/notes moved to Storage)

Add customization for ouput of frontend, or option to continue using the log file which outputs a lot more data about the packets which I commented out for now
Clean up comments
change including ws1.c to a header file
Fix case of line longer than buffer not just to EOF
-->




