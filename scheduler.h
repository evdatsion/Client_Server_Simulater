#include <vector>
#include"structure.h"
using namespace std;

class Scheduler
{
public:
	vector<TimeSliceResponse> ServerResponse;
	vector<int> ready,IO,waiting;
	int scheduling_type;		
};
class ST_Scheduler : public Scheduler
{
public:
	int Burst_type;

	virtual int Ready_State(int pid ) 
	{
		return 0;
	}
	virtual int Waiting_State(int pid ) 
	{
		return 0;
	}
	virtual int Process_ID ()
	{
		if(Burst_type == 1)
		{
			if(ready.empty()) return 0;
			return *ready.begin();
		}
		else
		{
			if(IO.empty()) return 0;
			return *IO.begin();
		}
	}

	virtual bool Preamtion( int pid)
	{
		if(Burst_type == 1)
		{
			if (ready.front() == pid)
				return false;
			return true;
		}
		else
		{
			if (IO.front() == pid)
				return false;
			return true;			}
	}
	virtual void put_in_waiting(int pid)
	{
		int i;	
		if (ServerResponse[pid].original_req.resource_type == 1)
		{
			for( i = 0 ; i < ready.size(); i++)
			{
				if(ready[i] == pid)
					break;
			}
			ready.erase(ready.begin() + i);
			waiting.push_back(pid);
		}
		else
		{
			for( i = 0 ; i < IO.size(); i++)
			{
				if(IO[i] == pid)
					break;
			}
			IO.erase(IO.begin() + i);
			waiting.push_back(pid);
		}
	}
	virtual	bool Check_Time_Slice(int pid)
	{
		if( ServerResponse[pid].original_req.time_required == 0) return true;
		return false;
	}
	virtual bool Terminate_Process(int pid)
	{
		int i;	
		if (ServerResponse[pid].original_req.resource_type == 1)
		{
			for( i = 0 ; i < ready.size(); i++)
			{
				if(ready[i] == pid)
					break;
			}
			ready.erase(ready.begin() + i);
		}
		else
		{
			for( i = 0 ; i < IO.size(); i++)
			{
				if(IO[i] == pid)
					break;
			}
			IO.erase(IO.begin() + i);
		}
		/*for( i = 0 ; i < ServerResponse.size(); i++)
		{
		if(ServerResponse[pid].original_req.process_id == pid)
		break;
		}
		ServerResponse.erase(ServerResponse.begin() + i);*/				
		return true;
	}

};
class SJF : public ST_Scheduler
{
public:
	int burst_length;
	int Ready_State(int pid )
	{
		vector<int>::iterator itr;
		bool less = false;

		if (ServerResponse[pid].original_req.resource_type == 1)	
		{	
			if(ready.empty())
				ready.push_back(pid);
			else
			{
				itr = ready.begin();
				for(; itr < ready.end(); itr++)
				{
					if( ServerResponse[*itr].original_req.time_required > ServerResponse[pid].original_req.time_required)
					{
						less = true;	
						break;
					}
				}
				if(less)
					ready.insert(itr,pid);
				else 
					ready.push_back(pid);
			}	
		}
		else
		{
			if(ServerResponse.empty())
				IO.push_back(pid);
			else
			{
				itr = IO.begin();
				for(; itr < IO.end(); itr++)
				{
					if( ServerResponse[*itr].original_req.time_required > ServerResponse[pid].original_req.time_required)
					{	
						less = true;
						break;
					}
				}
				if(less)
					IO.insert(itr,pid);
				else
					IO.push_back(pid);

			}
		}
		return 0;
	}

	int Waiting_State(int pid )
	{
		this->Ready_State(waiting.front());
		waiting.erase(waiting.begin());
		return 0;
	}
};
class Round_Robin : public ST_Scheduler
{
public:
	int burst_length;
	int Ready_State(int pid )
	{
		if (ServerResponse[pid].original_req.resource_type == 1)		
			ready.push_back(pid);
		else 
			IO.push_back(pid);
		return 0;
	}	
	int Waiting_State(int pid )
	{
		this->Ready_State(waiting.front());
		waiting.erase(waiting.begin());
		return 0;
	}

};
class Multilavel_Feedback : public ST_Scheduler
{

public:
	int cpu_running_level,io_running_level;
	int cpu_change_up,io_change_up;
	int burst_length;
	vector<int> C1,C2,C3,C4,I1,I2,I3,I4;

