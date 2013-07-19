#include "WINSPOOL.H" 
BOOL SetSystemDefaultPrinter(LPTSTR pPrinterName) 
{ 
	BOOL bFlag = FALSE; 
	LONG lResult = 0; 
	DWORD dwNeeded = 0; 
	LPTSTR pBuffer = NULL; 
	HANDLE hPrinter = NULL; 
	OSVERSIONINFO stOsvInfo = {0}; 
	PRINTER_INFO_2* pstPrintInfo2 = NULL; 

	stOsvInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO); 
	GetVersionEx(&stOsvInfo); 

	if(!pPrinterName) 
	{ 
		return FALSE; 
	} 

	if(stOsvInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) //Win9x 
	{ 
		// Open this printer so we can get information about it 
		bFlag = OpenPrinter(pPrinterName, &hPrinter, NULL); 
		if(!bFlag || hPrinter==NULL) 
		{ 
			return FALSE; 
		} 

		GetPrinter(hPrinter, 2, 0, 0, &dwNeeded); 
		if(dwNeeded == 0) 
		{ 
			ClosePrinter(hPrinter); 
			return FALSE; 
		} 

		pstPrintInfo2 = (PRINTER_INFO_2 *)GlobalAlloc(GPTR, dwNeeded); 
		if(pstPrintInfo2 == NULL) 
		{ 
			ClosePrinter(hPrinter); 
			return FALSE; 
		} 

		bFlag = GetPrinter(hPrinter, 2, (LPBYTE)pstPrintInfo2, dwNeeded, &dwNeeded); 
		if(!bFlag) 
		{ 
			ClosePrinter(hPrinter); 
			GlobalFree(pstPrintInfo2); 
			return FALSE; 
		} 

		// Set default printer attribute for this printer... 
		pstPrintInfo2->Attributes |= PRINTER_ATTRIBUTE_DEFAULT; 
		bFlag = SetPrinter(hPrinter, 2, (LPBYTE)pstPrintInfo2, 0); 
		if(!bFlag) 
		{ 
			ClosePrinter(hPrinter); 
			GlobalFree(pstPrintInfo2); 
			return FALSE; 
		} 

		lResult = SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0L, 
			(LPARAM)(LPCTSTR)"windows", SMTO_NORMAL, 1000, NULL); 
	} 
	else if (stOsvInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) 
	{ 
#if(WINVER >= 0x0500) 
		if(stOsvInfo.dwMajorVersion >= 5) // Windows 2000 or later... 
		{ 
			bFlag = SetDefaultPrinter(pPrinterName); 
			if(!bFlag) 
			{ 
				return FALSE; 
			} 
		} 
		else // NT4.0 or earlier... 
#endif 
		{ 
			bFlag = OpenPrinter(pPrinterName, &hPrinter, NULL); 
			if(!bFlag || hPrinter==NULL) 
			{ 
				return FALSE; 
			} 

			GetPrinter(hPrinter, 2, 0, 0, &dwNeeded); 
			if(dwNeeded == 0) 
			{ 
				ClosePrinter(hPrinter); 
				return FALSE; 
			} 

			pstPrintInfo2 = (PRINTER_INFO_2*)GlobalAlloc(GPTR, dwNeeded); 
			if(pstPrintInfo2 == NULL) 
			{ 
				ClosePrinter(hPrinter); 
				return FALSE; 
			} 

			bFlag = GetPrinter(hPrinter, 2, (LPBYTE)pstPrintInfo2, dwNeeded, &dwNeeded); 
			if((!bFlag) || (!pstPrintInfo2->pDriverName) || (!pstPrintInfo2->pPortName)) 
			{ 
				ClosePrinter(hPrinter); 
				GlobalFree(pstPrintInfo2); 
				return FALSE; 
			} 

			pBuffer = 
			(LPTSTR)GlobalAlloc(GPTR, lstrlen(pPrinterName)+lstrlen(pstPrintInfo2->pDriverName)
				+lstrlen(pstPrintInfo2->pPortName)+3); 
			if(pBuffer == NULL) 
			{ 
				ClosePrinter(hPrinter); 
				GlobalFree(pstPrintInfo2); 
				return FALSE; 
			} 

			// Build string in form "printername,drivername,portname" 
			lstrcpy(pBuffer, pPrinterName); lstrcat(pBuffer, ","); 
			lstrcat(pBuffer, pstPrintInfo2->pDriverName); lstrcat(pBuffer, ","); 
			lstrcat(pBuffer, pstPrintInfo2->pPortName); 

			bFlag = WriteProfileString("windows", "device", pBuffer); 
			if(!bFlag) 
			{ 
				ClosePrinter(hPrinter); 
				GlobalFree(pstPrintInfo2); 
				GlobalFree(pBuffer); 
				return FALSE; 
			} 
		} 

		lResult = SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE,
			0L, 0L, SMTO_NORMAL, 1000, NULL); 
	} 

	if(hPrinter) 
	{ 
		ClosePrinter(hPrinter); 
	} 
	if(pstPrintInfo2) 
	{ 
		GlobalFree(pstPrintInfo2); 
	} 
	if(pBuffer) 
	{ 
		GlobalFree(pBuffer); 
	} 

	return TRUE; 
} 

使用示例： 
SetSystemDefaultPrinter("PrimoPDF"); 
参数为打印机名，具体名称见“控制面板->打印机和传真”。 