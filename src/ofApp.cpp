#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  ofBackground(0);
  ofSetCircleResolution(20);
  
  box2d.init();
  box2d.setGravity(0, 0);
  box2d.setFPS(60);
  box2d.enableEvents();
  box2d.registerGrabbing(); // Enable grabbing the circles.
  
  ofAddListener(box2d.contactStartEvents, this, &ofApp::contactStart);
  ofAddListener(box2d.contactEndEvents, this, &ofApp::contactEnd);
  
  // Setup gui.
  setupGui();
  
  hideGui = false;
  startRepelling = false;
  
  // Boundaries
  ofRectangle bounds;
  bounds.x = 0; bounds.y = 0;
  bounds.width = ofGetWidth(); bounds.height = ofGetHeight();
  box2d.createBounds(bounds);
  
  agentNum = 0;
  
  //serial.setup("/dev/cu.usbmodem1411", 9600);
}


//--------------------------------------------------------------
void ofApp::update(){
  box2d.update();
//
//  handleSerial();
  updateAgentProps();
  
  for (auto &a : agents) {
    a.update();
  }
  
  // Should I create interAgent joints?
  if (collidingBodies.size()>0) {
    // We have pending bodies to create a joint with.
    auto jointList = collidingBodies[0]->GetJointList();
    auto j = std::make_shared<ofxBox2dJoint>();
    j->setup(box2d.getWorld(), collidingBodies[0], collidingBodies[1], agentProps.jointPhysics.x, agentProps.jointPhysics.y);
    j->setLength(ofRandom(50, 100));
    interAgentJoints.push_back(j);
    collidingBodies.clear();
  }
  
  // Joint removal condition
  ofRemove(interAgentJoints, [&](std::shared_ptr<ofxBox2dJoint> c){
      auto f = c->getReactionForce(ofGetElapsedTimef());
      if (f.length() > 2) {
        // Remove the joint history
        VertexData *aData = reinterpret_cast<VertexData*>(c->joint->GetBodyA()->GetUserData());
        VertexData *bData = reinterpret_cast<VertexData*>(c->joint->GetBodyB()->GetUserData());
        
        // History of A
        auto it = interAgentHistory.find(aData->vertexId);
        if (it != interAgentHistory.end()) {
          interAgentHistory.erase(it);
        }
        
        // History of B
        it = interAgentHistory.find(bData->vertexId);
        if (it != interAgentHistory.end()) {
          interAgentHistory.erase(it);
        }
        
        return true;
      }
    
      return false;
  });
  
}

//--------------------------------------------------------------
void ofApp::draw(){
  // Draw InterAgentJoints
  for (auto j: interAgentJoints) {
    ofSetColor(ofColor::green);
    j->draw();
  }
  
  for (auto a: agents) {
    a.draw(true);
  }
  
  // Health parameters
  if (hideGui) {
    gui.draw();
  }
}

void ofApp::handleSerial() {
  while (serial.available() > 0)
    {
        // [Summary] Data is sent from the Arduino byte by byte.
        // In this while loop, we read the data byte by byte until
        // there is incoming data. Every character we are sending from
        // there is a byte wide. We keep constructing the buffer until
        // we reach the new line character, which determines that we are
        // done reading the first set of data.
    
        // Read the byte.
        char b = serial.readByte();
      
        // End of line character.
        if (b == '\n')
        {
            // Skip
            cout<< "New line" << "\n";
        }
        else
        {
            // If it's not the line feed character, add it to the buffer.
            cout << "Received: " << b << "\n";
            startRepelling = b - '0';
            cout << "Replling: " << startRepelling << "\n";
        }
    }
}

void ofApp::updateAgentProps() {
    // Create Soft Body payload to create objects.
  agentProps.meshDimensions = ofPoint(meshRows, meshColumns);
  agentProps.meshSize = ofPoint(meshWidth, meshHeight);
  agentProps.vertexRadius = vertexRadius;
  agentProps.vertexPhysics = ofPoint(vertexBounce, vertexDensity, vertexFriction); // x (bounce), y (density), z (friction)
  agentProps.jointPhysics = ofPoint(jointFrequency, jointDamping); // x (frequency), y (damping)
  agentProps.agentId = agentNum; 
}

void ofApp::createAgent() {
  Agent a;
  a.setup(box2d, agentProps);
  agents.push_back(a);
  agentNum++;
}

void ofApp::clearAgents() {
  // Clear agent's soft bodies
  for (auto &a: agents) {
    a.clean();
  }
  
  // Empty the vector
  agents.clear();
}

void ofApp::setupGui() {
    gui.setup();
    gui.add(meshRows.setup("Mesh Rows", 20, 1, 100));
    gui.add(meshColumns.setup("Mesh Columns", 20, 1, 100));
    gui.add(meshWidth.setup("Mesh Width", 50, 10, ofGetWidth()));
    gui.add(meshHeight.setup("Mesh Height", 50, 10, ofGetHeight()));
    gui.add(vertexRadius.setup("Vertex Radius", 5, 1, 30));
    gui.add(vertexDensity.setup("Vertex density", 0.5, 0, 5));
    gui.add(vertexBounce.setup("Vertex bounce", 0.5, 0, 1));
    gui.add(vertexFriction.setup("Vertex friction", 0.5, 0, 1));
    gui.add(jointFrequency.setup("Joint frequency", 4.f, 0.f, 20.f ));
    gui.add(jointDamping.setup("Joint damping", 1.f, 0.f, 5.f));
  
    gui.loadFromFile("InterMesh.xml");
}

