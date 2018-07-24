#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include"structure.h"
#include <iostream>
#include <string>
#include <cstdio>
using namespace std;


int main(int argc, char * argv[] )
{
	struct sockaddr_un remote;
	int socketfd,length;	
	char str[100] = "zarnab";
	int n;

	TimeSliceRequest clientreq; 
	clientreq.process_id = 2;


	TimeSliceResponse client_accept ;
	socketfd = socket(AF_UNIX,SOCK_STREAM,0);

	if(socketfd == -1)
	{
		perror("socket");
		exit(1);
	}
	cout<<"  \n                     Trying to connect \n";

	char path[50]  = "temp/";
	int k , j = 5;	
	for(k = 0; k < strlen(argv[1]) ; k++)
	{
		path [j++] = argv[1][k];
	}
	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, path);

	length = strlen(remote.sun_path) + sizeof(remote.sun_family);

	if( connect(socketfd,(struct sockaddr *)&remote,length) == -1)
	{
		perror("connect");
		exit(1);
	} 
	cout<<"\nConnected\n";
	int stype; 
	recv(socketfd, &stype, sizeof(int), 0);
	if(stype == 1)
		cout<<"Keep In Mind :\n\t Your Server Doing SJF with burst of 10 Sec  \n";		
	else if(stype == 2)
		cout<<"Keep In Mind :\n\t Your Server Doing Round Robin with burst of 5 Sec  \n";		
	else
		cout<<"Keep In Mind :\n\t Your Server Multi Level Feedback with 4 level  \n";		
	int id;	
	char input;
	while(1)
	{	
		do{
		cout<<" Do you want to create a process ? (Y/N) \n";  
		cin>>input;
		  }
		while( input != 'y' &&  input != 'y' && input != 'n' && input != 'N'  )	;
		if( input == 'y' || input == 'Y' )
		{
			int a = 1;
			send( socketfd , &a , sizeof(int), 0);	
		}	
		else
		{
			int a = 0;
			send( socketfd , &a , sizeof(int), 0);	
		}
		if( input == 'y' || input == 'Y' )
		{
			do{	
			cout<<"Please enter your time slice required : ";  
			cin>>clientreq.time_required;}
			while( !(clientreq.time_required > 0 ));
			do{
			cout<<"Please enter Resource Type ? \n\tPress 1 for Cpu \n\tPress 2 for I/O \n ";  
			cout<<"Press Key : ";	
			cin>>clientreq.resource_type;}
			while(  clientreq.resource_type != 1 &&  clientreq.resource_type != 2 );	
			if(stype == 3)
			{	
				
				int p;
				cout<<"Server Using Multi Level Feed Back So,\n  ";
		do{				
cout<<"Please Enter Priority of your process\n\tPress 3 for very High \n\tPress 2 for High \n\tPress 1 for Normal\n\tPress 0 for low\n";
				cout<<"Press Key: ";
				cin>>p;}
			while( p != 0 &&  p != 1 &&  p != 2 &&  p != 3);	
				clientreq.priority = (Priority) p ; 
				 
			}	
			if (send(socketfd,&clientreq, sizeof(clientreq), 0) == -1) 
			{
				perror("send");
				exit(1);
			}
			recv(socketfd, &id, sizeof(int), 0);
			cout<<"\n\tProcess ID : "<<id;	
			cout<<"...... Time Slice Request Sent Waiting For Response ..... \n";  		
		}
		else 	
			break;
	}
cout<<" Clients Processes Requests Will completed soon \n";	
	//close(socketfd);
	return 0;
}


