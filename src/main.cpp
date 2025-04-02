#include <DMXSerial.h>
#include <DmxSimple.h>
#include <USB-MIDI.h>

#define ON_LED_PIN 13

#define DMX_TX_PIN 1
#define DMX_RX_PIN 0

#define MIDI_CHANNEL MIDI_CHANNEL_OMNI

#undef DMXSERIAL_MAX
#define DMXSERIAL_MAX 12

typedef USBMIDI_NAMESPACE::usbMidiTransport _umt;

typedef MIDI_NAMESPACE::MidiInterface<_umt> _mi;

_umt usbMIDI(0);

_mi MIDICoreUSB((_umt &)usbMIDI);

uint8_t prevDmxValues[DMXSERIAL_MAX + 1];

static void onNoteOn(byte channel, byte note, byte velocity)
{
  if (note <= DMXSERIAL_MAX && note > 0)
  {
    DmxSimple.write(note, (velocity - 1) * 2);
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(ON_LED_PIN, OUTPUT);
  digitalWrite(ON_LED_PIN, HIGH);

  MIDICoreUSB.setHandleNoteOn(onNoteOn);
  MIDICoreUSB.begin(MIDI_CHANNEL);

  DMXSerial.init(DMXReceiver);
  DmxSimple.usePin(DMX_TX_PIN);
  DmxSimple.maxChannel(DMXSERIAL_MAX);

  for (int i = 0; i <= DMXSERIAL_MAX; i++)
  {
    prevDmxValues[i] = -1;
  }
}

void loop()
{
  MIDICoreUSB.read();

  for (int channel = 1; channel <= DMXSERIAL_MAX; channel++)
  {
    int dmxValue = DMXSerial.read(channel);

    if (dmxValue != prevDmxValues[channel])
    {
      int midiNote = channel;
      int velocity = dmxValue / 2;

      if (!velocity)
      {
        velocity = 1;
      };

      MIDICoreUSB.sendNoteOn(midiNote, velocity, 1);
      MIDICoreUSB.sendNoteOff(midiNote, 0, 1);

      prevDmxValues[channel] = dmxValue;
    }
  }
}