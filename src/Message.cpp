#include "Message.h"

Message::Message(glm::vec2 loc, ofColor col, float s) {
  location = loc;
  color = col;
  size = s;
}

void Message::draw() {
  ofPushMatrix();
    ofTranslate(location);
      ofPushStyle();
      ofSetColor(color);
      ofDrawCircle(0, 0, size);
      ofPopStyle();
  ofPopMatrix();
}

