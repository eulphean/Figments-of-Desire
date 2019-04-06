#include "Agent.h"

void Agent::setup(ofxBox2d &box2d, AgentProperties agentProps, string fileName) {
  font.load("opensansbold.ttf", 25);
  
  // Prepare the agent's texture.
  readFile(fileName);
  assignMessages(agentProps.meshSize);
  
  
  // Initialize the iterator.
  curMsg = messages.begin(); // Need the message to draw
  
  createTexture(agentProps.meshSize);
  
  // Prepare agent's mesh.
  createMesh(agentProps);
  createSoftBody(box2d, agentProps);
  
  // Calculate a desireRadius based on the size of the mesh
  auto area = agentProps.meshSize.x * agentProps.meshSize.y;
  desireRadius = sqrt(area/PI);
  
  // Target position
  seekTargetPos = glm::vec2(ofRandom(150, ofGetWidth() - 200), ofRandom(50 , 250));
  
  // Force weights for various body actions..
  stretchWeight = 1.5;
  repulsionWeight = 0.5;
  attractionWeight = 0.3;
  
  seekWeight = 0.4; // Probably seek with a single vertex. 
  tickleWeight = 2.5;
  
  // These are actions. But, what are the desires?
  applyStretch = true;
  applySeek = false;
  applyTickle = false;
  applyRepulsion = false;
  
  maxInterAgentJoints = ofRandom(1, vertices.size());
}

void Agent::update() {
  // Use box2d circle to update the mesh.
  updateMesh();
  
  // Update desire
  // Any other conditions ? Like when it's stuck, I'll have to come
  // and update this.
  // Update this every frame.
  if (curDesireCounter < maxDesireCounter) {
    curDesireCounter += desireIncrement;
  }
  
  // Check if the two desire radius' intersect.
  // If desire radius intersect, time to apply interpersonal behaviors
  auto d = glm::distance(this->getCentroid(), partner->getCentroid());
  auto maxDistanceForIntersection = (this->desireRadius + partner->desireRadius) * 4/5;
  if (d < maxDistanceForIntersection) {
    // Apply the right behavior for this states
    if (curDesireCounter < 0) {
      applyRepulsion = true;
    } else {
      applyAttraction = true; 
    }
  }
  
  applyBehaviors();
  
  // Inputs from Ableton
  // Choregraphy behaviors should be applied seperately.
}

void Agent::draw(bool debug, bool showTexture) {
  // Draw the meshes.
  // Draw the soft bodies.
  ofPushStyle();
    for(auto v: vertices) {
      ofPushMatrix();
        ofTranslate(v->getPosition());
        ofSetColor(ofColor::red);
        ofFill();
        ofDrawCircle(0, 0, v->getRadius());
      ofPopMatrix();
    }
  ofPopStyle();
  
  if (showTexture) {
   filter->begin();
    fbo.getTexture().bind();
    mesh.draw();
    fbo.getTexture().unbind();
   filter->end();
  } else {
    ofPushStyle();
    for(auto j: joints) {
      ofPushMatrix();
        ofSetColor(ofColor::green);
        j->draw();
      ofPopMatrix();
    }
    ofPopStyle();
  }

  if (debug) {
    auto centroid = mesh.getCentroid();
    ofPushMatrix();
      ofTranslate(centroid);
      ofNoFill();
      ofSetColor(ofColor::white);
      ofDrawCircle(0, 0, desireRadius);
    
      // Write the current desire values for each figment
      ofPushMatrix();
        ofPushStyle();
        ofTranslate(0, -desireRadius);
        ofBitmapFont f;
        auto string = "Current Desire: " + ofToString(curDesireCounter);
        auto rec = f.getBoundingBox(string, 0, 0);
        font.drawString(string, -rec.width, 40);
    
        string = "Max Desire: " + ofToString(maxDesireCounter);
        rec = f.getBoundingBox(string, 0, 0);
        font.drawString(string, -rec.width, 70);
        ofPopStyle();
      ofPopMatrix();
    ofPopMatrix();
  }
}

void Agent::readFile(string fileName) {
  auto buffer = ofBufferFromFile(fileName);
  auto lines = ofSplitString(buffer.getText(), "\n");
 
  for (auto l: lines) {
    auto i = l.find(":");
    if (i > 0) {
      auto s = l.substr(i+1);
      textMsgs.push_back(s);
    } else {
      auto b = textMsgs.back();
      b = b + "\n" + l; // Append the line to the last value in the vector.
      textMsgs[textMsgs.size()-1] = b;
    }
  }
}

ofPoint Agent::getTextureSize() {
  return ofPoint(fbo.getWidth(), fbo.getHeight());
}

void Agent::clean(ofxBox2d &box2d) {
  // Remove joints.
  ofRemove(joints, [&](std::shared_ptr<ofxBox2dJoint> j){
    box2d.getWorld()->DestroyJoint(j->joint);
    return true;
  });
  
  // Remove vertices
  ofRemove(vertices, [&](std::shared_ptr<ofxBox2dCircle> c){
    return true;
  });

  // Clear all.
  joints.clear();
  vertices.clear();
}

