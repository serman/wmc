//#include "ofMain.h"
#include "ofApp.h"

#define NOWINDOW

#ifdef NC
#include "ofAppConsoleCurses.h"
#endif

#ifdef NOWINDOW
#include "ofAppNoWindow.h"
#endif

int main()
{
//#ifdef NC
//    ofAppConsoleCurses window;
//    ofSetupOpenGL(&window,1280,720,OF_WINDOW);
#ifdef NOWINDOW
    ofAppNoWindow w;
    ofSetupOpenGL(&w,1024,768, OF_WINDOW);
#else
    ofSetupOpenGL(1280,720,OF_WINDOW);
#endif	
    
	ofRunApp(new ofApp());
}
