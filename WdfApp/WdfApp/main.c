#include<stdio.h>
#include<Windows.h>
#include<winioctl.h>
#include<Windows.h>
#include"wdf_ioctl.h"


int main()
{
	HANDLE wdfFltr = INVALID_HANDLE_VALUE;
	INPUT_BUFFER inputBuffer;
	INPUT_BUFFER OutputBuffer;
	NTSTATUS ntStatus;
	BOOL ret;
	ULONG bytesReturn;

	inputBuffer.data = 5;
	wdfFltr = CreateFile(L"\\\\.\\FltrMyDriver", GENERIC_WRITE|GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (wdfFltr == INVALID_HANDLE_VALUE)
	{
		printf("\nUnable to open device\n");
		return 0;
	}

	inputBuffer.data = 4;

	printf("\nDevice opened\n");

	
	ret = DeviceIoControl(wdfFltr, IOCTL_WRITE, NULL, NULL, &OutputBuffer, sizeof(INPUT_BUFFER), &bytesReturn, (LPOVERLAPPED)NULL);
	if (ret)
	{
		printf("success : %d", OutputBuffer.data);
	}
	else
	{
		printf("Failed : %d", GetLastError());
	}

	/*ret = DeviceIoControl(wdfFltr, IOCTL_READ, &inputBuffer, sizeof(inputBuffer),&ntStatus , sizeof(ntStatus), &bytesReturn, (LPOVERLAPPED)NULL);
	if (ret)
	{
		printf("success");
	}
	else
	{
		printf("Failed : %d",GetLastError());
	}*/

	

	CloseHandle(wdfFltr);

	return 0;
}