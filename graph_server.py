#!/usr/bin/env python3
import concurrent.futures,subprocess,tempfile
import sys,os,concurrent,socket,struct,threading,logging

HOST="127.0.0.1"
PORT= 56453

logging.basicConfig(filename='server.log',
                    level=logging.INFO, datefmt='%d/%m/%y %H:%M:%S',
                    format='%(asctime)s - %(levelname)s - %(message)s')


def conn_handling(conn,addr):
    with conn:
        logging.debug(f"Connesso con {addr}")
        data = conn.recv(8)
        nodi = struct.unpack("!i",data[:4])[0]
        edges = struct.unpack("!i",data[4:8])[0]
        logging.info(f"[{threading.get_ident()}] Numero nodi: {nodi}");
        assert(nodi >0 and edges>0), "\nErrore dati non validi..."
        with tempfile.NamedTemporaryFile(mode="a+",suffix=".mtx",) as temp:
            logging.info(f"[{threading.get_ident()}] File temporaneo: {temp.name}");
            temp.write(f"{nodi} {nodi} {edges}")
            not_valid=0
            valid =0
            write_buffer = list()
            while True:
                #mi faccio mandare la dimensione
                length = conn.recv(4)
                n_values = struct.unpack("!i",length)[0]
                if n_values==-1: break
                vals = conn.recv(n_values*4)
                v1,v2 = struct.unpack(f"!{n_values}i",vals)
                if v1>nodi or v2>nodi:
                    not_valid+=1
                    continue
                valid+=1
                if len(write_buffer)<10:
                    write_buffer.append((v1,v2))
                else:
                    for el in write_buffer:
                        temp.write(f"\n{el[0]} {el[1]}")
                    write_buffer.clear()
                    write_buffer.append((v1,v2))
            for el in write_buffer:
                temp.write(f"\n{el[0]} {el[1]}")
            write_buffer.clear()
            logging.info(f"[{threading.get_ident()}] Archi scartati: {not_valid}")
            logging.info(f"[{threading.get_ident()}] Archi validi: {valid}")
            temp.seek(0)
            ret = subprocess.run(f"./pagerank {temp.name}",capture_output=True,shell=True)
            logging.info(f"[{threading.get_ident()}] Subprocess Exit code: {ret.returncode}")
            conn.sendall(struct.pack("!i",ret.returncode))
            if(ret.returncode==0): res=ret.stdout
            else: res=ret.stderr
            conn.sendall(struct.pack("!i",len(res.decode())))
            conn.sendall(res)
            logging.debug("Comunicazione terminata... ")
        
        
            

def main():
    with socket.socket(socket.AF_INET,socket.SOCK_STREAM) as server:
        with concurrent.futures.ThreadPoolExecutor() as exe:
            try:
                server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                server.bind((HOST,PORT))
                server.listen()
                while True:
                    logging.debug("In attesa di connessione...")
                    conn, add = server.accept()
                    exe.submit(conn_handling,conn,add)
            except KeyboardInterrupt:
                pass
            exe.shutdown(wait=True)
        server.shutdown(socket.SHUT_RDWR)
        print("Bye dal server")


main()