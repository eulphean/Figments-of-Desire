#include "Amay.h"

// Customize filters and textures for Amay

Amay::Amay(ofxBox2d &box2d, AgentProperties agentProps) {
  // Initial setup for this agent.
  // Prepare texture
  _filters.push_back(new PerlinPixellationFilter(agentProps.meshSize.x, agentProps.meshSize.y));
  _filters.push_back(new PixelateFilter(agentProps.meshSize.x, agentProps.meshSize.y));
  
  // Populate color slots.
  populateSlots();
  createTexture(agentProps.meshSize); // Design the look of this creature.
  
  // Assign Amay's mesh origin (left corner).
  agentProps.meshOrigin = ofPoint(10, 20);
  
  // Call parent's setup method.
  this->setup(box2d, agentProps);
}

void Amay::populateSlots() {
  colorSlots.clear();
  for (int i = 0; i < maxSlots; i++) {
    int randIdx = ofRandom(colors.size());
    colorSlots.push_back(colors.at(randIdx));
  }
}

void Amay::createTexture(ofPoint meshSize) {
  // Create a simple fbo.
  fbo.allocate(meshSize.x, meshSize.y, GL_RGBA);
  fbo.begin();
    ofClear(0, 0, 0, 0);
    ofColor c = ofColor(colorSlots.at(0), 200);
    ofBackground(c);
    const int firstRecs = 1; // Biased towards later colors
    // Create the slots in the fbo.
    for (int i = 0; i < maxSlots; i++) {
      //int numRecs = firstRecs * (i+1);
      int numRecs = 100;
      ofSetColor(colorSlots.at(i));
      for (int j = 0; j < numRecs; j++) {
        auto x = ofRandom(0, fbo.getWidth());
        auto y = ofRandom(0, fbo.getHeight());
        //ofDrawRectangle(x, y, x+ofRandom(10, 15), y+ofRandom(10, 15));
        ofDrawCircle(x, y, 10);
      }
    }
  fbo.end();
}

// Draw this agent and its super agent. 
void Amay::drawAgent(bool debug, bool showTexture) {
  this->draw(debug, showTexture);

  // Draw the mesh.
  // Bind the fbo.
  _filters[_currentFilter]->begin();
  fbo.getTexture().bind();
  mesh.draw();
  fbo.getTexture().unbind();
  _filters[_currentFilter]->end();
}
