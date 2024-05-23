#!/usr/bin/env python3
import sys,os,threading,socket,struct

HOST="127.0.0.1"
PORT= 56453

def thread_job(fname):
    with socket.socket(socket.AF_INET,socket.SOCK_STREAM) as sock:
        sock.connect((HOST,PORT))
        with open(fname,"r") as f:
            buffer=list()
            for line in f:
                if line[0]=='%': continue
                vals=line.split(" ")
                vals=list(map(int,vals))
                if len(vals)==3 :
                    assert(vals[0]==vals[1]),"\nErrore la matrice non e'una matrice di adiacenza"
                    sock.sendall(struct.pack("!2i",vals[0],vals[2]))
                elif len(vals)==2:
                    if len(buffer)==10:
                        sock.sendall(struct.pack("!i",10))
                        sock.sendall(struct.pack("!10i",*buffer))
                        buffer.clear()
                    buffer.append(vals[0])
                    buffer.append(vals[1])
            if not len(buffer)==0:
                sock.sendall(struct.pack("!i",len(buffer)))
                sock.sendall(struct.pack(f"!{len(buffer)}i",*buffer))
                buffer.clear()
            else:
                sock.sendall(struct.pack("!i",0))
        r=sock.recv(4)
        return_code=struct.unpack("!i",r)[0]
        print(f"{fname} Exit code: {return_code}")


            
                


def send_data(fnames):
    threads=list()
    for el in fnames:
        t = threading.Thread(target=thread_job,args=(el,))
        t.start()
        threads.append(t)
    for th in threads:
        th.join()
    

assert(not len(sys.argv)==1),"\nErrore argomenti non validi"

send_data(sys.argv[1:])