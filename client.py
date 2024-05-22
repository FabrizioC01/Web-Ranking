#!/usr/bin/env python3
import sys,os,threading,socket,struct

HOST="127.0.0.1"
PORT= 56453

def thread_fun(fname):
    with socket.socket(socket.AF_INET,socket.SOCK_STREAM) as s:
        s.connect((HOST,PORT))
        with open(fname,"r") as file:
            for line in file:
                if line[0]=='%': continue
                try:
                    row, col , edges = map(int, line.split())
                except ValueError:
                    print("\nFormato dei dati del file non valido...")
                    sys.exit(1)
                break
            assert(row==col and row>0 and col>0 and edges>0), "\nIl file passato non contiene una matrice di adiacenza"
            print(f"\nTrovati {row} {col} {edges}")
            s.sendall(struct.pack("!3i",row,col,edges))
            print(f"\nDati inviati al server {HOST}")
        s.shutdown(socket.SHUT_RDWR)
                    





def main():
    num=len(sys.argv)-1
    threads=list()
    for i in range(num):
        t = threading.Thread(target=thread_fun,args=(sys.argv[i+1],))
        threads.append(t)
        t.start()
    for th in threads:
        th.join()

if len(sys.argv)<2:
    print("\nErrore numero argomenti non valido...")
    sys.exit(1)
else:
    for i in range(len(sys.argv)):
        if not sys.argv[i].endswith(".mtx") and i!=0:
            print("\nErrore formato argomenti non valido...")
            sys.exit(1)
    main()