#include<ntddk.h>
#include<wdf.h>
#include"ioctl.h"



NTSTATUS AddDevice(WDFDRIVER driver, PWDFDEVICE_INIT pDeviceInit);
VOID DeviceIoControl(WDFQUEUE queue, WDFREQUEST request, size_t outputBufferLength, size_t inputBufferLength, ULONG IoControlCode);
VOID Write(WDFQUEUE queue, WDFREQUEST request, size_t length);
NTSTATUS CreateControlDevice(WDFDRIVER driver);
VOID CleanUp(WDFOBJECT device);


NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegisteryPath)
{
	WDF_DRIVER_CONFIG config;
	NTSTATUS ntStatus;
	WDFDRIVER driver;

	DbgPrint("\n%s : Enter\n", __FUNCTION__);

	WDF_DRIVER_CONFIG_INIT(&config, AddDevice);

	ntStatus = WdfDriverCreate(pDriverObject,
		pRegisteryPath,
		WDF_NO_OBJECT_ATTRIBUTES,
		&config, &driver);

	if (!NT_SUCCESS(ntStatus))
	{
		DbgPrint("\n%s : driver create failed\n", __FUNCTION__);
		return ntStatus;
	}

	ntStatus = CreateControlDevice(driver);
	if (!NT_SUCCESS(ntStatus))
	{
		DbgPrint("\n%s : failed create control device\n", __FUNCTION__);
		ntStatus = STATUS_SUCCESS;
	}

	DbgPrint("\n%s Exit\n", __FUNCTION__);
	return STATUS_SUCCESS;
}

NTSTATUS AddDevice(WDFDRIVER driver, PWDFDEVICE_INIT pDeviceInit)
{
	NTSTATUS ntStatus;
	WDF_OBJECT_ATTRIBUTES wdfObjectAttributes;
	WDFDEVICE wdfDevice;
	WDF_IO_QUEUE_CONFIG queueConfig;

	PAGED_CODE();
	UNREFERENCED_PARAMETER(driver);
	UNREFERENCED_PARAMETER(queueConfig);
	UNREFERENCED_PARAMETER(wdfObjectAttributes);

	DbgPrint("\n%s : Enter\n", __FUNCTION__);

	WdfFdoInitSetFilter(pDeviceInit);

	ntStatus = WdfDeviceCreate(&pDeviceInit, WDF_NO_OBJECT_ATTRIBUTES, &wdfDevice);
	if (!NT_SUCCESS(ntStatus))
	{
		DbgPrint("\n%s : failed (Wdf device create)\n", __FUNCTION__);
		return ntStatus;
	}

	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig, WdfIoQueueDispatchParallel);
	queueConfig.EvtIoWrite = Write;

	ntStatus = WdfIoQueueCreate(wdfDevice, &queueConfig, WDF_NO_OBJECT_ATTRIBUTES, NULL);
	if (!NT_SUCCESS(ntStatus))
	{
		DbgPrint("\n%s : failed (wdf io queue create)\n", __FUNCTION__);
		return ntStatus;
	}


	DbgPrint("\n%s : Exit\n", __FUNCTION__);
	return STATUS_SUCCESS;
}


VOID Write(WDFQUEUE queue, WDFREQUEST request, size_t length)
{
	WDF_REQUEST_SEND_OPTIONS options;
	WDFDEVICE device;
	WDFIOTARGET wdfIoTarget;

	UNREFERENCED_PARAMETER(length);
	UNREFERENCED_PARAMETER(queue);

	//DbgPrint("\n%s : Enter\n", __FUNCTION__);
	device = WdfIoQueueGetDevice(queue);
	wdfIoTarget = WdfDeviceGetIoTarget(device);

	WDF_REQUEST_SEND_OPTIONS_INIT(&options, WDF_REQUEST_SEND_OPTION_SEND_AND_FORGET);
	if (!WdfRequestSend(request, wdfIoTarget, &options))
	{

		DbgPrint("\n%s : failed request send\n", __FUNCTION__);
	}

	//DbgPrint("\n%s : Exit\n", __FUNCTION__);
	return;
}

