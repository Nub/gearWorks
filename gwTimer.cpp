/*************************************************************************\
 Engine: gearWorks
 File: gwTimer.cpp
 Author: Zachry Thayer
 Description: This class implements a gwTimer and 
 Requires: -lpsprtc
\*************************************************************************/

#include "gwTimer.h"

/*
 Constructor
 */
gwTimer::gwTimer(){
	startTick = clock();//Obtain current tick
	currTick = startTick;//All ticks are the same since we have just been created
	lastTick = currTick;//copy ^
	deltaTick = 0;// X - X = 0
	
	elapsedSeconds = 0.f;//no time has passed
	deltaSeconds = 0.f;//ditto ^
	
	clocksPerSecInv = 1.0f/(float)CLOCKS_PER_SEC;//invert
}

/*
 Deconstructor
 */

gwTimer::~gwTimer(){
	for(unsigned int i = 0; i < callbackList.size(); i++){
		deleteEvent(callbackList[i]);//free memory
	}
}

/*
 Internal Callbacks update
 */
void gwTimer::updateCallbacks(){
	for(unsigned int i = 0; i < callbackList.size(); i++){//loop through all callbacks
		if(currTick - callbackList[i]->lastTick >= callbackList[i]->interval){//if past due
			callbackList[i]->callback(callbackList[i]->pData);//call the function
			
			callbackList[i]->lastTick = currTick;//set the time last called to now
			
			if(callbackList[i]->iterations < 0)//if negative iterations (it is a timeout
				callbackList.erase(callbackList.begin() + i);//remove this callback, as it is a timeout
			else//is an interval
				callbackList[i]->iterations ++;//increase the iteration count
		}
	}
}


/*
 Update Timer (to be called once per main loop[more often for more accuracy])
 */
void gwTimer::update(){
	lastTick = currTick;//store the currTick before we update it
	currTick = clock();//update current tick
	deltaTick = currTick - lastTick;//calculate the difference in ticks
	deltaSeconds = (float)deltaTick*(float)clocksPerSecInv;//Simple multiplication to find the change in seconds
	elapsedSeconds += deltaSeconds;// add to overall time
	
	//calculate the Frames Per Second
	fpsTick += deltaTick;
	frameCount ++;
	currFps = (float)frameCount/(float)fpsTick*(float)CLOCKS_PER_SEC;
	if(fpsTick >= CLOCKS_PER_SEC){//one second has passed
		frameCount = 0;
		fpsTick = 0;
	}
	
	updateCallbacks();// need I comment this?
}

/*
 Create a callback creates event and adds it to the callback list
 */
gwTimerEvent *gwTimer::newEvent(float interval, gwTimer_callback callback, void *pData, bool isTimeout){
	
	gwTimerEvent *event = new gwTimerEvent;// allocate data
	if(!event) return NULL;//if failed to allocate return NULL
	event->index = callbackList.size();
	event->iterations = isTimeout?-1:1;//initial values
	event->interval = (int)((float)interval * (float)CLOCKS_PER_SEC); //convert from seconds to ticks (for speed increase)
	event->callback = callback;//set the callback function
	event->lastTick = currTick;//initial value
	event->pData = pData;
	//add to event list
	callbackList.push_back(event);
	
	return event;
}

/*
 Removes a callback from the callback list
 */
bool gwTimer::deleteEvent(gwTimerEvent *event){
	if(!event) return false;//invalid arguement
	
	callbackList.erase(callbackList.begin() + event->index);//erase the callback from the list
	delete event;//free data
	event = NULL;//set to NULL to avoid issues
	
	return true;//we went through the whole array and didnt find the intance
}

/*
 Return Elapsed time since the timer was created
 */
const float gwTimer::elapsedTime(){
	return elapsedSeconds;
}

/*
 Return Delta time since the timer was updated
 */
const float gwTimer::deltaTime(){
	return deltaSeconds;
}

/*
 Returns the fps of the update (how many times update is called per second)
 */
const float gwTimer::fps(){
	return currFps;
}

/*
 Reset elapsed time to 0
 */
void gwTimer::reset(){
	elapsedSeconds = 0.f;
}

