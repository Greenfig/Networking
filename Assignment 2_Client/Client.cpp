#define _CRT_SECURE_NO_DEPRECATE
#include <WinSock2.h>
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <stdio.h>
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

	struct sockaddr_in Svr_Addr;
	int my_address = 127, Connected_to_Address=0;
	SOCKET ClientSocket, WelcomeSocket, ConnectionSocket;
	int PortNumber = 65432;
	char *IPAddress = "127.0.0.1";
	NSP_Packet RxDevice_File = { NULL, NULL, NULL, NULL, NULL, NULL, NULL }, TxDevice_File = { NULL, NULL, NULL, NULL, NULL, NULL, NULL }, TraxDevice_File = { NULL, NULL, NULL, NULL, NULL, NULL, NULL };
	fstream DF;
	stringstream ss;
	char* Des = new char[sizeof(NSP_Packet)];

	//Connecto to the server
	ClientSocket = socket(AF_INET, SOCK_STREAM, 0);

	Svr_Addr.sin_family = AF_INET;
	Svr_Addr.sin_addr.s_addr = inet_addr(IPAddress);
	Svr_Addr.sin_port = htons(PortNumber);

	//CONNECT
	if (connect(ClientSocket, (struct sockaddr*)&Svr_Addr, sizeof(Svr_Addr)) != 0)
	{
		cout << "Failed to connect" << endl;
		WSACleanup();
		system("PAUSE");
		return 0;

	}

	//Send My NSP address to connected machine
	send(ClientSocket, (char*)&my_address, sizeof(my_address), 0);

	//Receive connected to address
	recv(ClientSocket, (char*)&Connected_to_Address, sizeof(int), 0);

	//Open file
	DF.open("Device_Data.txt");

	if (!DF.is_open())
		cout << "Cannot open file." << endl;
	
	//Read RAW data from file
	
	while (!DF.eof())
	{
		char temp2[256];
		char temp[3];
		int j = 0;
		DF.getline(Des, 256);
		//create packet
		TxDevice_File.Source_Address= (unsigned char)my_address;
		
		TxDevice_File.Destination_Address=(unsigned char)Connected_to_Address;

		for (int i = 0; Des[i] != '\0'; i++){
			if (i<2){
				temp[i] = Des[i];
				temp[i + 1] = '\0';
			}
			if (i>2 && i<4)
				TxDevice_File.Packet_Correlation = (unsigned int)Des[i];
			if (i>4 && i<6)
				TxDevice_File.Reply = (unsigned int)Des[i];
			if (i>6 && i<8)
				TxDevice_File.ACK = (unsigned int)Des[i];
			if (i>8 && Des[i] != DF.eof())
				temp2[j++] = Des[i];
			
		}
		TxDevice_File.DataField = (unsigned char*)malloc(sizeof(temp2));
		strcpy((char*)TxDevice_File.DataField,(char*)temp2);

		TxDevice_File.Command = (unsigned int)temp;
		//Transmit data
		TraxDevice_File = TxDevice_File;
		send(ClientSocket, (const char*)&TraxDevice_File, sizeof(TraxDevice_File), 0);
		//Receive acknowledgement
		recv(ClientSocket, (char*)&RxDevice_File, sizeof(RxDevice_File), 0);
		if (RxDevice_File.ACK == 1 && RxDevice_File.DataField == TxDevice_File.DataField){
			RxDevice_File.Reply = 1;
			TxDevice_File = RxDevice_File;
			//Reply
			send(ClientSocket, (const char*)&TxDevice_File, sizeof(TxDevice_File), 0);
		}
		if (RxDevice_File.ACK != 1 && RxDevice_File.DataField != TxDevice_File.DataField){
			//Resend packet
			send(ClientSocket, (const char*)&TxDevice_File, sizeof(TxDevice_File), 0);
		}
	}

	DF.close();
	closesocket(ClientSocket);
	WSACleanup();
	return NULL;
}