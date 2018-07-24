enum Priority
	{
		VeryHigh,	
		High,
		Normal,
		Low
	}; 

struct TimeSliceRequest
{
	int process_id;
	int time_required;
	int resource_type;
	Priority priority;
};


struct TimeSliceResponse 
{

	TimeSliceRequest original_req;
	// Unix timestamp of when process was started on server

	unsigned long time_started;
	// Waiting and running time till end of CPU bust

	unsigned long ttl;
};

