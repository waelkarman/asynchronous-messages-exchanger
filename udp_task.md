## Introduction

This is a simple and loosely defined technical interview task. 
We would like to discuss how to create a project of two docker containers, UDP Client and UDP Server.

## Description
Required functionality:
+ The client and the server should be two separate processes running in two separate Docker containers.
+ The client and the server should periodically (e.g. 0.01Hz) exchange some arbitrary data.
+ The client and the server should be both able to detect communication failures: repeated data, lost data, data out of order.

## Instructions
+ 1. We would like you to set-up an empty git repository and create commits as you feel comfortable.
+ 2. We would like you to create a CMake project containing both executables for the server and the client, written in C/C++, standard of your choosing. 

We would prefer the project being as clean of dependencies and simple as possible.

+ The project should compile either in a container of your choosing and on a standard base Debian system.

+ 3. List/Reference/Comment any information you found useful in designing/programming this project.

Please send the project including .git repository 1 day before we meet in the technical interview.
In the interview, we would like to discuss the reasoning, techniques and thought process behind your solution. 
If you have any questions, please reach out.

