
// mqtt_client.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// Cmqtt_clientApp: 
// �йش����ʵ�֣������ mqtt_client.cpp
//

class Cmqtt_clientApp : public CWinApp
{
public:
	Cmqtt_clientApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern Cmqtt_clientApp theApp;