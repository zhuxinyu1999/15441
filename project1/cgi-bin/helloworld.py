#!/usr/bin/env python3

msg = "hello world"

response = "HTTP/1.1 200 OK\r\n"
response += "Content-Length: " + str(len(msg)) + "\r\n"
response += "Content-type: text/html\r\n\r\n"
response += msg

print(response)