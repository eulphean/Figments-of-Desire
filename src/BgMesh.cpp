#include "BgMesh.h"

void BgMesh::setup() {
  bgImage.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
  bgImage.begin();
    ofClear(0, 0, 0, 0);
    int numRows = 30;
    int numCols = 30;
  
    int rectW = bgImage.getWidth()/numCols;
    int rectH = bgImage.getHeight()/numRows;
  
    int a = 0;
    for (int y = 0; y < numCols; y++) {
      for (int x = 0; x < numRows; x++) {
        if (a % 2 == 0) {
          ofSetColor(ofColor::fromHex(0xDBDBDB));
        } else {
          ofSetColor(ofColor::fromHex(0x706F6F));
        }
        ofPushMatrix();
        ofTranslate(x * rectW, y * rectH);
          ofDrawRectangle(0, 0, rectW, rectH);
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
