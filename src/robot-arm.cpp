#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sstream>
#include <thread>
#include <ctime>
#include <signal.h>
#include <math.h>

#include <uln200xa.h>
#include <joystick12.h>

#include "html.h"
#include "../lib/crow/crow_all.h"

using namespace std;


// The hardware devices that the example is going to connect to
struct Devices
{
  upm::ULN200XA* stepOne;
  upm::ULN200XA* stepTwo;
  upm::Joystick12* joy;

  Devices(){
  };

  // Initialization function
  void init() {

    // One stepper motor connected to d9,10,11,12
    stepOne = new upm::ULN200XA(4096, 9, 10, 11, 12);
    // Two stepper motor connected to 4, 5, 6, 7
    stepTwo = new upm::ULN200XA(4096, 4, 5, 6, 7);
    //joystick connected to A0 and A1
    joy = new upm::Joystick12(0,1);
  };

  // Cleanup on exit
  void cleanup() {
    stepOne->release();
    stepTwo->release();
    delete stepOne;
    delete stepTwo;
  }

  //functions to set speed and direction of motors
  //steps 4096 is one full revolution of the motor
  void OneMoveClockwise(){
	  stepOne->setSpeed(5);
	  stepOne->setDirection(upm::ULN200XA::DIR_CW);
	  stepOne->stepperSteps(4096);
  }
  void OneMoveCounterClock(){
	  stepOne->setSpeed(5);
	  stepOne->setDirection(upm::ULN200XA::DIR_CCW);
	  stepOne->stepperSteps(4096);
  }
  void TwoMoveClockwise(){
	  stepTwo->setSpeed(5);
	  stepTwo->setDirection(upm::ULN200XA::DIR_CW);
	  stepTwo->stepperSteps(4096);
  }
  void TwoMoveCounterClock(){
	  stepTwo->setSpeed(5);
	  stepTwo->setDirection(upm::ULN200XA::DIR_CCW);
	  stepTwo->stepperSteps(4096);
  }

  //function to turn the joystick input -1, 0, or 1
  int scale(int n){
	  auto val = (n - -0.5) / -0.4;
	  if (val > 1) { val = 1; }
	  if (val < 0) { val = 0; }
	  return round(val * 2 - 1);
  }

  //functions to get xy axis from joystick
  float getX(){
	 float x = scale(joy->getXInput());
		 return x;
  }
  float getY(){
	  float y = scale(joy->getYInput());
	  	  return y;
  }
//function for joystick input and movement
  void joyMove(){
	  float move = getX();
	  float move2 = getY();

	  	  if(move == 1 )
	  		OneMoveClockwise();
	  	  else if(move == -1)
	  		OneMoveCounterClock();

	  	  if (move2 == 1)
	  		TwoMoveClockwise();
	  	  else if (move2 == -1)
	  		TwoMoveCounterClock();
  }
};


void runner(Devices& devices){
	for(;;){
		//devices.joyMove();
	}
}
Devices devices;
// Exit handler for program
void exit_handler(int param)
{
  devices.cleanup();
  exit(1);
}
// The main function for the example program
int main() {
  // Handles ctrl-c or other orderly exits
  signal(SIGINT, exit_handler);

  // check that we are running on Galileo or Edison
  mraa_platform_t platform = mraa_get_platform_type();
  if ((platform != MRAA_INTEL_GALILEO_GEN1) &&
    (platform != MRAA_INTEL_GALILEO_GEN2) &&
    (platform != MRAA_INTEL_EDISON_FAB_C)) {
    std::cerr << "ERROR: Unsupported platform" << std::endl;
    return MRAA_ERROR_INVALID_PLATFORM;
  }

  devices.init();
  std::thread t1(runner, ref(devices));
  crow::SimpleApp app;

  CROW_ROUTE(app, "/")
  ([](const crow::request& req) {
	    std::stringstream text;
	    text << index_html;
	    return text.str();
  });

  CROW_ROUTE(app, "/one-cw")
  .methods("POST"_method)
  ([](const crow::request& req) {
      if (req.method == "POST"_method) {
      devices.OneMoveClockwise();
      return crow::response("OK");
      }
  });

  CROW_ROUTE(app, "/one-ccw")
  .methods("POST"_method)
  ([](const crow::request& req) {
      if (req.method == "POST"_method) {
      devices.OneMoveCounterClock();
      return crow::response("OK");
      }
  });

  CROW_ROUTE(app, "/two-cw")
  .methods("POST"_method)
  ([](const crow::request& req) {
      if (req.method == "POST"_method) {
      devices.TwoMoveClockwise();
      return crow::response("OK");
      }
  });

  CROW_ROUTE(app, "/two-ccw")
  .methods("POST"_method)
  ([](const crow::request& req) {
      if (req.method == "POST"_method) {
      devices.TwoMoveCounterClock();
      return crow::response("OK");
      }
  });

  // start web server
  app.port(3000).multithreaded().run();
  t1.join();

  return MRAA_SUCCESS;
}
