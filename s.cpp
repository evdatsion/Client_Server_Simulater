#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include"scheduler.h"
#include <iostream>
#include <string>
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>	 
#include <queue>
#include <unistd.h>
#include <fstream>
using namespace std;
int threadcount = 1;
int processcount = 1;
int newfd;
fstream indata;  
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex4 = PTHREAD_MUTEX_INITIALIZER;
bool taskcomplete=false;
queue<int> threadqueue;
char str[100];
bool cpu_first = 1;
bool io_first = 1;
Round_Robin RR;
SJF sjf;
Multilavel_Feedback MFB;
TimeSliceResponse tempresponse;
int Scheduling_type;
void * function (void * a );
void * function2 ( void * a);
pthread_t tid[1000];
pthread_attr_t attr;
int main(int argc, char * argv[] )
{
	indata.open ("file.csv", fstream::in | fstream::out | fstream::app);	

	indata<<"Process ID"<<","<<"Time Started"<<","<<"Resource type,Time Required,Burst Count,Turn Around Time"<<endl;
		
indata.close();

	struct sockaddr_un local,remote;	
	int socketfd,size_of_unixsock,length;	
	socketfd = socket(AF_UNIX,SOCK_STREAM,0);
	if(socketfd == -1)
	{
		perror("socket");
		exit(1);
	}

	char path[50]  = "temp/";
	int k , j = 5;	
	for(k = 0; k < strlen(argv[1]) ; k++)
	{
		path [j++] = argv[1][k];
	}
	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, path);
	unlink(local.sun_path);
	length = strlen(local.sun_path) + sizeof(local.sun_family);
	cout<<"Please Enter The Scheduling Scheme : \n";
	cout<<"\tPress 1 for SJF with Preamtion\n\tPress 2 for Round Robin\n\tPress 3 for Multi Level Feedback\n";
	cout<<"press Key : ";	
	cin>>Scheduling_type;
	if( Scheduling_type == 1)
	{
		sjf.Burst_type = 1;	
		sjf.scheduling_type = Scheduling_type; 
		sjf.burst_length = 10;
		sjf.ServerResponse.push_back(tempresponse);
	}
	else if( Scheduling_type == 2 )
	{
		RR.Burst_type = 1;
		RR.burst_length = 5;
		RR.scheduling_type = Scheduling_type ;
		RR.ServerResponse.push_back(tempresponse);
	} 
	else if( Scheduling_type == 3 )
	{
		MFB.Burst_type = 1;
		MFB.burst_length = 5;
		MFB.cpu_change_up = 1;
		MFB.io_change_up = 1;
		MFB.cpu_running_level = 1;
		MFB.io_running_level = 1;	
		MFB.scheduling_type = Scheduling_type ;
		MFB.ServerResponse.push_back(tempresponse);		
	}

	if(bind(socketfd, (struct sockaddr *)&local, length) == -1 )
	{
		perror("bind");
		exit(1);
	}

	if (listen(socketfd, 5) == -1)   
	{
		perror("listen");
		exit(1);
	}
	while(1) 
	{
		cout<<"Waiting for a connection."<<endl;
		size_of_unixsock = sizeof(remote);
		if ( (newfd = accept(socketfd, (struct sockaddr *)&remote,(socklen_t*)&size_of_unixsock)) == -1)
		{
			perror("accept");
			exit(1);
		}
		pthread_attr_init(&attr);
		if(pthread_create( &tid[threadcount], &attr, function2, &processcount))
			cout<<"Thread creation failed:"<<endl;

		else
			cout<<"thread created"<<endl;		  
		threadcount++;
		cout<<"Connected"<<endl;
	}
}

