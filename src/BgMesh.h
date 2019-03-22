#pragma once
#include "ofMain.h"

class BgMesh {
  public:
    void setup();
    void update();
    void draw();
  
  private:
    ofFbo bgImage;
};
