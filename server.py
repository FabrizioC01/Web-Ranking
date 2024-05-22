#!/usr/bin/env python3
import sys,os,threading,socket,struct

HOST="127.0.0.1"
PORT= 56453

def recv_all(conn,n):
  chunks = b''
  bytes_recd = 0
  while bytes_recd < n:
    # riceve blocchi di al più 1024 byte
    chunk = conn.recv(min(n - bytes_recd, 1024))
    if len(chunk) == 0:
      raise RuntimeError("socket connection broken")
    chunks += chunk
    bytes_recd = bytes_recd + len(chunk)
    assert bytes_recd == len(chunks)
  return chunks

def main():
    with socket.socket(socket.AF_INET,socket.SOCK_STREAM) as server:
        try:
            server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            server.bind((HOST,PORT))
            server.listen()
            while True:
                print("\nAttendo un client...")

                conn, addr = server.accept()
                
                data = recv_all(conn,12)

                assert (not len(data)==12),"\nErrore nei dati in arrivo"

                row = struct.unpack("!i",data[:4])[0]
                col = struct.unpack("!i",data[:4])[0]
                edges = struct.unpack("!i",data[:4])[0]

                print(f"\nRicevuti dati da remoto {row} | {col} | {edges}")
        except KeyboardInterrupt:
           pass
        print("\nBye da server")


main()