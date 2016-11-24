

int hourCall(void)
{
	
	time_t timeCall;
    char timeBuffer[7];
    struct tm* tm_info;

    time(&timeCall);
    tm_info = localtime(&timeCall);

    strftime(timeBuffer, 7, "%H%M%S", tm_info);
    //puts(timeBuffer);	//writes string
	
	//rintf("%s\n",timeBuffer);	//print string
	
	int hr10 = 10*(timeBuffer[0] - '0');
	int hr01 = timeBuffer[1] - '0';
	int hr = hr10 + hr01;	//hr represents hour

	
	//printf("%i:%i\n", hr, mn);
	
	return hr;
}

int minCall(void)
{
	
	time_t timeCall;
    char timeBuffer[7];
    struct tm* tm_info;

    time(&timeCall);
    tm_info = localtime(&timeCall);

    strftime(timeBuffer, 7, "%H%M%S", tm_info);
    //puts(timeBuffer);	//writes string
	
	//rintf("%s\n",timeBuffer);	//print string
	

	int mn10 = 10*(timeBuffer[2] - '0');
	int mn01 = timeBuffer[3] - '0';
	int mn = mn10 + mn01;	//mn represents minute
	
	//printf("%i:%i\n", hr, mn);
	
	return mn;
}





