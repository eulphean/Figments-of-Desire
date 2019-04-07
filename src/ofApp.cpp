#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  // Setup OSC
  receiver.setup(PORT);
  //ofHideCursor();
  
  debugFont.load("opensansbond.ttf", 30);
  
  ofBackground(ofColor::fromHex(0x2E2F2D));
  ofSetCircleResolution(20);
  ofDisableArbTex();
  ofEnableSmoothing();
  ofEnableAlphaBlending();
  
  box2d.init();
  box2d.setGravity(0, 0.0);
  box2d.setFPS(60);
  box2d.enableEvents();
  box2d.registerGrabbing(); // Enable grabbing the circles.
  
  ofAddListener(box2d.contactStartEvents, this, &ofApp::contactStart);
  ofAddListener(box2d.contactEndEvents, this, &ofApp::contactEnd);
  
  // Setup gui.
  setupGui();
  
  hideGui = false;
  debug = false;
  stopEverything = false; 
  showTexture = true;
  
  // Boundaries
  bounds.x = -50; bounds.y = -50;
  bounds.width = ofGetWidth() + (-1) * bounds.x * 2; bounds.height = ofGetHeight() + (-1) * 2 * bounds.y;
  box2d.createBounds(bounds);
  
  enableSound = true;
  
  // Instantiate Midi.
  Midi::instance().setup();
  
  serial.setup("/dev/cu.usb modem1411", 9600);
  
  // Store params and create background. 
  bg.setParams(bgParams);
  bg.createBg();
}

void ofApp::contactStart(ofxBox2dContactArgs &e) {
  
}

// Joint creation sequence.
void ofApp::contactEnd(ofxBox2dContactArgs &e) {
  // Based on the current state of desire, what should the vertices do if they hit each other
  // How do they effect each other?
  if (agents.size() > 0) {
    if(e.a != NULL && e.b != NULL) {
      if(e.a->GetType() == b2Shape::e_circle && e.b->GetType() == b2Shape::e_circle
          && e.a->GetBody() && e.b->GetBody()) {
        // Extract Agent pointers.
        Agent* agentA = reinterpret_cast<VertexData*>(e.a->GetBody()->GetUserData())->agent;
        Agent* agentB = reinterpret_cast<VertexData*>(e.b->GetBody()->GetUserData())->agent;
        
        // DEFINE INDIVIDUAL VERTEX BEHAVIORS.
        if (agentA != agentB) {
          // REPEL AGENT B's vertices
          if (agentA->desireState == LOW) {
            auto data =  reinterpret_cast<VertexData*>(e.b->GetBody()->GetUserData());
            data->applyRepulsion = true;
            e.b->GetBody()->SetUserData(data);
          }
          
          // Repel AGENT A's vertice
          if (agentB->desireState == LOW) {
            auto data =  reinterpret_cast<VertexData*>(e.a->GetBody()->GetUserData());
            data->applyRepulsion = true;
            e.a->GetBody()->SetUserData(data);
          }
          
          // Really long routine to evaluate if two vertices belonging to two different agents
          // can actually bond with each other or not. Take a look at the conditions under which
          // this bonding actually happens.
          if (agentA->desireState == HIGH && agentB->desireState == HIGH) {
            auto data =  reinterpret_cast<VertexData*>(e.a->GetBody()->GetUserData());
            
            // Body repels if the it doesn't have a joint.
            if (!data->hasInterAgentJoint) {
              data->applyRepulsion = true;
              e.a->GetBody()->SetUserData(data);
            }
            
            // Body repels if tt doesn't have a joint. 
            data =  reinterpret_cast<VertexData*>(e.b->GetBody()->GetUserData());
            if (!data->hasInterAgentJoint) {
              data->applyRepulsion = true;
              e.b->GetBody()->SetUserData(data);
            }
            
            evaluateBonding(e.a->GetBody(), e.b->GetBody(), agentA, agentB);
          }
        }
      }
    }
  }
}

//--------------------------------------------------------------
void ofApp::update(){
  box2d.update();
  processOsc();
  
  //handleSerial();
  ofRemove(superAgents, [&](SuperAgent &sa){
    sa.update(box2d, maxJointForce);
    return sa.shouldRemove;
  });
  
  // GUI props.
  updateAgentProps();
  
  std::vector<ofMesh> meshes;
  // Update agents
  for (auto &a : agents) {
    a -> update();
    meshes.push_back(a->getMesh());
  }
  
  // Create super agents based on collision bodies.
  createSuperAgents();
  
  // Update background
  bg.updateWithVertices(meshes);
}

