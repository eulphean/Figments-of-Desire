#include "Midi.h"

void Midi::setup() {
  // MIDI setup.
  midiOut.openVirtualPort("ofxMidiOut"); // open a virtual port
  bondMakeChannel = 2;
  bondBreakChannel = 3;
}

// ROtary controls (Midi)
void Midi::sendBondMakeMidi(int midiNote) {
  // Constant velocity
  midiOut.sendNoteOn(bondMakeChannel, midiNote, 64);
}

void Midi::sendBondBreakMidi(int midiNote) {
  // Constant velocity
  midiOut.sendNoteOn(bondBreakChannel, midiNote, 64);
  //midiOut.sendNoteOff(bondMakeChannel, midiNote, 64);
}


void Midi::exit() {
  midiOut.closePort();
}

Midi &Midi::instance() {
  return m;
}

// For a static class, variable needs to be
// initialized in the implementation file.
Midi Midi::m;