	void Runing_Level(void)
	{	
		if(Burst_type == 1)
		{
		if( !C1.empty() )
			cpu_running_level = 1;
		else if( !C2.empty() )
			cpu_running_level = 2;
		else if( !C3.empty() )
			cpu_running_level = 3;
		else if( !C4.empty() )
			cpu_running_level = 4;
		}
		else if(Burst_type == 2)
		{
		if( !I1.empty() )
			io_running_level = 1;
		else if( !I2.empty() )
			io_running_level = 2;
		else if( !I3.empty() )
			io_running_level = 3;
		else if( !I4.empty() )
			io_running_level = 4;
		}

	}	
	void Set_Burst_Length()
	{
	if(Burst_type == 2)
	{
		if( io_running_level == 1)
			burst_length = 5;
		else if(io_running_level == 2)
			burst_length = 10;
		else if(io_running_level == 3)
			burst_length = 10;
		else 
			burst_length = 0;
					
	}
	if(Burst_type == 1)
	{
		if(cpu_running_level == 1)
			burst_length = 5;
		else if( cpu_running_level == 2)
			burst_length = 10;
		else if(cpu_running_level == 3)
			burst_length = 10;	
		else 
			burst_length = 0;
	
	}

	}
	int Ready_State( int pid )
	{	
		if( ServerResponse[pid].original_req.priority  == 0)
		{	
			if( ServerResponse[pid].original_req.resource_type == 1)
				C1.push_back(pid);
			else
				I1.push_back(pid);
		}
		else if( ServerResponse[pid].original_req.priority  == 1 )
		{
			if( ServerResponse[pid].original_req.resource_type == 1)
				C2.push_back(pid);
			else
				I2.push_back(pid);
		}
		else if(  ServerResponse[pid].original_req.priority  == 2)
		{
			vector<int>::iterator itr;
			bool less = false;
			if (ServerResponse[pid].original_req.resource_type == 1)	
			{	
				if(C3.empty())
					C3.push_back(pid);
				else
				{
					itr = C3.begin();
					for(; itr < C3.end(); itr++)
					{
						if( ServerResponse[*itr].original_req.time_required > ServerResponse[pid].original_req.time_required)
						{
							less = true;	
							break;
						}
					}
					if(less)
						C3.insert(itr,pid);
					else 
						C3.push_back(pid);
				}	
			}
			else
			{
				if(ServerResponse.empty())
					I3.push_back(pid);
				else
				{
					itr = I3.begin();
					for(; itr < I3.end(); itr++)
					{
						if( ServerResponse[*itr].original_req.time_required > ServerResponse[pid].original_req.time_required)
						{	
							less = true;
							break;
						}
					}
					if(less)
						I3.insert(itr,pid);
					else
						I3.push_back(pid);
				}
			}
		}
		else
		{
			if( ServerResponse[pid].original_req.resource_type == 1)
				C4.push_back(pid);
			else
				I4.push_back(pid);
		}

	}	
	int Process_ID ()
	{
		if(Burst_type == 1)
		{
			if( cpu_running_level == 1 && !C1.empty() )
				return *C1.begin();
			else if( cpu_running_level == 2 && !C2.empty())			
				return *C2.begin();
			else if( cpu_running_level == 3 && !C3.empty())
				return *C3.begin();
			else if( cpu_running_level == 4 && !C4.empty())
				return *C4.begin();

		}
		else
		{
			if( io_running_level == 1 && !I1.empty())
				return *I1.begin();
			else if( io_running_level == 2 && !I2.empty())
				return *I2.begin();
			else if( io_running_level == 3 && !I3.empty())
				return *I3.begin();
			else if( io_running_level == 4 && !I4.empty())
				return *I4.begin();
		}
		return 0;
	}

	bool Preamtion( int pid)
	{
		if(Burst_type == 1)
		{
			if( cpu_running_level == 1)
			{
				if (C1.front() == pid)
					return false;
				return true;
			}
			else if( cpu_running_level == 2)
			{
				if (C1.empty())
				{
					if (C2.front() == pid)
						return false;
					return true;
				}
				else 
				{
					cpu_change_up = 1;
					return true;
				}			
			}
			else if( cpu_running_level == 3)
			{
				if (C1.empty())
				{
					if (C2.empty())
					{
						if(C3.front() == pid)
							return false;
						return true;
					}
					else
					{
						cpu_change_up = 2;
						return true;
					}
				}
				else 
				{
					cpu_change_up = 1;
					return true;
				}
			}
			else
			{
				if (C1.empty())
				{
					if (C2.empty())
					{
						if(C3.empty())
						{
							if(C4.front() == pid)
								return false;
							return true;
						}
						else
						{
							cpu_change_up = 3;
							return true;
						}
					}
					else
					{
						cpu_change_up = 2;
						return true;
					}
				}
				else 
				{
					cpu_change_up = 1;
					return true;
				}				
			}
		}
		else
		{
			if( io_running_level == 1)
			{
				if (I1.front() == pid)
					return false;
				return true;
			}
			else if( io_running_level == 2)
			{
				if (I1.empty())
				{
					if (I2.front() == pid)
						return false;
					return true;
				}
				else 
				{
					io_change_up = 1;
					return true;
				}			
			}
			else if( io_running_level == 3)
			{
				if (I1.empty())
				{
					if (I2.empty())
					{
						if(I3.front() == pid)
							return false;
						return true;
					}
					else
					{
						io_change_up = 2;
						return true;
					}
				}
				else 
				{
					io_change_up = 1;
					return true;
				}
			}
			else
			{
				if (I1.empty())
				{
					if (I2.empty())
					{
						if(I3.empty())
						{
							if(I4.front() == pid)
								return false;
							return true;
						}
						else
						{
							io_change_up = 3;
							return true;
						}
					}
					else
					{
						io_change_up = 2;
						return true;
					}
				}
				else 
				{
					io_change_up = 1;
					return true;
				}				
			}
		}
	}

