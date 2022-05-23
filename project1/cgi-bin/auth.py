#! /usr/bin/env python3

# this script works for home.html

#name=xxxxx&password=xxxxxx

req = input()

ptr1 = req.find("=", 0, len(req))
ptr2 = req.find("=", ptr1 + 1, len(req))

name = req[ptr1 + 1 : ptr2 - 9]

password = req[ptr2 + 1 : len(req)]

msg = ""

if name == "zzxy" and password == "123456":
  msg = "<html><h1>success\n</h1></html>"
else:
  msg = "<html><h1>failed\n</h1></html>"

response = "HTTP/1.1 200 OK\r\n"
response += "Content-Length: " + str(len(msg)) + "\r\n"
response += "Content-type: text/html\r\n\r\n"
response += msg

print(response)
