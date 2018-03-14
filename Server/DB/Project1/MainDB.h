#pragma once

class MainDB
{
private:
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;
	SQLINTEGER num;

	SQLWCHAR sCustID[20];
	SQLLEN cbName = 0, cbCustID = 0, cbPhone = 0;

public:
	bool Initialize();
	void Connect_DB();
	MainDB();
	~MainDB();


};

