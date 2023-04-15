## Webserv
Webserv is a custom HTTP server implementation written in C/C++ that supports HTTP/1.1. The purpose of this project is to gain a better understanding of how web servers work and to be able to test it with an actual web browser. The project was completed with the help of a teammate, the one and only <a href="https://github.com/otmallah">Othman Mallah</a>
<br>

General Information
Webserv is a lightweight web server designed to handle HTTP/1.1 requests. It is built from the ground up to be fast, reliable, and scalable. The server is written in C++ and uses a modular architecture to support multiple server blocks, multiple location blocks, and multiple CGI scripts.

Webserv is intended to be used as a learning tool for anyone interested in web server development. It provides a great starting point for exploring the intricacies of HTTP/1.1, understanding how web servers handle requests and responses, and learning about the various technologies and protocols involved in web development.

Technologies Used
The following technologies were used in the development of Webserv:

- C++: Webserv is written in C++, a powerful and efficient programming language that provides low-level control over system resources.
- Makefile: Webserv is built using a Makefile, a build automation tool that compiles source code and links it into executables or libraries.
- HTTP/1.1: Webserv supports the HTTP/1.1 protocol, a widely-used standard for communication between web servers and clients.
- CGI: Webserv supports Common Gateway Interface (CGI) scripts, which allow external programs to be executed in response to HTTP requests.
- Socket Programming: Webserv uses socket programming to establish connections with clients and handle incoming requests.

Usage
To run the server, execute the following command:

<code>make run</code>

This will start the server on of the listed servers. You can then open your web browser and navigate to the address to interact with the server. Configuration files can be added or modified in the config file

Features
- Supports GET, POST, and DELETE methods
- Supports multiple CGI scripts (php, python)
- Supports multiple server blocks
- Supports multiple location blocks
- Supports multiple index files
- Supports multiple error pages
- Supports auto-indexing
- Supports URL encoding
- Supports client body size limit
- Supports redirecting
- Supports chunked transfer encoding
- Supports keep-alive connections
- And more...

Key Learnings
Building Webserv provided a great opportunity to learn about how web servers work, how they deal with web clients, and the underlying technologies that make up the web. This project helped me gain a deeper understanding of the HTTP protocol, socket programming, and CGI scripts. I also learned about the importance of scalability, reliability, and security in web server design.
