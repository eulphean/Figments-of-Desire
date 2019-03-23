#pragma once
#include "ofMain.h"

class BgMesh {
  public:
    void createBg(int rectWidth, int rectHeight);
    void update();
    void draw();
  
  private:
    ofFbo bgImage;
};
