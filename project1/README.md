This is a HTTP/1.1 Web Server, supports CGI and HTTPS with TLS1.2.

# Build
```bash
cd src
make

./server <HTTP port> <HTTPS port> <log file> <lock file> 
         <www folder> <CGI script path> <private key file> <certificate file>
```

# Files
    cert:     Certificate file and private key file
    cgi-bin:  Cgi scripts
    src:      Source code
    static:   Simple web page for test
    test:     Test scripts
