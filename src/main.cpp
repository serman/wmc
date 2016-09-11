//#include "ofMain.h"
#include "ofApp.h"
//#define NCURSES

#ifdef NCURSES
#include "ofAppConsoleCurses.h"
#endif

int main()
{
#ifdef NCURSES
    ofAppConsoleCurses window;
    ofSetupOpenGL(&window,1280,720,OF_WINDOW);
#else
    ofSetupOpenGL(1280,720,OF_WINDOW);
#endif	
    
	ofRunApp(new ofApp());
}
