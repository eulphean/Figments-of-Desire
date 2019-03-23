#include "BgMesh.h"

// Setup background
void BgMesh::createBg(int rectWidth, int rectHeight) {
  bgImage.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
  bgImage.begin();
    ofClear(0, 0, 0, 0);
  int numRows = bgImage.getHeight()/rectHeight;
  int numCols = bgImage.getWidth()/rectWidth;
  
    int a = 0;
    for (int y = 0; y < numRows; y++) {
      for (int x = 0; x < numCols; x++) {
        if (a % 2 == 0) {
          ofSetColor(ofColor::fromHex(0xDBDBDB));
        } else {
          ofSetColor(ofColor::fromHex(0x706F6F));
        }
        ofPushMatrix();
        ofTranslate(x * rectWidth, y * rectHeight);
          ofDrawRectangle(0, 0, rectWidth, rectHeight);
        ofPopMatrix();
        a++;
      }

    a++;
  }
  
  // Create a mesh and texture map the fbo to it.
  bgImage.end();
}

void BgMesh::update() {
  // Pass the centroid of two organisms into
}

void BgMesh::draw() {
  bgImage.draw(0, 0);
}