void ofApp::contactStart(ofxBox2dContactArgs &e) {
  
}

// Store information during collision and make
// the bodies right after update. Check for joint making.
void ofApp::contactEnd(ofxBox2dContactArgs &e) {
  if(e.a != NULL && e.b != NULL) {
    
    // Only when a circle contacts the polygon, then this routine will run.
    if(e.a->GetType() == b2Shape::e_circle && e.b->GetType() == b2Shape::e_circle) {
      // Extract sound data.
      VertexData* aData = reinterpret_cast<VertexData*>(e.a->GetBody()->GetUserData());
      VertexData* bData = reinterpret_cast<VertexData*>(e.b->GetBody()->GetUserData());
      
      if (aData->agentId != bData->agentId) {
            // Should I create this joint? Do a and b agree?
            // Whose more eager to create this joint?
            // Bonding eagerness?
            collidingBodies.clear();
            bool a = isJointed(aData->vertexId, bData->agentId);
            bool b = isJointed(bData->vertexId, aData->agentId);
            if (!a & !b) {
              // Create a joint
              auto a = e.a->GetBody();
              auto b = e.b->GetBody();
              collidingBodies.push_back(a);
              collidingBodies.push_back(b);
              
              // This logic maintains the joint history for each of the InterAgentJoints
              
              // Update joint history for aData's vertexId
              auto it1 = interAgentHistory.find(aData->vertexId);
              if (it1 != interAgentHistory.end()) {
                auto &val = it1->second;
                val->push_back(bData->agentId);
              } else {
                auto vec = new std::vector<int>();
                vec->push_back(bData->agentId);
                interAgentHistory.insert(std::make_pair(aData->vertexId, vec));
              }
              
              // Update joint history for bData's vertexId
              auto it2 = interAgentHistory.find(bData->vertexId);
              if (it2 != interAgentHistory.end()) {
                auto &val = it2->second;
                val->push_back(aData->agentId);
              } else {
                auto vec = new std::vector<int>();
                vec->push_back(aData->agentId);
                interAgentHistory.insert(std::make_pair(bData->vertexId, vec));
              }
            }
        }
      }
    }
}

bool ofApp::isJointed(string vId, int agentId) {
  auto it1 = interAgentHistory.find(vId);
  if (it1 != interAgentHistory.end()) {
    // Does it have the agentid?
    auto a = it1->second;
    for (int i = 0; i < a->size(); i++) {
      if (agentId == a->at(i)) {
        return true;
      }
    }
    
    return false;
  } else {
    return false;
  }
}

void ofApp::cleanInterAgentJoints() {
  ofRemove(interAgentJoints, [&](std::shared_ptr<ofxBox2dJoint> c){
      return true;
  });
  
  interAgentJoints.clear();
}

void ofApp::enableRepulsion() {
    // Enable repelling on the agent.
    for (auto &j: interAgentJoints) {
      auto bodyA = j->joint->GetBodyA();
      auto bodyB = j->joint->GetBodyB();
      
      // Agent A (centroid will be the repulsion point)
      auto agentIdA = reinterpret_cast<VertexData*>(bodyA -> GetUserData()) -> agentId;
      auto &agentA = agents.at(agentIdA);
      auto centroidA = agentA.getCentroid();
      
      // Agent B (centroid will be the repulsion point)
      auto agentIdB = reinterpret_cast<VertexData*>(bodyB -> GetUserData()) -> agentId;
      auto &agentB = agents.at(agentIdB);
      auto centroidB = agentB.getCentroid();
      
      // Set a repulsion target for agent A
      agentA.setRepulsionTarget(centroidB);
      agentB.setRepulsionTarget(centroidA);
      
      // Set a repulsion target for agent B as well. After trying.
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
  if (key == 'n') {
    createAgent(); 
  }
  
  if (key == 'c') {
    collidingBodies.clear();
    cleanInterAgentJoints();
    clearAgents();
  }
  
  if (key == 'h') {
    hideGui = !hideGui;
  }
  
  if (key == 'f') {
    // Apply a random force
    for (auto &a: agents) {
      a.setRandomForce();
    }
  }
  
  if (key == 'j') {
    cleanInterAgentJoints();
  }
  
  if (key == 'r') {
    enableRepulsion();
  }
}

void ofApp::mousePressed(int x, int y, int button) {
   for (auto &a: agents) {
    a.setAttractionTarget(glm::vec2(x, y));
  }
}

void ofApp::exit() {
  gui.saveToFile("InterMesh.xml");
}


//
//      auto v = (bodyA->GetPosition() - bodyB->GetPosition());
//      auto v2 = b2Vec2(v.x * 2, v.y * 5);
//
//      // Get bodyA's agentId
//      // apply force on bodyB's vertices away from the centroid of bodyA
//
//      // Apply force on bodyA in the opposite direction of bodyB
//      bodyA->ApplyForceToCenter(v2, true);
//
//      // Apply force on bodyB in the opposite direction of bodyA
//      v2 = b2Vec2(-v.x * 2, -v.y * 5);
//      bodyB->ApplyForceToCenter(v2, true);
