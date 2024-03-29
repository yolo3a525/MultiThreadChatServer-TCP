﻿
#include "pch.h"
#include <winsock2.h>
#include <stdio.h>
#include <thread>
#include <vector>
#include <iostream>
#include <string>




DWORD WINAPI  ThreadProcB(LPVOID lpParam) {
	std::vector<SOCKET> *a = (std::vector<SOCKET> *)lpParam;
	std::string name;
	while (1) {
		std::getline(std::cin, name);
		int count = a->size();
		for (int i = 0; i < count; i++)
		{
			if (name.find(std::to_string(i)) == 0) {
				name.append("\r\n");
				send(a->at(i), name.data(), strlen(name.data()), 0);
				break;
			}
			else if(name.find("Q") == 0){
				closesocket(a->at(i));
				a->erase(a->begin() + i);
				break;
			}
		}
	}

	
	return 0;
}

DWORD WINAPI  ThreadProc(LPVOID lpParam) {
	SOCKET *socket = (SOCKET *)lpParam;
	char			recvbuff[1024];
	int				receiveByte;
	int k = 0;
	char sendbuf[1024] = "This is a test string from sender\r\n";
	int sendByte;
	bool flag = false;
	bool socketFlag = true;
	while (1) {
		k = 0;
		flag = false;
		while (1) {
			char			t[1024];
			receiveByte = recv(*socket, t, sizeof(t), 0);
			if (receiveByte > 0)
			{
				for (int i = 0; i < receiveByte; i++) {
					if (t[i] == '\n') {

						for (int ii = 0; ii < k; ii++)
							printf("%c", recvbuff[ii]);
						printf("\n");
						flag = true;
						break;
					}
					else {
						recvbuff[k++] = t[i];
					}
				}
				if (flag) {
					break;
				}

			}
			else if (receiveByte == 0) {
				socketFlag = false;
				printf("Server: Connection closed!\n");
				break;
			}

			else {
				socketFlag = false;
				printf("Server: recv() failed with error code:\n");
				break;
			}

		}
		//暂时屏蔽
		//sendByte = send(*socket, sendbuf, strlen(sendbuf), 0);
		if (!socketFlag) {
			break;
		}
	}

	return 0;
}

int main(void)
{
	WSADATA			wsaData;
	SOCKET			ListeningSocket;
	SOCKET*			NewConnection;
	std::vector<SOCKET> vec;
	SOCKADDR_IN		ServerAddr;
	int				Port = 6000;

	// Initialize Winsock version 2.2
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		// MSDN: An application can call the WSAGetLastError() function to determine the
		// extended error code for other Windows sockets functions as is normally
		// done in Windows Sockets even if the WSAStartup function fails or the WSAStartup
		// function was not called to properly initialize Windows Sockets before calling a
		// Windows Sockets function. The WSAGetLastError() function is one of the only functions
		// in the Winsock 2.2 DLL that can be called in the case of a WSAStartup failure
		printf("Server: WSAStartup failed with error %ld\n", WSAGetLastError());
		// Exit with error
		return -1;
	}
	else
	{
		printf("Server: The Winsock dll found!\n");
		printf("Server: The current status is %s.\n", wsaData.szSystemStatus);
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		//Tell the user that we could not find a usable WinSock DLL
		printf("Server: The dll do not support the Winsock version %u.%u!\n", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
		// Do the clean up
		WSACleanup();
		// and exit with error
		return -1;
	}
	else
	{
		printf("Server: The dll supports the Winsock version %u.%u!\n", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
		printf("Server: The highest version this dll can support is %u.%u\n", LOBYTE(wsaData.wHighVersion), HIBYTE(wsaData.wHighVersion));
	}

	// Create a new socket to listen for client connections.
	ListeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Check for errors to ensure that the socket is a valid socket.
	if (ListeningSocket == INVALID_SOCKET)
	{
		printf("Server: Error at socket(), error code: %ld\n", WSAGetLastError());
		// Clean up
		WSACleanup();
		// and exit with error
		return -1;
	}
	else
		printf("Server: socket() is OK!\n");

	// Set up a SOCKADDR_IN structure that will tell bind that we
	// want to listen for connections on all interfaces using port 5150.

	// The IPv4 family
	ServerAddr.sin_family = AF_INET;
	// host-to-network byte order
	ServerAddr.sin_port = htons(Port);
	// Listen on all interface, host-to-network byte order
	ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// Associate the address information with the socket using bind.
	// Call the bind function, passing the created socket and the sockaddr_in
	// structure as parameters. Check for general errors.
	if (bind(ListeningSocket, (SOCKADDR *)&ServerAddr, sizeof(ServerAddr)) == SOCKET_ERROR)
	{
		printf("Server: bind() failed! Error code: %ld.\n", WSAGetLastError());
		// Clos the socket
		closesocket(ListeningSocket);
		// Do the clean up
		WSACleanup();
		// and exit with error
		return -1;
	}
	else
		printf("Server: bind() is OK!\n");

	// Listen for client connections. We used a backlog of 5, which
	// is normal for many applications.
	if (listen(ListeningSocket, 5) == SOCKET_ERROR)
	{
		printf("Server: listen(): Error listening on socket %ld.\n", WSAGetLastError());
		// Close the socket
		closesocket(ListeningSocket);
		// Do the clean up
		WSACleanup();
		// Exit with error
		return -1;
	}
	else
		printf("Server: listen() is OK, I'm waiting for connections...\n");

	printf("Server: accept() is ready...\n");
	// Accept a new connection when one arrives.
	int a = 0;


	while (1)
	{
		printf("while start \n");
		NewConnection = (SOCKET*)malloc(sizeof(SOCKET));
		*NewConnection = accept(ListeningSocket, NULL, NULL);
		vec.push_back(*NewConnection);


		printf("Server: accept() is OK...\n");
		printf("Server: Client connected, ready for receiving and sending data...\n");
		//Transfer the connected socket to the listening socket
		//ListeningSocket = NewConnection;
		char welcome[1024] = "welcome\r\n";
		send(*NewConnection, welcome, strlen(welcome), 0);
		CreateThread(
			NULL,// default security attributes
			0,// use default stack size
			&ThreadProc,// thread function
			NewConnection,// argument to thread function
			0, // use default creation flags
			NULL);

		CreateThread(
			NULL,// default security attributes
			0,// use default stack size
			&ThreadProcB,// thread function
			&vec,// argument to thread function
			0, // use default creation flags
			NULL);

	}

	// At this point you can do two things with these sockets. Wait
	// for more connections by calling accept again on ListeningSocket
	// and start sending or receiving data on NewConnection (need a loop). We will
	// describe how to send and receive data later in the chapter.

	// When you are finished sending and receiving data on the
	// NewConnection socket and are finished accepting new connections
	// on ListeningSocket, you should close the sockets using the
	// closesocket API.
	if (closesocket(*NewConnection) != 0)
		printf("Server: Cannot close \"NewConnection\" socket. Error code: %ld\n", WSAGetLastError());
	else
		printf("Server: Closing \"NewConnection\" socket...\n");

	// When your application is finished handling the connections, 
	// call WSACleanup.
	if (WSACleanup() != 0)
		printf("Server: WSACleanup() failed! Error code: %ld\n", WSAGetLastError());
	else
		printf("Server: WSACleanup() is OK...\n");
	return 0;
}