//--------------------------------------------------------------
void ofApp::draw(){
  // Draw background.
  if (!debug) {
   bg.draw();
  }
  
  // Draw box2d bounds.
  ofPushStyle();
    ofSetColor(ofColor::fromHex(0x341517));
    ofFill();
    ofDrawRectangle(0, 0, bounds.x, ofGetHeight());
    ofDrawRectangle(0, ofGetHeight() - bounds.x, ofGetWidth(), bounds.x);
    ofDrawRectangle(ofGetWidth()-bounds.x, 0, bounds.x, ofGetHeight());
  ofPopStyle();

  // Draw all what's inside the super agents.
  for (auto sa: superAgents) {
    sa.draw();
  }
  
  // Draw Agent is the virtual method for derived class. 
  for (auto a: agents) {
    a -> draw(debug, showTexture);
  }
  
  // Health parameters
  if (hideGui) {
    gui.draw();
  }
}

void ofApp::processOsc() {
  while(receiver.hasWaitingMessages()){
    // get the next message
    ofxOscMessage m;
    receiver.getNextMessage(m);
    
    // ABLETON messages.
    // Process these OSC messages and based on which agent this needs to be delivered,
    // we send the messages to that agent.
    // They all correspond to some behavior.
    
    if(m.getAddress() == "/Back1"){
      float val = m.getArgAsFloat(0);
      for (auto &a : agents) {
        a -> setTickle(5.0);
      }
    }
    
    if(m.getAddress() == "/Back2"){
      float val = m.getArgAsFloat(0);
      for (auto &a : agents) {
        a -> setStretch(ofRandom(4, 7));
      }
    } 
    
    if(m.getAddress() == "/Bell"){
      float val = m.getArgAsFloat(0);
    }
    
    // STATE CHANGER!
    if(m.getAddress() == "/Melody"){
      float val = m.getArgAsFloat(0);
      cout << val << endl; 
      for (auto &a : agents) {
        if (val > 0 && val < 0.98) {
          a->setDesireState(HIGH);
        } else {
          a->setDesireState(LOW);
        }
      }
    }
    
    // UI messages.
    if(m.getAddress() == "/interMesh/width"){
      float val = m.getArgAsFloat(0);
      cout << val << endl;
      meshWidth = ofMap(val, 0, 1, 50, 250, true);
      cout << meshWidth << endl;
    }
    
    if(m.getAddress() == "/interMesh/height"){
      float val = m.getArgAsFloat(0);
      meshHeight = ofMap(val, 0, 1, 50, 250, true);
    }
    
    if(m.getAddress() == "/interMesh/rows"){
      float val = m.getArgAsFloat(0);
      meshRows = ofMap(val, 0, 1, 5, 15, true);
    }
    
    if(m.getAddress() == "/interMesh/columns"){
      float val = m.getArgAsFloat(0);
      meshColumns = ofMap(val, 0, 1, 5, 15, true);
    }
    
    if(m.getAddress() == "/interMesh/jointForce"){
      float val = m.getArgAsFloat(0);
      maxJointForce = ofMap(val, 0, 1, 1, 100, true);
    }
    
    if(m.getAddress() == "/interMesh/newMesh"){
      int val = m.getArgAsInt(0);
      createAgents();
    }
    
    if(m.getAddress() == "/interMesh/clearScreen"){
      int val = m.getArgAsInt(0);
      clearScreen();
    }
    
    if(m.getAddress() == "/interMesh/debug"){
      int val = m.getArgAsInt(0);
      debug = !debug;
    }
    
    if(m.getAddress() == "/interMesh/removeUnbonded"){
      int val = m.getArgAsInt(0);
      removeUnbonded();
    }
    
    if(m.getAddress() == "/interMesh/removeJoints"){
      int val = m.getArgAsInt(0);
      removeJoints();
    }
    
    if(m.getAddress() == "/interMesh/sound"){
      int val = m.getArgAsInt(0);
      enableSound = !enableSound;
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
}

void ofApp::createAgents() {
  // Create Amay & Azra
  Amay *a = new Amay(box2d, agentProps);
  Azra *b = new Azra(box2d, agentProps);
  
  // Set partners
  a->partner = b;
  b->partner = a;
  
  // Push agents in the array.
  agents.push_back(a);
  agents.push_back(b);
}

void ofApp::setupGui() {
    gui.setup();
    settings.setName("Inter Mesh Settings");
  
    // Mesh parameters.
    meshParams.setName("Mesh Params");
    meshParams.add(meshRows.set("Mesh Rows", 5, 5, 100)); // Add the current value
    meshParams.add(meshColumns.set("Mesh Columns", 5, 5, 100));
    meshParams.add(meshWidth.set("Mesh Width", 100, 10, ofGetWidth()/2));
    meshParams.add(meshHeight.set("Mesh Height", 100, 10, ofGetHeight()/2));
  
    // Vertex parameters
    vertexParams.setName("Vertex Params");
    vertexParams.add(vertexRadius.set("Vertex Radius", 6, 1, 30));
    vertexParams.add(vertexDensity.set("Vertex Density", 1, 0, 5));
    vertexParams.add(vertexBounce.set("Vertex Bounce", 0.3, 0, 1));
    vertexParams.add(vertexFriction.set("Vertex Friction", 1, 0, 1));
  
    // Joint parameters
    jointParams.setName("Joint Params");
    jointParams.add(jointFrequency.set("Joint Frequency", 2.0f, 0.0f, 20.0f));
    jointParams.add(jointDamping.set("Joint Damping", 1.0f, 0.0f, 5.0f));
  
    // InterAgentJoint parameters
    interAgentJointParams.setName("InterAgentJoint Params");
    interAgentJointParams.add(frequency.set("Joint Frequency", 2.0f, 0.0f, 20.0f));
    interAgentJointParams.add(damping.set("Joint Damping", 1.0f, 0.0f, 10.0f));
    interAgentJointParams.add(maxJointForce.set("Max Joint Force", 6.f, 1.f, 100.0f));
  
    // Background group
    bgParams.setName("Background Params");
    bgParams.add(rectWidth.set("Width", 20, 10, 50));
    bgParams.add(rectHeight.set("Height", 20, 10, 50));
    bgParams.add(attraction.set("Attraction", 20, -200, 200));
    bgParams.add(repulsion.set("Repulsion", -20, -200, 200));
    bgParams.add(shaderScale.set("Scale", 1.f, 0.f, 10.f));
    rectWidth.addListener(this, &ofApp::widthChanged);
    rectHeight.addListener(this, &ofApp::heightChanged);
    attraction.addListener(this, &ofApp::updateForce);
    repulsion.addListener(this, &ofApp::updateForce);
    shaderScale.addListener(this, &ofApp::updateParams);
  
    settings.add(meshParams);
    settings.add(vertexParams);
    settings.add(jointParams);
    settings.add(interAgentJointParams);
    settings.add(bgParams);
  
    gui.setup(settings);
    gui.loadFromFile("InterMesh.xml");
}

void ofApp::clearScreen() {
  // [WARNING] For some reason, these events are still fired when trying to clean things as one could be in the
    // middle of a step function. Disabling and renabling the events work as a good solution for now.
  box2d.disableEvents();
  collidingBodies.clear();

  // Clear SuperAgents
  for (auto &sa : superAgents) {
    sa.clean(box2d);
  }
  superAgents.clear();

  // Clean agents
  for (auto &a : agents) {
    a -> clean(box2d);
    delete a;
  }
  agents.clear();

  box2d.enableEvents();
}

void ofApp::removeUnbonded() {
  ofRemove(agents, [&](Agent *a) {
//    if (a->getPartner() == NULL) {
//      a->clean(box2d);
//      return true;
//    }
    
    return false;
  });
}

void ofApp::removeJoints() {
  box2d.disableEvents();

  // Clear superAgents only
  for (auto &sa : superAgents) {
    sa.clean(box2d);
  }
  superAgents.clear();

  superAgents.clear();
  box2d.enableEvents();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){  
  if (key == 'd') {
    debug = !debug;
  }
  
  if (key == 'n') {
    createAgents(); 
  }
  
  if (key == 'c') {
    clearScreen();
  }
  
  if (key == 'j') {
    removeJoints();
  }
  
  if (key == 'h') {
    hideGui = !hideGui;
  }
  
  if (key == 'f') {
    // Apply a random force
    for (auto &a: agents) {
      a -> setTickle(1.0);
    }
  }
  
  if (key == 's') {
    enableSound = !enableSound;
  }
  
  if (key == ' ') {
    stopEverything = !stopEverything;
  }
  
  if (key == 't') {
    showTexture = !showTexture; 
  }
}

void ofApp::exit() {
  box2d.disableEvents();
  gui.saveToFile("InterMesh.xml");
}

// Massive important function that determines when the 2 bodies actually bond.
void ofApp::evaluateBonding(b2Body *bodyA, b2Body *bodyB, Agent *agentA, Agent *agentB) {
  collidingBodies.clear();
  
  // Vertex level checks. Is this vertex bonded to anything except itself?
  bool a = canVertexBond(bodyA, agentA);
  bool b = canVertexBond(bodyB, agentB);
  if (a && b) {
    // Prepare for bond.
    collidingBodies.push_back(bodyA);
    collidingBodies.push_back(bodyB);
  }
}

bool ofApp::canVertexBond(b2Body* body, Agent *curAgent) {
  // If it joins anything except itself, then it cannot join.
  auto curEdge = body->GetJointList();
  // Traverse the joint doubly linked list.
  while (curEdge) {
    // Other agent that this joint is joined to.
    auto data = reinterpret_cast<VertexData*>(curEdge->other->GetUserData());
    if (data != NULL) {
      auto otherAgent = data->agent;
      if (otherAgent != curAgent) {
        return false;
      }
    }
    curEdge = curEdge->next;
  }

  return true;
}

void ofApp::widthChanged (int & newWidth) {
  // New background
  bg.setParams(bgParams);
  bg.createBg();
}

void ofApp::heightChanged (int & newHeight) {
  // New background
  bg.setParams(bgParams);
  bg.createBg();
}

void ofApp::updateParams(float & newVal) {
  bg.setParams(bgParams);
}

void ofApp::updateForce(int & newVal) {
  bg.setParams(bgParams);
}


void ofApp::createSuperAgents() {
  // Joint creation based on when two bodies collide at certain vertices.
  if (collidingBodies.size()>0) {
      // Find the agent of this body.
      auto agentA = reinterpret_cast<VertexData*>(collidingBodies[0]->GetUserData())->agent;
      auto agentB = reinterpret_cast<VertexData*>(collidingBodies[1]->GetUserData())->agent;
    
      // If both the agents have that state, then they'll bond.
      SuperAgent superAgent; bool found = false;
      std::shared_ptr<ofxBox2dJoint> j;
      // Check for existing joints.
      for (auto &sa : superAgents) {
        if (sa.contains(agentA, agentB)) {
          j = createInterAgentJoint(collidingBodies[0], collidingBodies[1]);
          sa.joints.push_back(j);
          found = true;
        }
      }
    
      if (!found) {
        j = createInterAgentJoint(collidingBodies[0], collidingBodies[1]);
        superAgent.setup(agentA, agentB, j); // Create a new super agent.
        superAgents.push_back(superAgent);
      }
    
      collidingBodies.clear();
  }
}

std::shared_ptr<ofxBox2dJoint> ofApp::createInterAgentJoint(b2Body *bodyA, b2Body *bodyB) {
    auto j = std::make_shared<ofxBox2dJoint>();
    float f = ofRandom(0.3, frequency);
    float d = ofRandom(1, damping);
    j->setup(box2d.getWorld(), bodyA, bodyB, f, d); // Use the interAgentJoint props.
  
    // Joint length
    int jointLength = ofRandom(250, 300);
    j->setLength(jointLength);
  
    // Enable interAgentJoint
    auto data = reinterpret_cast<VertexData*>(bodyA->GetUserData());
    data->hasInterAgentJoint = true;
    bodyA->SetUserData(data);
  
    data = reinterpret_cast<VertexData*>(bodyB->GetUserData());
    data->hasInterAgentJoint = true;
    bodyB->SetUserData(data);
  
    return j;
}

void ofApp::handleSerial() {
  while (serial.available() > 0)
    {
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
        
        }
    }
}
