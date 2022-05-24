#!/usr/bin/env python3

import socket

host = "192.168.52.144"
port = 8080

req = b"GET / HTTP/1.1\r\n"
req += b"Connection: keep-alive\r\n\r\n"

arr = []

n = 800

for i in range(10):
  for j in range(n):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))
    arr.append(sock)
  for j in range(n):
    arr[j].send(req)
  for j in range(n):
    msg = arr[j].recv(1024)
    print(str(msg, encoding="utf-8"))
  for j in range(n):
    arr[j].close()
  arr.clear()
