#!/usr/bin/env python3
import socket

host = "192.168.52.144"
port = 8080

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((host, port))

msg = b"POST /home/zzxy/Documents/15441/project1/cgi-bin/helloworld.py HTTP/1.1\r\n\r\n"
sock.send(msg)

response = sock.recv(1024)
print(str(response, encoding="utf-8"))

sock.close()