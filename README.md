# Packet Monitor/Sniffer
- C (Linux), libcap, libwebsockets, vanilla html/css/js

Packet sniffer built with raw packets in raw C that captures and filters them into a html, css, js through Websockets which is implemented using libwebsockets. 

Architecture:
- Two processes connected with a pipe; the packet sniffer and the web socket server
- Sniffer: takes cmd args, opens a raw packet socket after giving capability then drop capability and permission, parses packets IPv4 UDP and TCP with strict bounds checking, outputs json to web socket
- WebSocket Server: Buffers into a ring, opens local html, cs, js page (checking origin) 
- Single threaded, event driven, NOT thread safe

## Demo

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
cd ../../Code/
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

<!-- 
Improvements:

--!>

<!-- 
ldconfig?

Running on Ubuntu on a VMware workstation pro instance
- just so testing works better as wsl i think will mess up some of the packets

IPv6
gcc
Running thru ssh to vs code

Built using only raw sockets in c with linux sockets.  
Using AF_PACKET (raw socket type) not PACKET_MMAP

Can also make like a web extension to see the specific packets a website is sending/reciving
make a frontend/backend with java?
- can add a db to it to store like packets from what sources over time/what is the most frequent etc
i gotta learn some concurrency stuff - prob do it bymself b4 the course

Security Considerations:
- only giving capability needed and immediately cleared after it is used to create the socket
- only turning it on when running otherwise executable only has p not e
- CAP_NET_RAW, turned off +pe after socket made
- bound checks (checking all the lengths and not trusting them for what they say the are)
- signal handling for exiting
- poll for exiting via terminal
- websocket with libwebsockets (LWS)

Current frontend:
- just html css js connected to the backend with a websocket
- opened by running the c program with localhost

Next steps/Improvements:
- add IPv6, handling for other protocols, promiscuous mode
- Use PACKET_MMAP - it's the optimized version of af_packet much higher performance
- normally i do a recvfrom which is a system call (program->kernel and back, the kernel copies its buffer into the program's): hence high performance
- mmap: kernel and program share a chunk of memory "ring buffer" so rewrites same, exchanges ownership
- BPF: can process packets in the kernel (ie) filtering) before copying into memory (eBPF, cBPF)
- add exiting based on user input using poll to manage blocking
- refine poll with ppoll, if after while(!stop) before poll i get SIGINT or just libevent
- promiscuous mode4
- frontend improvement: run a node.js server in between so I can have a form page to add the filters and then switch pages securely
- switch from lws to raw c websocket handling the handshake etc



Here's the project idea:
A chat service type thing but to make it interesting you can give yourself a pfp and it'll show like "bill" calling 

Settign up libwebsockets:
https://libwebsockets.org/lws-api-doc-master/html/md_README_8build.html
Change settings.json to speicfy where build directory
Also error in two files - i will double check


compile: gcc ws1.c -o test -I/opt/libwebsockets/include -L/opt/libwebsockets/lib -lwebsockets -Wl,-rpath,/opt/libwebsockets/lib
run: ./test
Nah I set it right now. just gcc ws1.c -o test
./sniffer2 | ./test

just use pck-config, but i installed to opt so have to configure path
use a package manager vcpkg
use dockeffile for dev/build environment
should have installed to usr/local? submodule git?

I guess I should follow the header architecture

Path for adding sniffer event loop into the web socket one:
minimal-examples-lowlevel/raw/minimal-raw-file/minimal-raw-file.c

add ring buffer: https://github.com/warmcat/libwebsockets/blob/main/minimal-examples-lowlevel/ws-server/minimal-ws-server-ring/protocol_lws_minimal.c

I think there are a lot of other examples ofr other optimizations

Can change to also log to logfile at the same time

-->




