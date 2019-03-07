#pragma once

#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofxGui.h"
#include "Agent.h"
#include "SuperAgent.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
  
    void setupGui();
    void createAgent();
    void clearAgents();
    void updateAgentProps();
  
    // InterAgentJoints
    void cleanInterAgentJoints();
  
    // Contact listening callbacks.
    void contactStart(ofxBox2dContactArgs &e);
    void contactEnd(ofxBox2dContactArgs &e);
  
    // Interactive elements
		void keyPressed(int key);
    void mousePressed(int x, int y, int button); 
    void exit();
  
    bool hideGui;
    bool debug;
  
    // Box2d
    ofxBox2d box2d;
  
    // Agents
    std::vector<Agent *> agents;
    AgentProperties agentProps;
  
    // GUI
    ofxPanel gui;
    ofParameterGroup settings; 
  
    // Mesh group
    ofParameterGroup meshParams;
    ofParameter<int> meshColumns;
    ofParameter<int> meshRows;
    ofParameter<int> meshWidth;
    ofParameter<int> meshHeight;
  
    // Vertex group
    ofParameterGroup vertexParams;
    ofParameter<float> vertexRadius;
    ofParameter<float> vertexDensity;
    ofParameter<float> vertexBounce;
    ofParameter<float> vertexFriction;
  
    // Agent joint (joints inside the agent)
    ofParameterGroup jointParams;
    ofParameter<float> jointFrequency;
    ofParameter<float> jointDamping;
  
    // InterAgentJoint
    ofParameterGroup interAgentJointParams;
    ofParameter<float> frequency;
    ofParameter<float> damping;
    ofParameter<int> maxJointForce; 
  
  private:
    // Body repel timer. 
    bool startRepelling; 
  
    std::vector<std::shared_ptr<ofxBox2dJoint>> interAgentJoints;
    std::vector<b2Body *> collidingBodies;
  
    // Helper methods.
    void handleSerial();
  
    // Super Agents (Inter Agent Bonding Logic)
    void createSuperAgents();
    std::shared_ptr<ofxBox2dJoint> createInterAgentJoint(b2Body *bodyA, b2Body *bodyB);
    void evaluateBonding(b2Body* bodyA, b2Body* bodyB, Agent *agentA, Agent *agentB);
    bool hasVisualSimilarities(Agent *agentA, Agent *agentB);
    bool canVertexBond(b2Body* body, Agent *curAgent);
  
    // Unused for now.
    void enableRepulsion();
    int findOtherAgent(b2Body* body, int curAgentId);
  
    // Serial
    ofSerial serial;
  
    // SuperAgents => These are abstract agents that have a bond with each other. 
    std::vector<SuperAgent> superAgents;
};
