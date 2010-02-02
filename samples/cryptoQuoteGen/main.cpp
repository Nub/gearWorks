#include <pspdebug.h>
#include <pspkernel.h>
#include <gw/gearWorks.h>
#include <string.h>

PSP_MODULE_INFO("Crypto Quote Gen", PSP_MODULE_USER, 0, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(20480);

gwRect letterSize = {2,2,40,40};

void renderLetter(gwRender *context, float X, float Y, unsigned char letter){
	if(letter == ' ') return;//nothing to do
	float xPos = 0, yPos = 0;
	for(int y = 0; y < 4; y ++){
		for(int x = 0; x < 4; x ++){
			xPos = X + x * (letterSize.w/4);
			yPos = Y + y * (letterSize.h/4);
			
			if(letter & (1 << (y + (x*4)))){
				context->setPrimitiveRenderMode(GW_FILL);
				context->rectangle(context->tmpRect(xPos,yPos,(letterSize.w/4)-letterSize.x,(letterSize.h/4)-letterSize.y));
			}
			
			context->setPrimitiveRenderMode(GW_OUTLINE);
			context->rectangle(context->tmpRect(xPos,yPos,(letterSize.w/4)-letterSize.x,(letterSize.h/4)-letterSize.y),0xFF000000);
			
			
		}
	}
}

int main(int argc, void *argv[]){
	
	char *quote = "abcdef";
	
	GearWorks::init();
	pspDebugScreenInit();

	gwRender *render = new gwRender();
	render->setClearColor(0xFF888888);
	render->setPrimitiveRenderMode(GW_FILL);
		
	
	gwTimer *timer = new gwTimer();
	
	while (GearWorks::isRunning()) {
		timer->update();
		
		render->start();
		{
			for(int i = 0; i < strlen(quote); i++){
				renderLetter(render,10 + i * (letterSize.w+5), 50 , quote[i]);
			}
		}
		render->end(true);
		
		pspDebugScreenSetXY(0,0);
		pspDebugScreenPrintf("Quote: %s\tFPS: %f", quote,timer->fps());

	}
		
	GearWorks::exitGame();
	return 1;
}
