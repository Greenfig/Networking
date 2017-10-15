#define _CRT_SECURE_NO_DEPRECATE
#include <WinSock2.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#define MAX 256
using namespace std;


typedef struct{
	//////////////
	////HEADER////
	////24bits////
	//////////////
	unsigned int Command : 5;				//5 bit
	unsigned int Packet_Correlation : 1;	//1 bit
	unsigned int Reply : 1;					//1 bit
	unsigned int ACK : 1;					//1 bit
	unsigned _int8 Destination_Address, Source_Address;		//8 bits + 8 bits
	//////////////
	/////DATA/////
	//0-25048bits/
	//////////////
	unsigned char* DataField;

}NSP_Packet;


int main(){


	//INI WINSOCK
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(2, 2);

	if (int stat = WSAStartup(wVersionRequested, &wsaData) != 0)
		return stat;
	

	struct sockaddr_in SvrAddr;
	int my_address = 145, Connected_to_Address = 0;
	SOCKET WelcomeSocket, ConnectionSocket;
	int PortNumber = 65432;
	NSP_Packet RxDevice_File = { NULL, NULL, NULL, NULL, NULL, NULL, NULL }, TxDevice_File = { NULL, NULL, NULL, NULL, NULL, NULL, NULL };
	fstream _file;
	string Message;
	int last_value = 0;

	//Open output file file
	_file.open("SaveData.txt");

	WelcomeSocket = socket(AF_INET, SOCK_STREAM, 0);

	SvrAddr.sin_family = AF_INET;
	SvrAddr.sin_addr.s_addr = INADDR_ANY;
	SvrAddr.sin_port = htons(PortNumber);

	bind(WelcomeSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
	listen(WelcomeSocket, 1);

	//Listen for Clients
	while (1)
	{
		ConnectionSocket = accept(WelcomeSocket, NULL, NULL);

		//Reveice Connected to's address
		recv(ConnectionSocket, (char*)&Connected_to_Address, sizeof(int), 0);
		//Send Server address
		send(ConnectionSocket, (const char*)&my_address, sizeof(my_address), 0);

		//Recieve NSP Packet
		recv(ConnectionSocket, (char*)&RxDevice_File, sizeof(RxDevice_File), 0);

		//Processing	
		//Check that the source and destination addresses match
		if ((int)RxDevice_File.Destination_Address==my_address && (int)RxDevice_File.Source_Address==Connected_to_Address){
			RxDevice_File.ACK = 1;
			TxDevice_File = RxDevice_File;
			send(ConnectionSocket, (const char*)&TxDevice_File, sizeof(TxDevice_File), 0);
			recv(ConnectionSocket, (char*)&RxDevice_File, sizeof(RxDevice_File), 0);
			if (RxDevice_File.Reply == 1 && last_value == RxDevice_File.Packet_Correlation){
				Message = Message + (const char*)RxDevice_File.DataField;
			}
			if (RxDevice_File.Reply == 1 && last_value == 0){

			}
			
		}
		//Save data to file
		char *s = (char*)RxDevice_File.DataField;

		_file.write(s, sizeof(RxDevice_File.DataField));
	}

	system("PAUSE");
	_file.close();
	closesocket(ConnectionSocket);
	WSACleanup();
	return NULL;
}