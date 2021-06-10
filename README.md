# Flex HTTP Server
*WIP*
## Supported OS
    - Linux and Unix-like systems

## Compiling server
Make sure that your distribution supports pthread library. 
```bash
cmake .
make
```

## Starting server
```bash
./http_server <ip address> <port>
```
### Example
```bash
./http_server 192.168.1.45 8080
```