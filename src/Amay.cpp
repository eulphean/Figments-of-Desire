#include "Amay.h"

// Customize filters and textures for Amay

Amay::Amay(ofxBox2d &box2d, AgentProperties agentProps) {  
  // Assign Amay's mesh origin (left corner).
  agentProps.meshOrigin = ofPoint(10, 20);
  
  // Assign a color palette
  palette = { ofColor::fromHex(0x540D6E), ofColor::fromHex(0x982A41), ofColor::fromHex(0xFFEEB9), ofColor::fromHex(0x3BCEAC), ofColor::fromHex(0x0EAD69) };
  
  // Messages
  this->numBogusMessages = 400;
  this->font.load("opensansbold.ttf", 60);
  setup(box2d, agentProps, "amay.txt"); // TODO: Actually pass a pointer to all the messages later (for now it's assigned randomly)

  // Post process filters.
  filter = new PerlinPixellationFilter(agentProps.meshSize.x, agentProps.meshSize.y, 1.f);
}
