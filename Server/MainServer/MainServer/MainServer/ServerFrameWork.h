#pragma once
class ServerFrameWork
{
private:
	SOCKET m_listensock;

public:
	void Initialize();
	void Accept_Process();
public:

	void error_display(const char *msg, int err_no);
	void error_display(const char *msg);
	
public:
	ServerFrameWork();
	~ServerFrameWork();

};