VOID DeviceIoControl(WDFQUEUE queue, WDFREQUEST request, size_t outputBufferLength, size_t inputBufferLength, ULONG IoControlCode)
{
	NTSTATUS ntStatus = STATUS_NOT_SUPPORTED;
	PINPUT_BUFFER pInputBuffer;// = NULL;
	PINPUT_BUFFER pOutputBuffer = NULL;
	ULONG64 pBufSize;
	UNREFERENCED_PARAMETER(queue);
	UNREFERENCED_PARAMETER(outputBufferLength);
	UNREFERENCED_PARAMETER(inputBufferLength);

	PAGED_CODE();

	DbgPrint("\n%s : Enter\n", __FUNCTION__);

	switch (IoControlCode)
	{
	case IOCTL_READ:
		DbgPrint("\n%s : IOCTL_READ\n", __FUNCTION__);
		ntStatus = WdfRequestRetrieveInputBuffer(request, (size_t)sizeof(INPUT_BUFFER), (PVOID*)&pInputBuffer, &pBufSize);
		if (!NT_SUCCESS(ntStatus))
		{
			DbgPrint("\n%s : request retrive input buffer\n", __FUNCTION__);
			WdfRequestComplete(request, STATUS_INVALID_PARAMETER_1);
			return;
		}
		DbgPrint("\n%s : Value %d\n", __FUNCTION__, pInputBuffer->data);
		break;
	case IOCTL_WRITE:
		DbgPrint("\n%s : IOCTL_WRITE\n", __FUNCTION__);

		ntStatus = WdfRequestRetrieveOutputBuffer(request, (size_t)sizeof(INPUT_BUFFER), &pOutputBuffer, &pBufSize);

		if (!NT_SUCCESS(ntStatus))
		{
			DbgPrint("\n%s : request retrive output buffer\n", __FUNCTION__);
			WdfRequestComplete(request, STATUS_INVALID_PARAMETER_1);
			return;
		}

		pOutputBuffer->data = 101;
		break;
	default:
		DbgPrint("\n%s : default io control\n", __FUNCTION__);
		ntStatus = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	WdfRequestCompleteWithInformation(request, ntStatus, (size_t)sizeof(INPUT_BUFFER));
	DbgPrint("\n%s : Exit\n", __FUNCTION__);

}


NTSTATUS CreateControlDevice(WDFDRIVER driver)
{
	PWDFDEVICE_INIT pDeviceInit = NULL;
	WDF_IO_QUEUE_CONFIG queueConfig;
	NTSTATUS ntStatus;
	WDFDEVICE controlDevice;

	DECLARE_CONST_UNICODE_STRING(dosName, L"\\DosDevices\\FltrMyDriver");
	DECLARE_CONST_UNICODE_STRING(devName, L"\\Device\\FltrMyDriver");

	PAGED_CODE();

	DbgPrint("\n%s : Entry\n", __FUNCTION__);

	pDeviceInit = WdfControlDeviceInitAllocate(driver, &SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R);
	if (pDeviceInit == NULL)
	{
		DbgPrint("\n%s : device init allocate failed\n", __FUNCTION__);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	WdfDeviceInitSetExclusive(pDeviceInit, FALSE);

	ntStatus = WdfDeviceInitAssignName(pDeviceInit, &devName);
	if (!NT_SUCCESS(ntStatus))
	{
		DbgPrint("\n%s : device init assign name failed\n", __FUNCTION__);
		return ntStatus;
	}



	ntStatus = WdfDeviceCreate(&pDeviceInit, WDF_NO_OBJECT_ATTRIBUTES, &controlDevice);
	if (!NT_SUCCESS(ntStatus))
	{
		DbgPrint("\n%s : wdf device create failed\n", __FUNCTION__);
		return ntStatus;
	}

	ntStatus = WdfDeviceCreateSymbolicLink(controlDevice, &dosName);
	if (!NT_SUCCESS(ntStatus))
	{
		DbgPrint("\n%s : wdf create symbolic link failed\n", __FUNCTION__);
		return ntStatus;
	}

	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig, WdfIoQueueDispatchSequential);
	queueConfig.EvtIoDeviceControl = DeviceIoControl;

	ntStatus = WdfIoQueueCreate(controlDevice, &queueConfig, WDF_NO_OBJECT_ATTRIBUTES, NULL);
	if (!NT_SUCCESS(ntStatus))
	{
		DbgPrint("\n%s : wdf io queue create failed\n", __FUNCTION__);
		return ntStatus;
	}


	WdfControlFinishInitializing(controlDevice);
	//ControlDevice = controlDevice;
	DbgPrint("\n%s : Exit\n", __FUNCTION__);
	return STATUS_SUCCESS;
}

VOID CleanUp(WDFOBJECT device)
{
	PAGED_CODE();
	UNREFERENCED_PARAMETER(device);

	DbgPrint("\n%s : Enter\n", __FUNCTION__);

	DbgPrint("\n%s : Exit\n", __FUNCTION__);
}
