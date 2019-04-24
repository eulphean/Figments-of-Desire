// This is a class for memory object that gets created for each joint.
#pragma once
#include "ofMain.h"
#include "ofxBox2d.h"

class Memory {
  public:
    Memory(ofxBox2d &box2d);
    void update();
    void draw();
  
  private:
    std::shared_ptr<ofxBox2dCircle> mem; 
};
