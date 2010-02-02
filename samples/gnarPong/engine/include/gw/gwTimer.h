/*************************************************************************\
 Engine: gearWorks
 File: gwTimer.h
 Author: Zachry Thayer
 Description: This class implements a Timer and Time based callbacks
 Requires: ctime
\*************************************************************************/
#ifndef _GWTIMER_H
#define _GWTIMER_H

#include <ctime>
#include <vector>

// This is the callback template     this is a pointer to data passed to the callback
typedef void (*gwTimer_callback)(void *pData);

// Callback structure (stores vital infomation)
typedef struct _timer_event {
	int index;//index in calback vector (for faster deletion)
	gwTimer_callback callback;
	void *pData;//to be passed to the callback
	unsigned int interval;//time in ticks to wait for each call (converted from seconds for speed improvements)
	int iterations;//how many times has this function been called?
	clock_t lastTick;//the time this func was last called
} gwTimerEvent;

class gwTimer{
private:
	float clocksPerSecInv;//1/CLOCKS_PER_SEC (used for a speed increase since * is faster than /)
	
	clock_t startTick;//the tick for when we first create an instance
	clock_t currTick;//the most recent tick read
	clock_t lastTick;//the tick from the last update
	clock_t deltaTick;//currTick - lastTick
	
	float elapsedSeconds;// time since created in seconds
	float deltaSeconds;// time since last tick in seconds
	
	clock_t fpsTick;
	float currFps;
	int frameCount;
	
	std::vector<gwTimerEvent*> callbackList;//list of callbacks
	
	void updateCallbacks();//internall called by update (calls all callbacks that ought to be called)
public:
	/*
	 Constructor
	 */
	gwTimer();
	
	/*
	 Deconstructor
	 */
	~gwTimer();
	
	/*
	 Update Timer (to be called once per main loop[more often for more accuracy])
	 */
	void update();
	
	/*
	 Create a callback creates event and adds it to the callback list
	 */
	gwTimerEvent *newEvent(float interval, gwTimer_callback callback, void *pData = NULL, bool isTimeout = false);
	
	/*
	 Removes a callback from the callback list
	 */
	bool deleteEvent(gwTimerEvent *event);
	
	/*
	 Return Elapsed time since the timer was created
	 */
	const float elapsedTime();
	
	/*
	 Return Delta time since the timer was created
	 */
	const float deltaTime();
	
	/*
	 Returns the fps of the update (how many times update is called per second)
	 */
	const float fps();
	
	/*
	 Reset elapsed time to 0
	 */
	void reset();
};

#endif
