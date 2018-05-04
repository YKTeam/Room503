

#ifndef GAMETIMER_H
#define GAMETIMER_H

class GameTimer
{
public:
	GameTimer();

	float TotalTime()const; // 초 단위
	float DeltaTime()const; // 초 단위

	void Reset(); // 메시지 루프 이전에 호출
	void Start(); // 타이머를 시작 또는 재개
	void Stop();  // 타이머를 정지
	void Tick();  // 매 프레임 호출한다

private:
	double mSecondsPerCount;
	double mDeltaTime;

	__int64 mBaseTime;   //리셋 호출시 현재 시간으로 초기화
	__int64 mPausedTime; //펄즈시 누적
	__int64 mStopTime;    // 정지 시점의 시간
	__int64 mPrevTime;   
	__int64 mCurrTime;

	bool mStopped;
};

#endif // GAMETIMER_H