void Agent::assignMessages(ofPoint meshSize) {
  // Create Bogus message circles.
  for (int i = 0; i < numBogusMessages; i++) {
    // Pick a random location on the mesh.
    int w = meshSize.x; int h = meshSize.y;
    auto x = ofRandom(0, w); auto y = ofRandom(0, h);
    
    // Pick a random color for the message.
    int idx = ofRandom(1, palette.size());
    ofColor c = ofColor(palette.at(idx));
    
    // Pick a random size (TOOD: Based off on the length of the message).
    int size = ofRandom(20, 40);
    
    // Create a message.
    Message m = Message(glm::vec2(x, y), c, size, "~");
    messages.push_back(m);
  }
}

void Agent::createTexture(ofPoint meshSize) {
  // Create a simple fbo.
  fbo.allocate(meshSize.x, meshSize.y, GL_RGBA);
  fbo.begin();
    ofClear(0, 0, 0, 0);
  
    // Assign background.
    ofColor c = ofColor(palette.at(0), 200);
    ofBackground(c);
  
    // Draw assigned messages.
    for (auto m : messages) {
      m.draw(font);
    }
    //curMsg->draw(font);
    
  fbo.end();
}

void Agent::applyBehaviors() {

  // ----Current actions/behaviors---
  handleStretch();
  handleRepulsion();
  handleAttraction();
  //  handleTickle();
  //  handleSeek();
}

void Agent::handleAttraction() {
  if (applyAttraction) {
    // Don't apply attraction on all the vertices.
    // Algorithm to pick bunch of vertices to apply forces on.
    
    auto v = vertices[0];
    auto pos = glm::vec2(partner->getCentroid().x, partner->getCentroid().y);
    v->addAttractionPoint({pos.x, pos.y}, attractionWeight);
    applyAttraction = false;
  }
}

void Agent::handleRepulsion() {
  if (applyRepulsion) {
    // Don't apply repulsion on all the vertices.
    // Algorithm to pick a good vertex to apply the repulsion on.
    auto v = vertices[0];
    auto pos = glm::vec2(partner->getCentroid().x, partner->getCentroid().y);
    v->addRepulsionForce(pos.x, pos.y, repulsionWeight);
    applyRepulsion = false;
  }
}

void Agent::handleStretch() {
  // Check for counter.
  if (applyStretch) { // Time to apply a stretch.
    for (auto &v : vertices) {
      if (ofRandom(1) < 0.2) {
        v->addAttractionPoint({mesh.getCentroid().x, mesh.getCentroid().y}, stretchWeight);
      } else {
        v->addRepulsionForce(mesh.getCentroid().x, mesh.getCentroid().y, stretchWeight);
      }
      v->setRotation(ofRandom(150));
    }
    applyStretch = false;
  }
}

void Agent::handleSeek() {
  // Don't seek if this agent has a partner.
  if (partner != NULL) {
    return;
  }
  
  // New target? Add an impulse in that direction.
  if (applySeek) {
    vertices[0]->addAttractionPoint(seekTargetPos.x, seekTargetPos.y, seekWeight);
    vertices[0]->setRotation(ofRandom(180, 360));
    
    vertices[vertices.size()-1]->addAttractionPoint(seekTargetPos.x, seekTargetPos.y, seekWeight);
    vertices[vertices.size()-1]->setRotation(ofRandom(180, 360));
    
    applySeek = false;
  }
}

void Agent::handleTickle() {
  // Does the agent want to tickle? Check with counter conditions.
  if (applyTickle == true) {
    // Apply the tickle.
    for (auto &v: vertices) {
      glm::vec2 force = glm::vec2(ofRandom(-5, 5), ofRandom(-5, 5));
      v -> addForce(force, tickleWeight);
    }
    applyTickle = false;
  }
}

int Agent::getMaxInterAgentJoints() {
  return maxInterAgentJoints;
}

glm::vec2 Agent::getCentroid() {
  return mesh.getCentroid();
}

ofMesh& Agent::getMesh() {
  return mesh;
}

void Agent::setSeekTarget() {
  // Pick a new target location once it starts slowing down.
  glm::vec2 avgVel;
  for (auto v : vertices) {
    avgVel += glm::vec2(v->getVelocity().x, v->getVelocity().y);
  }
  avgVel = avgVel/vertices.size();
  
  // Pick a new target when the agent has really slowed down. Is this really what I want?
  if (abs(avgVel.g) < 0.5 && !applySeek) {
    // Calculate a new target position.
    auto x = desireRadius * sin(ofRandom(360)); auto y = desireRadius * cos(ofRandom(360));
    seekTargetPos = glm::vec2(mesh.getCentroid().x, mesh.getCentroid().y) + glm::vec2(x, y);
    applySeek = true;
  }
}

void Agent::setTickle(float avgForceWeight) {
  applyTickle = true;
  tickleWeight = avgForceWeight;
}

