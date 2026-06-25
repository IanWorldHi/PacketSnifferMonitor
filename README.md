# Network Programming

Running on Ubuntu on a VMware workstation pro instance
- just so testing works better as wsl i think will mess up some of the packets

IPv6
gcc
Running thru ssh to vs code

Built using only raw sockets in c with linux sockets.  
Using AF_PACKET (raw socket type) not PACKET_MMAP

<!-- 
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

Next steps/Improvements:
Use PACKET_MMAP - it's the optimized version of af_packet much higher performance
- normally i do a recvfrom which is a system call (program->kernel and back, the kernel copies its buffer into the program's): hence high performance
- mmap: kernel and program share a chunk of memory "ring buffer" so rewrites same, exchanges ownership
- BPF: can process packets in the kernel (ie) filtering) before copying into memory (eBPF, cBPF)
- add exiting based on user input using poll to manage blocking
- refine poll with ppoll, if after while(!stop) before poll i get SIGINT or just libevent
- promiscuous mode

Here's the project idea:
A chat service type thing but to make it interesting you can give yourself a pfp and it'll show like "bill" calling 


-->




