#!/usr/bin/env python3
import sys,os,threading,socket,struct,pickle

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
                    sock.sendall(struct.pack("!i",2))
                    sock.sendall(struct.pack("!2i",vals[0],vals[1]))
        sock.sendall(struct.pack("!i",-1))
        r=sock.recv(4)
        return_code=struct.unpack("!i",r)[0]
        print(f"{fname} Exit code: {return_code}")
        s = sock.recv(4)
        val = struct.unpack("!i",s)[0]
        str_val = sock.recv(val)
        formatted = str_val.decode().split("\n")
        for row in formatted:
            print(f"{fname} {row}")
        print(f"{fname} Bye")


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