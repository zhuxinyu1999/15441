#! /usr/bin/env python3

a = int(input())
b = int(input())

msg = str(a) + " + " + str(b) + " = " + str(a + b)

response = "HTTP/1.1 200 OK\r\n"
response += "Content-Length: " + str(len(msg)) + "\r\n"
response += "Content-type: text/html\r\n\r\n"
response += msg

print(response)