void * function2 ( void * a)
{
	int processfd = newfd;
	send ( processfd , &Scheduling_type , sizeof(int) , 0);  	
	int pid;
	int temp;	
	while(1)
	{
		recv(processfd, &temp, sizeof (int), 0);
		if(temp == 1 ) 
		{
			pthread_mutex_lock( &mutex2 );
			pid = processcount;
			processcount++;                  //  critical section
			pthread_mutex_unlock( &mutex2 );
			if( Scheduling_type == 1)
			{
				pthread_mutex_lock( &mutex3 );		
				sjf.ServerResponse.push_back(tempresponse);
				sjf.ServerResponse[pid].original_req.process_id = pid;
				recv(processfd, &sjf.ServerResponse[pid].original_req, sizeof(sjf.ServerResponse[pid].original_req), 0);
				sjf.Ready_State(pid);
				pthread_mutex_unlock( &mutex3 );

				time_t rawtime;
				struct tm * timeinfo;
				time ( &rawtime );
				timeinfo = localtime ( &rawtime );

				sjf.ServerResponse[pid].time_started = time(NULL);				
				send( processfd , &pid, sizeof( int ) , 0);		
				cout<<"Process ID : "<<pid<<"\n\tRequested for Time Slice : "<<sjf.ServerResponse[pid].original_req.time_required;
				cout<<"\n\tAt : "<<asctime (timeinfo) <<"\n";				

				if(pthread_create( &tid[threadcount], &attr, function, &pid ))
					cout<<"Thread creation failed:"<<endl;
				threadcount++;	
			}

			else if( Scheduling_type == 2)
			{
				pthread_mutex_lock( &mutex3 );		
				RR.ServerResponse.push_back(tempresponse);
				RR.ServerResponse[pid].original_req.process_id = pid;
				recv(processfd, &RR.ServerResponse[pid].original_req, sizeof(RR.ServerResponse[pid].original_req), 0);
				RR.Ready_State(pid);
				pthread_mutex_unlock( &mutex3 );

				time_t rawtime;
				struct tm * timeinfo;
				time ( &rawtime );
				timeinfo = localtime ( &rawtime );
				RR.ServerResponse[pid].time_started = time(NULL);				
				send( processfd , &pid, sizeof( int ) , 0);
				cout<<"Process ID : "<<pid<<"\n\tRequested for Time Slice : "<<RR.ServerResponse[pid].original_req.time_required;
				cout<<"\n\tAt : "<<asctime (timeinfo) <<"\n";								
				if(pthread_create( &tid[threadcount], &attr, function, &pid ))
				cout<<"Thread creation failed:"<<endl;
				threadcount++;	
			}
			else if( Scheduling_type == 3)
			{
				pthread_mutex_lock( &mutex3 );		
				MFB.ServerResponse.push_back(tempresponse);
				MFB.ServerResponse[pid].original_req.process_id = pid;
				recv(processfd, &MFB.ServerResponse[pid].original_req, sizeof(MFB.ServerResponse[pid].original_req), 0);
				MFB.Ready_State(pid);
				if( MFB.ServerResponse[pid].original_req.resource_type == 1)
				{
					if(cpu_first)
					{
					if ( MFB.ServerResponse[pid].original_req.priority ==  0 ) 
						MFB.cpu_running_level = 1;			
					else if ( MFB.ServerResponse[pid].original_req.priority ==  1 ) 
						MFB.cpu_running_level = 2;			
					else if ( MFB.ServerResponse[pid].original_req.priority ==  2 ) 
						MFB.cpu_running_level = 3;			
					else if ( MFB.ServerResponse[pid].original_req.priority ==  3 ) 
						MFB.cpu_running_level = 4;	
		
						if( MFB.I1.empty() && MFB.I2.empty() && MFB.I3.empty() && MFB.I4.empty() ) 							MFB.Set_Burst_Length();
						
						MFB.cpu_change_up =  MFB.cpu_running_level;
						cpu_first = false;	
					}	
				}
				else
				{
					if(io_first)
					{
					if ( MFB.ServerResponse[pid].original_req.priority ==  0 ) 
						MFB.io_running_level = 1;			
					else if ( MFB.ServerResponse[pid].original_req.priority ==  1 ) 
						MFB.io_running_level = 2;			
					else if ( MFB.ServerResponse[pid].original_req.priority ==  2 ) 
						MFB.io_running_level = 3;			
					else if ( MFB.ServerResponse[pid].original_req.priority ==  3 ) 
						MFB.io_running_level = 4;	
		
						if( MFB.C1.empty() && MFB.C2.empty() && MFB.C3.empty() && MFB.C4.empty() ) 							MFB.Set_Burst_Length();
						
						MFB.io_change_up =  MFB.io_running_level;
						io_first = false;	
					}	
				}
						
				pthread_mutex_unlock( &mutex3 );

				time_t rawtime;
				struct tm * timeinfo;
				time ( &rawtime );
				timeinfo = localtime ( &rawtime );
				MFB.ServerResponse[pid].time_started = time(NULL);				
				send( processfd , &pid, sizeof( int ) , 0);		
				cout<<"Process ID : "<<pid<<"\n\tRequested for Time Slice : "<<MFB.ServerResponse[pid].original_req.time_required;
				cout<<"\n\tAt : "<<asctime (timeinfo) <<"\n";							
				if(pthread_create( &tid[threadcount], &attr, function, &pid ))
					cout<<"Thread creation failed:"<<endl;
				threadcount++;	
			}
		}
	}

}
void* function ( void * a )
{
	int cpid = *( (int*) a);
	int burst_count=0;
	while(1)
	{
		if(Scheduling_type == 1)
		{
			if( sjf.Burst_type == 1 && sjf.ready.empty() )
			{
				bool temp = false;	
				while(sjf.ready.empty())
				{	
					sleep(1);
					sjf.burst_length--;
					if(sjf.burst_length == 0)
					{
						temp = true;
						break;
					}
				} 	
				if(temp == true)
				{
					sjf.Burst_type = 2;
					sjf.burst_length = 10;	
				}

			}						
			else if( sjf.Burst_type == 2 && sjf.IO.empty() )
			{
				bool temp = false;	
				while(sjf.IO.empty())
				{	
					sleep(1);
					sjf.burst_length--;
					if(sjf.burst_length == 0)
					{
						temp = true;
						break;
					}
				} 	
				if(temp == true)
				{
					sjf.Burst_type = 1;
					sjf.burst_length = 10;	
				}
			}
			if ( sjf.Process_ID() == cpid)
			{
				burst_count++;
				pthread_mutex_lock( &mutex1 );	
				if( sjf.Burst_type == 1)
					cout<<"\n ... CPU Burst ...\n";	
				else
					cout<<"\n ... I/O Burst ...\n";
				cout<<"\nProcess Id : "<<cpid<<"\n" ;
				cout<<"Time Slice Remaining :"<<sjf.ServerResponse[cpid].original_req.time_required<<endl;
				if( sjf.Burst_type == 1)
					cout<<"Process Aquire CPU Burst of : "<<sjf.burst_length<<"\n";
				else
					cout<<"Process Aquire I/O Burst of : "<<sjf.burst_length<<"\n";	
				while(1)
				{
					sleep(1);
					sjf.burst_length--;
					sjf.ServerResponse[cpid].original_req.time_required--;	
					if (sjf.ServerResponse[cpid].original_req.time_required == 0 )
					{
						cout<<"Process Time Slice Request Completed\n"<<endl;
						if(sjf.burst_length == 0)
						{	
							if( sjf.Burst_type == 1)
							{
								cout<<" ... CPU Burst Completed ...\n";	
								sjf.Burst_type = 2;					
							}						
							else if(sjf.Burst_type == 2)
							{
								cout<<" ... I/O Burst Completed ...\n";							
								sjf.Burst_type = 1;
							}
							sjf.burst_length = 5;	
						}
						sjf.Terminate_Process(cpid);	
						sjf.ServerResponse[cpid].ttl = time(NULL);	
indata.open ("file.csv", fstream::in | fstream::out | fstream::app);	

						indata<<cpid<<","<<sjf.ServerResponse[cpid].time_started<<","<<sjf.ServerResponse[cpid].original_req.resource_type<<",";
						indata<<sjf.ServerResponse[cpid].original_req.time_required<<","<<burst_count<<","
<<sjf.ServerResponse[cpid].ttl-sjf.ServerResponse[cpid].time_started<<endl;

indata.close();

						pthread_mutex_unlock( &mutex1 );	
						pthread_exit(NULL);									
					}

					if( sjf.Preamtion(cpid) )
					{
						cout<<"... Process Preempted ... \n";
						sjf.put_in_waiting(cpid);
						sjf.Waiting_State(cpid);
						break;	
					}			

					if( sjf.burst_length == 0)
					{
						sjf.put_in_waiting(cpid);
						sjf.Waiting_State(cpid);
						break;	
					}	
				}
				if( sjf.burst_length == 0)
				{
					if( sjf.Burst_type == 1)
					{
						cout<<" ... CPU Burst Completed ...\n";	
						sjf.Burst_type = 2;					
					}						
					else if(sjf.Burst_type == 2)
					{
						cout<<" ... I/O Burst Completed ...\n";							
						sjf.Burst_type = 1;
					}
					sjf.burst_length = 10;	
				}
				pthread_mutex_unlock( &mutex1 );	
			} // end of else if
		}		
		else if(Scheduling_type == 2)
		{
			if( RR.Burst_type == 1 && RR.ready.empty() )
			{
				bool temp = false;	
				while(RR.ready.empty())
				{	
					sleep(1);
					RR.burst_length--;
					if(RR.burst_length == 0)
					{
						temp = true;
						break;
					}
				} 	
				if(temp == true)
				{
					RR.Burst_type = 2;
					RR.burst_length = 5;	
				}

			}						
			else if( RR.Burst_type == 2 && RR.IO.empty() )
			{
				bool temp = false;	
				while(RR.IO.empty())
				{	
					sleep(1);
					RR.burst_length--;
					if(RR.burst_length == 0)
					{
						temp = true;
						break;
					}
				} 	
				if(temp == true)
				{
					RR.Burst_type = 1;
					RR.burst_length = 5;	
				}
			}
			if ( RR.Process_ID() == cpid)
			{
				burst_count++;
				pthread_mutex_lock( &mutex1 );	
				if( RR.Burst_type == 1)
					cout<<"\n ... CPU Burst ...\n";	
				else
					cout<<"\n ... I/O Burst ...\n";
				cout<<"\nProcess Id : "<<cpid<<"\n" ;
				cout<<"Time Slice Remaining :"<<RR.ServerResponse[cpid].original_req.time_required<<endl;
				if( RR.Burst_type == 1)
					cout<<"Process Aquire CPU Burst of : "<<RR.burst_length<<"\n";
				else
					cout<<"Process Aquire I/O Burst of : "<<RR.burst_length<<"\n";	
				while(1)
				{
					sleep(1);
					RR.burst_length--;
					RR.ServerResponse[cpid].original_req.time_required--;	
					if (RR.ServerResponse[cpid].original_req.time_required == 0 )
					{
						cout<<"Process Time Slice Request Completed\n"<<endl;
						if(RR.burst_length == 0)
						{	
							if( RR.Burst_type == 1)
							{
								cout<<" ... CPU Burst Completed ...\n";	
								RR.Burst_type = 2;					
							}						
							else if(RR.Burst_type == 2)
							{
								cout<<" ... I/O Burst Completed ...\n";							
								RR.Burst_type = 1;
							}
							RR.burst_length = 5;	
						}
						RR.Terminate_Process(cpid);	
						RR.ServerResponse[cpid].ttl = time(NULL);	
indata.open ("file.csv", fstream::in | fstream::out | fstream::app);	

						indata<<cpid<<","<<RR.ServerResponse[cpid].time_started<<","<<RR.ServerResponse[cpid].original_req.resource_type<<",";
						indata<<RR.ServerResponse[cpid].original_req.time_required<<","<<burst_count<<","
<<RR.ServerResponse[cpid].ttl-RR.ServerResponse[cpid].time_started<<endl;

indata.close();
						pthread_mutex_unlock( &mutex1 );	
						pthread_exit(NULL);									
					}
					if( RR.burst_length == 0)
					{
						RR.put_in_waiting(cpid);
						RR.Waiting_State(cpid);
						break;	
					}	
				}
				if( RR.burst_length == 0)
				{
					if( RR.Burst_type == 1)
					{
						cout<<" ... CPU Burst Completed ...\n";	
						RR.Burst_type = 2;					
					}						
					else if(RR.Burst_type == 2)
					{
						cout<<" ... I/O Burst Completed ...\n";							
						RR.Burst_type = 1;
					}
					RR.burst_length = 5;	
				}
				pthread_mutex_unlock( &mutex1 );	
			} // end of else if
		}

		else if(Scheduling_type == 3)
		{
			if( MFB.Burst_type == 1 && MFB.C1.empty() && MFB.C2.empty() && MFB.C3.empty() && MFB.C4.empty() )
			{
				bool temp = false;	
				while( MFB.C1.empty() && MFB.C2.empty() && MFB.C3.empty() && MFB.C4.empty() )
				{	
					sleep(1);
					MFB.burst_length--;
					if(MFB.burst_length <= 0)
					{
						temp = true;
						break;
					}
				} 	
				if(temp == true)
				{
					MFB.Burst_type = 2;
					MFB.Set_Burst_Length();	
				}

			}	
			else if( MFB.Burst_type == 2 && MFB.I1.empty() && MFB.I2.empty() && MFB.I3.empty() && MFB.I4.empty() )
			{
				bool temp = false;	
				while( MFB.I1.empty() && MFB.I2.empty() && MFB.I3.empty() && MFB.I4.empty() )
				{	
					sleep(1);
					MFB.burst_length--;
					if(MFB.burst_length <= 0)
					{
						temp = true;
						break;
					}
				} 	
				if(temp == true)
				{
					MFB.Burst_type = 1;
					MFB.Set_Burst_Length();	
				}
			}	
			 
			if ( MFB.Process_ID() == cpid )
			{
				burst_count++;
				pthread_mutex_lock( &mutex1 );	
				if( MFB.Burst_type == 1)
					cout<<"\n ... CPU Burst ...\n";	
				else
					cout<<"\n ... I/O Burst ...\n";
				cout<<"\nProcess Id : "<<cpid<<"\n" ;
		cout<<"Time Slice Remaining :"<<MFB.ServerResponse[cpid].original_req.time_required<<" Of Priority : "<<MFB.ServerResponse[cpid].original_req.priority<<endl;
				if( MFB.Burst_type == 1)
					{
					if(MFB.cpu_running_level == 4)
						cout<<"Process Aquire CPU Burst Untill End of Time Slice \n";
					else
					cout<<"Process Aquire CPU Burst of : "<<MFB.burst_length<<"\n";
					}
				else
					{
					if(MFB.io_running_level == 4)
						cout<<"Process Aquire I/O Burst Untill End of Time Slice \n";
					else					
					cout<<"Process Aquire I/O Burst of : "<<MFB.burst_length<<"\n";	
					}			
				while(1)
				{
					sleep(1);
					MFB.burst_length--;
					MFB.ServerResponse[cpid].original_req.time_required--;	
					if (MFB.ServerResponse[cpid].original_req.time_required == 0 )
					{
						cout<<"Process Time Slice Request Completed\n"<<endl;
						MFB.Terminate_Process(cpid);
						if(MFB.Burst_type == 1)
						{
							if(MFB.C1.empty() && MFB.C2.empty() && MFB.C3.empty() && MFB.C4.empty())
								cpu_first = true;
							else
							{
								MFB.Runing_Level();
								MFB.cpu_change_up = MFB.cpu_running_level; 
							}
						}
						else
						{
							if(MFB.I1.empty() && MFB.I2.empty() && MFB.I3.empty() && MFB.I4.empty())
								io_first = true;
							else
							{
								MFB.Runing_Level();
								MFB.io_change_up = MFB.io_running_level;
							}
						}
						if(MFB.burst_length <= 0)
						{	
							if( MFB.Burst_type == 1)
							{
								cout<<" ... CPU Burst Completed ...\n";		
								MFB.Burst_type = 2; 	
								MFB.Set_Burst_Length();	
											
							}						
							else if(MFB.Burst_type == 2)
							{
					cout<<" ... I/O Burst Completed ...\n";								
					MFB.Burst_type = 1;		
					MFB.Set_Burst_Length();
							}
							
						}
	
						MFB.ServerResponse[cpid].ttl = time(NULL);	
indata.open ("file.csv", fstream::in | fstream::out | fstream::app);	

						indata<<cpid<<","<<MFB.ServerResponse[cpid].time_started<<","<<MFB.ServerResponse[cpid].original_req.resource_type<<",";
						indata<<MFB.ServerResponse[cpid].original_req.time_required<<","<<burst_count<<","
<<MFB.ServerResponse[cpid].ttl-MFB.ServerResponse[cpid].time_started<<endl;

indata.close();

						pthread_mutex_unlock( &mutex1 );	
						pthread_exit(NULL);									
					}

					if( MFB.Preamtion(cpid) )
					{
						cout<<"... Process Preempted ... \n";
						MFB.put_in_waiting(cpid);
						
												
						if(MFB.Burst_type == 1)
						{
						
						if(MFB.cpu_running_level != MFB.cpu_change_up )
								{
								int t = MFB.cpu_running_level; 
								MFB.cpu_running_level = MFB.cpu_change_up;
								if( t == 4)
								MFB.Set_Burst_Length();		
								}
						}
						else
						{
						if(MFB.io_running_level != MFB.io_change_up )
								{
								int t = MFB.io_running_level; 									MFB.io_running_level = MFB.io_change_up;
								if( t == 4)
								MFB.Set_Burst_Length();
								}
						}
						break;	
					}			
					
					if( MFB.burst_length == 0)
					{
					MFB.put_in_waiting(cpid);
					MFB.Runing_Level();	
					if(MFB.Burst_type == 1)
				MFB.cpu_change_up = MFB.cpu_running_level;								
					else if(MFB.Burst_type == 2)
				MFB.io_change_up = MFB.io_running_level;								
						
					break;	
					}	
				}
				if( MFB.burst_length == 0)
				{
					if( MFB.Burst_type == 1)
					{
						cout<<" ... CPU Burst Completed ...\n";				
						MFB.Burst_type = 2;						
						MFB.Set_Burst_Length();					
					}						
					else if(MFB.Burst_type == 2)
					{
						cout<<" ... I/O Burst Completed ...\n";							
						MFB.Burst_type = 1;						
						MFB.Set_Burst_Length();	
					}				
				}
				pthread_mutex_unlock( &mutex1 );	
			} // end of else if
		}

	}// end of while
}


