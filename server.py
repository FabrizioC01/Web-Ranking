#!/usr/bin/env python3
import concurrent.futures,subprocess,tempfile
import sys,os,concurrent,socket,struct,threading,logging

HOST="127.0.0.1"
PORT= 56453

logging.basicConfig(filename=os.path.basename(sys.argv[0])[:-3] + '.log',
                    level=logging.DEBUG, datefmt='%d/%m/%y %H:%M:%S',
                    format='%(asctime)s - %(levelname)s - %(message)s')

logging.info("------------------------------------------------------------------------")

def conn_handling(conn,addr):
    with conn:
        logging.debug(f"Connesso con {addr}")
        data = conn.recv(8)
        nodi = struct.unpack("!i",data[:4])[0]
        edges = struct.unpack("!i",data[4:8])[0]
        logging.info(f"[{threading.get_ident()}] Numero nodi: {nodi}");
        assert(nodi >0 and edges>0), "\nErrore dati non validi..."
        temp=tempfile.NamedTemporaryFile(mode="a",suffix=".mtx")
        logging.info(f"[{threading.get_ident()}] File temporaneo: {temp.name}");
        temp.write(f"{nodi} {nodi} {edges}")
        loop= True
        while loop:
            #mi faccio mandare la dimensione del buffer
            length = conn.recv(4)
            n_values = struct.unpack("!i",length)[0]
            if n_values!=10 : loop= False
            if n_values==0 : break
            logging.debug(f"[{threading.get_ident()}] Ricezione di {n_values} -> {int(n_values/2)} coppie")
            buff = conn.recv(n_values*4)
            inizio = 0
            fine = 8
            not_valid=0
            for i in range(int(n_values/2)):
                l = struct.unpack("!i",buff[inizio:inizio+4])[0]
                r = struct.unpack("!i",buff[inizio+4:fine])[0]
                if not 0<l<n_values or not 0<r<n_values:
                    not_valid=not_valid+1
                    continue
                inizio=fine
                fine=fine+8
                temp.write(f"\n{l} {r}")
        logging.debug("Comunicazione dati terminata...")
        ret = subprocess.run(["./res",temp.name],capture_output=True)
        temp.close()
        conn.sendall(struct.pack("!i",ret.returncode))
        
        
            

def main():
    with socket.socket(socket.AF_INET,socket.SOCK_STREAM) as server:
        threads =list()
        try:
            server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            server.bind((HOST,PORT))
            server.listen()
            while True:
                logging.debug("In attesa di connessione...")
                conn, add = server.accept()
                th = threading.Thread(target=conn_handling,args=(conn,add))
                th.start()
                threads.append(th)
        except KeyboardInterrupt:
            for t in threads:
                t.join()
        server.shutdown(socket.SHUT_RDWR)
        print("\nBye da server")


main()