void Agent::setStretch(float avgWeight) {
  applyStretch = true;
  stretchWeight = avgWeight;
}

std::shared_ptr<ofxBox2dCircle> Agent::getRandomVertex() {
  int randV = ofRandom(vertices.size());
  auto v = vertices[randV];
  return v;
}

void Agent::createMesh(AgentProperties agentProps) {
  mesh.clear();
  mesh.setMode(OF_PRIMITIVE_TRIANGLES);
  
  // Create a mesh for the grabber.
  int nRows = agentProps.meshDimensions.x;
  int nCols = agentProps.meshDimensions.y;
  
  // Width, height for mapping the correct texture coordinate.
  int w = agentProps.meshSize.x;
  int h = agentProps.meshSize.y;
  
  // Create the mesh.
  for (int y = 0; y < nRows; y++) {
    for (int x = 0; x < nCols; x++) {
      float ix = agentProps.meshOrigin.x + w * x / (nCols - 1);
      float iy = agentProps.meshOrigin.y + h * y / (nRows - 1);
     
      mesh.addVertex({ix, iy, 0});
      
      // Height and Width of the texture is same as the width/height sent in via agentProps
      float texX = ofMap(ix - agentProps.meshOrigin.x, 0, w, 0, 1, true); // Map the calculated x coordinate from 0 - 1
      float texY = ofMap(iy - agentProps.meshOrigin.y, 0, h, 0, 1, true); // Map the calculated y coordinate from 0 - 1
      mesh.addTexCoord({texX, texY});
    }
  }

  // We don't draw the last row / col (nRows - 1 and nCols - 1) because it was
  // taken care of by the row above and column to the left.
  for (int y = 0; y < nRows - 1; y++)
  {
      for (int x = 0; x < nCols - 1; x++)
      {
          // Draw T0
          // P0
          mesh.addIndex((y + 0) * nCols + (x + 0));
          // P1
          mesh.addIndex((y + 0) * nCols + (x + 1));
          // P2
          mesh.addIndex((y + 1) * nCols + (x + 0));

          // Draw T1
          // P1
          mesh.addIndex((y + 0) * nCols + (x + 1));
          // P3
          mesh.addIndex((y + 1) * nCols + (x + 1));
          // P2
          mesh.addIndex((y + 1) * nCols + (x + 0));
      }
  }
}

void Agent::createSoftBody(ofxBox2d &box2d, AgentProperties agentProps) {
  auto meshVertices = mesh.getVertices();
  vertices.clear();
  joints.clear();

  // Create mesh vertices as Box2D elements.
  for (int i = 0; i < meshVertices.size(); i++) {
    auto vertex = std::make_shared<ofxBox2dCircle>();
    vertex -> setPhysics(agentProps.vertexPhysics.x, agentProps.vertexPhysics.y, agentProps.vertexPhysics.z); // bounce, density, friction
    vertex -> setup(box2d.getWorld(), meshVertices[i].x, meshVertices[i].y, agentProps.vertexRadius); // ofRandom(3, agentProps.vertexRadius)
    vertex -> setFixedRotation(true);
    vertex -> setData(new VertexData(this)); // Data is passed with current Agent's pointer
    vertices.push_back(vertex);
  }
  
  int meshRows = agentProps.meshDimensions.x;
  int meshColumns = agentProps.meshDimensions.y;
  
  // Create Box2d joints for the mesh.
  for (int y = 0; y < meshRows; y++) {
    for (int x = 0; x < meshColumns; x++) {
      int idx = x + y * meshColumns;
      
      // Do this for all columns except last column.
      // NOTE: Connect current vertex with the next vertex in the same row.
      if (x != meshColumns - 1) {
        auto joint = std::make_shared<ofxBox2dJoint>();
        int rightIdx = idx + 1;
        joint -> setup(box2d.getWorld(), vertices[idx] -> body, vertices[rightIdx] -> body, agentProps.jointPhysics.x, agentProps.jointPhysics.y); // frequency, damping
        joints.push_back(joint);
      }
      
      
      // Do this for each row except the last row. There is no further joint to
      // be made there.
      if (y != meshRows - 1) {
        auto joint = std::make_shared<ofxBox2dJoint>();
        int downIdx = x + (y + 1) * meshColumns;
        joint -> setup(box2d.getWorld(), vertices[idx] -> body, vertices[downIdx] -> body, agentProps.jointPhysics.x, agentProps.jointPhysics.y);
        joints.push_back(joint);
      }
    }
  }
}

void Agent::updateMesh() {
  auto meshPoints = mesh.getVertices();
  
  for (int j = 0; j < meshPoints.size(); j++) {
    // Get the box2D vertex position.
    glm::vec2 pos = vertices[j] -> getPosition();
    
    // Update mesh point's position with the position of
    // the box2d vertex.
    auto meshPoint = meshPoints[j];
    meshPoint.x = pos.x;
    meshPoint.y = pos.y;
    mesh.setVertex(j, meshPoint);
  }
}
