#include "Azra.h"

Azra::Azra(ofxBox2d &box2d, AgentProperties agentProps) {
  // Assign Azra's mesh origin (right corner).
  ofPoint p = ofPoint(ofGetWidth() - agentProps.meshSize.x - 10, ofGetHeight() - agentProps.meshSize.y - 20);
  agentProps.meshOrigin = p;
  
  // Color palette
  //palette = { ofColor::fromHex(0x0FA3B1), ofColor::fromHex(0x777D75), ofColor::fromHex(0xEEE1AC), ofColor::fromHex(0xF7A072), ofColor::fromHex(0xFF9B42) };
  
  palette = { ofColor::fromHex(0xFFBE0B), ofColor::fromHex(0xFB5607), ofColor::fromHex(0xFF006E), ofColor::fromHex(0x8338EC), ofColor::fromHex(0x3A86FF) };
  this->numBogusMessages = 400;
  this->font.load("caviar.ttf", 25);
  setup(box2d, agentProps, "azra.txt");

  // Post process filters.
  filter = new PerlinPixellationFilter(agentProps.meshSize.x, agentProps.meshSize.y, 9.f);
}