	int Waiting_State(int pid )
	{
		
		return 0;
	}

	void put_in_waiting(int pid)
	{
		int i;

		if ( ServerResponse[pid].original_req.resource_type == 1 )
		{
			if(cpu_running_level == 1)
			{
				for( i = 0 ; i < C1.size(); i++)
				{
					if( C1[i] == pid )
						break;
				}
				C1.erase(C1.begin() + i);
				ServerResponse[pid].original_req.priority = (Priority) 1;
				C2.push_back(pid);
			}
			else if( cpu_running_level == 2)
			{
				for( i = 0 ; i < C2.size(); i++)
				{
					if( C2[i] == pid )
						break;
				}
				C2.erase(C2.begin() + i);
				ServerResponse[pid].original_req.priority = (Priority)2;
				this->Ready_State(pid);
			}
			else if( cpu_running_level == 3)
			{
				for( i = 0 ; i < C3.size(); i++)
				{
					if( C3[i] == pid )
						break;
				}
				C3.erase(C3.begin() + i);
				ServerResponse[pid].original_req.priority = (Priority)3;
				C4.push_back(pid);
			}
			else
			{
				for( i = 0 ; i < C4.size(); i++)
				{
					if( C4[i] == pid )
						break;
				}
				C4.erase(C4.begin() + i);
				ServerResponse[pid].original_req.priority = (Priority)3;
				C4.push_back(pid);

			}
		}
		else
		{
			if(io_running_level == 1)
			{
				for( i = 0 ; i < I1.size(); i++)
				{
					if( I1[i] == pid )
						break;
				}
				I1.erase(I1.begin() + i);
				ServerResponse[pid].original_req.priority == 1;
				I2.push_back(pid);
			}
			else if( io_running_level == 2)
			{
				for( i = 0 ; i < I2.size(); i++)
				{
					if( I2[i] == pid )
						break;
				}
				I2.erase(I2.begin() + i);
				ServerResponse[pid].original_req.priority == 2;
				this->Ready_State(pid);
			}
			else if( io_running_level == 3)
			{
				for( i = 0 ; i < I3.size(); i++)
				{
					if( I3[i] == pid )
						break;
				}
				I3.erase(I3.begin() + i);
				ServerResponse[pid].original_req.priority == 3;
				I4.push_back(pid);
			}
			else
			{
				for( i = 0 ; i < I4.size(); i++)
				{
					if( I4[i] == pid )
						break;
				}
				I4.erase(I4.begin() + i);
				ServerResponse[pid].original_req.priority == 3;
				I4.push_back(pid);
			}
		}	
	}

	bool Terminate_Process(int pid)
	{
		int i;	
		if (ServerResponse[pid].original_req.resource_type == 1)
		{
			if( cpu_running_level == 1)
			{
				for( i = 0 ; i < C1.size(); i++)
				{
					if(C1[i] == pid)
						break;
				}
				C1.erase(C1.begin() + i);
			}
			else if( cpu_running_level == 2 )
			{
				for( i = 0 ; i < C2.size(); i++)
				{
					if(C2[i] == pid)
						break;
				}
				C2.erase(C2.begin() + i);
			}
			else if( cpu_running_level == 3 )
			{
				for( i = 0 ; i < C3.size(); i++)
				{
					if(C3[i] == pid)
						break;
				}
				C3.erase(C3.begin() + i);
			}
			else if( cpu_running_level == 4 )
			{
				for( i = 0 ; i < C4.size(); i++)
				{
					if(C4[i] == pid)
						break;
				}
				C4.erase(C4.begin() + i);
			}
		}
		else
		{
			if( io_running_level == 1)
			{
				for( i = 0 ; i < I1.size(); i++)
				{
					if(I1[i] == pid)
						break;
				}
				I1.erase(I1.begin() + i);
			}
			else if( io_running_level == 2 )
			{
				for( i = 0 ; i < I2.size(); i++)
				{
					if(I2[i] == pid)
						break;
				}
				I2.erase(I2.begin() + i);
			}
			else if( io_running_level == 3 )
			{
				for( i = 0 ; i < I3.size(); i++)
				{
					if(I3[i] == pid)
						break;
				}
				I3.erase(I3.begin() + i);
			}
			else if( io_running_level == 4 )
			{
				for( i = 0 ; i < I4.size(); i++)
				{
					if(I4[i] == pid)
						break;
				}
				I4.erase(I4.begin() + i);
			}
		}
		return true;
	}
};


