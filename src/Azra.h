// Azra is a synthetic agent carrying information regarding Azra's behavior and traits. 
#pragma once
#include "ofMain.h"
#include "Agent.h"

class Azra : public Agent {
  public:
    Azra(ofxBox2d &box2d, AgentProperties agentProps);
    void drawAgent(bool debug, bool showTexture);
    
  private:
    void createTexture(ofPoint meshSize);
    void populateSlots();
  
    // Define Azra's own texture requirements for the texture design. 
    const int maxSlots = 8; // Number of slots.
    ofFbo fbo;
    vector<AbstractFilter *>  _filters;
    int _currentFilter = 0;
  
    // Color dimension of this agent.
    std::vector<ofColor> colorSlots;
    // Core colors.
    std::array<ofColor, 8> colors = { ofColor::fromHex(0x5D3DFF), ofColor::fromHex(0xF48327), ofColor::fromHex(0xF72E57), ofColor::fromHex(0x7CC934),
            ofColor::fromHex(0x2D3D30), ofColor::fromHex(0xFF6B6C), ofColor::fromHex(0xFFC145), ofColor::fromHex(0x064789) };
    std::vector<std::shared_ptr<ofxBox2dCircle>> vertices; // Every vertex in the mesh is a circle.
};
