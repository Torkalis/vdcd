//
//  Copyright (c) 2013-2014 plan44.ch / Lukas Zeller, Zurich, Switzerland
//
//  Author: Lukas Zeller <luz@plan44.ch>
//
//  This file is part of vdcd.
//
//  vdcd is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  vdcd is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with vdcd. If not, see <http://www.gnu.org/licenses/>.
//

// File scope debugging options
// - Set ALWAYS_DEBUG to 1 to enable DBGLOG output even in non-DEBUG builds of this file
#define ALWAYS_DEBUG 0
// - set FOCUSLOGLEVEL to non-zero log level (usually, 5,6, or 7==LOG_DEBUG) to get focus (extensive logging) for this file
//   Note: must be before including "logger.hpp" (or anything that includes "logger.hpp")
#define FOCUSLOGLEVEL 0

#include "enoceanrps.hpp"

#include "buttonbehaviour.hpp"
#include "binaryinputbehaviour.hpp"


using namespace p44;

#pragma mark - EnoceanRPSDevice

EnoceanRPSDevice::EnoceanRPSDevice(EnoceanDeviceContainer *aClassContainerP) :
  inherited(aClassContainerP)
{
}


#pragma mark - EnoceanRpsHandler

EnoceanRpsHandler::EnoceanRpsHandler(EnoceanDevice &aDevice) :
  inherited(aDevice)
{
}


EnoceanDevicePtr EnoceanRpsHandler::newDevice(
  EnoceanDeviceContainer *aClassContainerP,
  EnoceanAddress aAddress,
  EnoceanSubDevice aSubDeviceIndex,
  EnoceanProfile aEEProfile, EnoceanManufacturer aEEManufacturer,
  bool aNeedsTeachInResponse
) {
  EnoceanDevicePtr newDev; // none so far
  EnoceanProfile functionProfile = aEEProfile & eep_ignore_type_mask;
  if (functionProfile==0xF60200 || functionProfile==0xF60300) {
    // F6-02-xx or F6-03-xx: 2 or 4 rocker switch = max 2 or 4 dsDevices
    EnoceanSubDevice numSubDevices = functionProfile==0xF60300 ? 4 : 2;
    if (aSubDeviceIndex<numSubDevices) {
      // create EnoceanRPSDevice device
      newDev = EnoceanDevicePtr(new EnoceanRPSDevice(aClassContainerP));
      // standard device settings without scene table
      newDev->installSettings();
      // assign channel and address
      newDev->setAddressingInfo(aAddress, aSubDeviceIndex);
      // assign EPP information
      newDev->setEEPInfo(aEEProfile, aEEManufacturer);
      newDev->setFunctionDesc("rocker switch");
      // set icon name: even-numbered subdevice is left, odd is right
      newDev->setIconInfo(aSubDeviceIndex & 0x01 ? "enocean_br" : "enocean_bl", true);
      // RPS switches can be used for anything
      newDev->setPrimaryGroup(group_black_joker);
      // Create two handlers, one for the up button, one for the down button
      // - create button input for down key
      EnoceanRpsButtonHandlerPtr downHandler = EnoceanRpsButtonHandlerPtr(new EnoceanRpsButtonHandler(*newDev.get()));
      downHandler->switchIndex = aSubDeviceIndex; // each switch gets its own subdevice
      downHandler->isRockerUp = false;
      ButtonBehaviourPtr downBhvr = ButtonBehaviourPtr(new ButtonBehaviour(*newDev.get()));
      downBhvr->setHardwareButtonConfig(0, buttonType_2way, buttonElement_down, false, 1); // counterpart up-button has index 1
      downBhvr->setGroup(group_yellow_light); // pre-configure for light
      downBhvr->setHardwareName("Down key");
      downHandler->behaviour = downBhvr;
      newDev->addChannelHandler(downHandler);
      // - create button input for up key
      EnoceanRpsButtonHandlerPtr upHandler = EnoceanRpsButtonHandlerPtr(new EnoceanRpsButtonHandler(*newDev.get()));
      upHandler->switchIndex = aSubDeviceIndex; // each switch gets its own subdevice
      upHandler->isRockerUp = true;
      ButtonBehaviourPtr upBhvr = ButtonBehaviourPtr(new ButtonBehaviour(*newDev.get()));
      upBhvr->setGroup(group_yellow_light); // pre-configure for light
      upBhvr->setHardwareButtonConfig(0, buttonType_2way, buttonElement_up, false, 0); // counterpart down-button has index 0
      upBhvr->setHardwareName("Up key");
      upHandler->behaviour = upBhvr;
      newDev->addChannelHandler(upHandler);
    }
  }
  else if (functionProfile==0xF61000) {
    // F6-10-00 : Window handle = single device
    if (aSubDeviceIndex<1) {
      // create EnoceanRPSDevice device
      newDev = EnoceanDevicePtr(new EnoceanRPSDevice(aClassContainerP));
      // standard device settings without scene table
      newDev->installSettings();
      // assign channel and address
      newDev->setAddressingInfo(aAddress, aSubDeviceIndex);
      // assign EPP information
      newDev->setEEPInfo(aEEProfile, aEEManufacturer);
      newDev->setFunctionDesc("window handle");
      // Window handle switches can be used for anything
      newDev->setPrimaryGroup(group_black_joker);
      // Current simple dS mapping: two binary inputs
      // - Input0: 0: Window closed (Handle down position), 1: Window open (all other handle positions)
      EnoceanRpsWindowHandleHandlerPtr newHandler = EnoceanRpsWindowHandleHandlerPtr(new EnoceanRpsWindowHandleHandler(*newDev.get()));
      BinaryInputBehaviourPtr bb = BinaryInputBehaviourPtr(new BinaryInputBehaviour(*newDev.get()));
      bb->setHardwareInputConfig(binInpType_none, usage_undefined, true, Never);
      bb->setGroup(group_black_joker); // joker by default
      bb->setHardwareName("Window open");
      newHandler->isTiltedStatus = false;
      newHandler->behaviour = bb;
      newDev->addChannelHandler(newHandler);
      // - Input1: 0: Window fully open (Handle horizontal left or right), 1: Window tilted (Handle up position)
      newHandler = EnoceanRpsWindowHandleHandlerPtr(new EnoceanRpsWindowHandleHandler(*newDev.get()));
      bb = BinaryInputBehaviourPtr(new BinaryInputBehaviour(*newDev.get()));
      bb->setHardwareInputConfig(binInpType_none, usage_undefined, true, Never);
      bb->setGroup(group_black_joker); // joker by default
      bb->setHardwareName("Window tilted");
      newHandler->isTiltedStatus = true;
      newHandler->behaviour = bb;
      newDev->addChannelHandler(newHandler);
    }
  }
  else if (functionProfile==0xF60400) {
    // F6-04-01 and F6-04-02 : key card activated switch = single device
    if (aSubDeviceIndex<1) {
      // create EnoceanRPSDevice device
      newDev = EnoceanDevicePtr(new EnoceanRPSDevice(aClassContainerP));
      // standard device settings without scene table
      newDev->installSettings();
      // assign channel and address
      newDev->setAddressingInfo(aAddress, aSubDeviceIndex);
      // assign EPP information
      newDev->setEEPInfo(aEEProfile, aEEManufacturer);
      newDev->setFunctionDesc("key card switch");
      // key card switches can be used for anything
      newDev->setPrimaryGroup(group_black_joker);
      // Current simple dS mapping: one binary input
      // - 1: card inserted, 0: card extracted
      EnoceanRpsCardKeyHandlerPtr newHandler = EnoceanRpsCardKeyHandlerPtr(new EnoceanRpsCardKeyHandler(*newDev.get()));
      BinaryInputBehaviourPtr bb = BinaryInputBehaviourPtr(new BinaryInputBehaviour(*newDev.get()));
      bb->setHardwareInputConfig(binInpType_none, usage_undefined, true, Never);
      bb->setGroup(group_black_joker); // joker by default
      bb->setHardwareName("card inserted");
      newHandler->behaviour = bb;
      newDev->addChannelHandler(newHandler);
    }
  }
  else if (aEEProfile==0xF605C0) {
    // F6-05-xx - EEP for "detectors"
    // F6-05-C0 - custom pseudo-EEP for not yet defined smoke alarm profile (Eltako FRW and alphaEOS GUARD)
    if (aSubDeviceIndex<1) {
      // create EnoceanRPSDevice device
      newDev = EnoceanDevicePtr(new EnoceanRPSDevice(aClassContainerP));
      // standard device settings without scene table
      newDev->installSettings();
      // assign channel and address
      newDev->setAddressingInfo(aAddress, aSubDeviceIndex);
      // assign EPP information
      newDev->setEEPInfo(aEEProfile, aEEManufacturer);
      newDev->setFunctionDesc("smoke detector");
      // smoke detectors can be used for anything
      newDev->setPrimaryGroup(group_black_joker);
      // Current simple dS mapping: one binary input for smoke alarm status, one for low bat status
      EnoceanRpsSmokeDetectorHandlerPtr newHandler;
      BinaryInputBehaviourPtr bb;
      // - Alarm: 1: Alarm, 0: no Alarm
      newHandler = EnoceanRpsSmokeDetectorHandlerPtr(new EnoceanRpsSmokeDetectorHandler(*newDev.get()));
      bb = BinaryInputBehaviourPtr(new BinaryInputBehaviour(*newDev.get()));
      bb->setHardwareInputConfig(binInpType_smoke, usage_room, true, Never);
      bb->setGroup(group_black_joker); // joker by default
      bb->setHardwareName("Smoke alarm");
      newHandler->behaviour = bb;
      newHandler->isBatteryStatus = false;
      newDev->addChannelHandler(newHandler);
      // - Low Battery: 1: battery low, 0: battery OK
      newHandler = EnoceanRpsSmokeDetectorHandlerPtr(new EnoceanRpsSmokeDetectorHandler(*newDev.get()));
      bb = BinaryInputBehaviourPtr(new BinaryInputBehaviour(*newDev.get()));
      bb->setHardwareInputConfig(binInpType_lowBattery, usage_room, true, Never);
      bb->setGroup(group_black_joker); // joker by default
      bb->setHardwareName("Low Battery");
      newHandler->behaviour = bb;
      newHandler->isBatteryStatus = true;
      newDev->addChannelHandler(newHandler);
    }
  }
  // RPS never needs a teach-in response
  // return device (or empty if none created)
  return newDev;
}


#pragma mark - button

EnoceanRpsButtonHandler::EnoceanRpsButtonHandler(EnoceanDevice &aDevice) :
  inherited(aDevice)
{
  switchIndex = 0; // default to first
  pressed = false;
}


// device specific radio packet handling
void EnoceanRpsButtonHandler::handleRadioPacket(Esp3PacketPtr aEsp3PacketPtr)
{
  // extract payload data
  uint8_t data = aEsp3PacketPtr->radioUserData()[0];
  uint8_t status = aEsp3PacketPtr->radioStatus();
  LOG(LOG_INFO, "RPS message: data=0x%02X, status=0x%02X, processing in %s (switchIndex=%d, isRockerUp=%d)\n", data, status, device.shortDesc().c_str(), switchIndex, isRockerUp);
  // decode
  if (status & status_NU) {
    // N-Message
    FOCUSLOG("- N-message\n");
    // collect action(s)
    for (int ai=1; ai>=0; ai--) {
      // first action is in DB7..5, second action is in DB3..1 (if DB0==1)
      uint8_t a = (data >> (4*ai+1)) & 0x07;
      if (ai==0 && (data&0x01)==0)
        break; // no second action
      FOCUSLOG("- action #%d = %d\n", 2-ai, a);
      if (((a>>1) & 0x03)==switchIndex) {
        // querying this subdevice/rocker
        FOCUSLOG("- is my switchIndex == %d\n", switchIndex);
        if (((a & 0x01)!=0) == isRockerUp) {
          FOCUSLOG("- is my side (%s) of the switch, isRockerUp == %d\n", isRockerUp ? "Up" : "Down", isRockerUp);
          // my half of the rocker, DB4 is button state (1=pressed, 0=released)
          setButtonState((data & 0x10)!=0);
        }
      }
    }
  }
  else {
    // U-Message
    FOCUSLOG("- U-message\n");
    uint8_t b = (data>>5) & 0x07;
    bool pressed = (data & 0x10);
    FOCUSLOG("- number of buttons still pressed code = %d, action (energy bow) = %s\n", b, pressed ? "PRESSED" : "RELEASED");
    if (!pressed) {
      // report release if all buttons are released now
      if (b==0) {
        // all buttons released, this includes this button
        FOCUSLOG("- released multiple buttons, report RELEASED for all\n");
        setButtonState(false);
      }
    }
    // ignore everything else (more that 2 press actions simultaneously)
  }
}


void EnoceanRpsButtonHandler::setButtonState(bool aPressed)
{
  // only propagate real changes
  if (aPressed!=pressed) {
    // real change, propagate to behaviour
    ButtonBehaviourPtr b = boost::dynamic_pointer_cast<ButtonBehaviour>(behaviour);
    if (b) {
      LOG(LOG_INFO,"Enocean Button %s - %08X, subDevice %d, channel %d: changed state to %s\n", b->getHardwareName().c_str(), device.getAddress(), device.getSubDevice(), channel, aPressed ? "PRESSED" : "RELEASED");
      b->buttonAction(aPressed);
    }
    // update cached status
    pressed = aPressed;
  }
}



string EnoceanRpsButtonHandler::shortDesc()
{
  return "Pushbutton";
}


#pragma mark - window handle


EnoceanRpsWindowHandleHandler::EnoceanRpsWindowHandleHandler(EnoceanDevice &aDevice) :
  inherited(aDevice)
{
  isTiltedStatus = false; // default to "open" status
}





// device specific radio packet handling
void EnoceanRpsWindowHandleHandler::handleRadioPacket(Esp3PacketPtr aEsp3PacketPtr)
{
  // extract payload data
  uint8_t data = aEsp3PacketPtr->radioUserData()[0];
  uint8_t status = aEsp3PacketPtr->radioStatus();
  // decode
  if ((status & status_NU)==0 && (status & status_T21)!=0) {
    // Valid window handle status change message
    // extract status
    bool tilted =
      (data & 0xF0)==0xD0; // turned up from sideways
    bool closed =
      (data & 0xF0)==0xF0; // turned down from sideways
    // report data for this binary input
    BinaryInputBehaviourPtr bb = boost::dynamic_pointer_cast<BinaryInputBehaviour>(behaviour);
    if (bb) {
      if (isTiltedStatus) {
        LOG(LOG_INFO,"Enocean Window Handle %08X reports state: %s\n", device.getAddress(), closed ? "closed" : (tilted ? "tilted open" : "fully open"));
        bb->updateInputState(tilted); // report the tilted status
      }
      else {
        bb->updateInputState(!closed); // report the open/close status
      }
    }
  }
}


string EnoceanRpsWindowHandleHandler::shortDesc()
{
  return "Window Handle";
}


#pragma mark - key card switch


EnoceanRpsCardKeyHandler::EnoceanRpsCardKeyHandler(EnoceanDevice &aDevice) :
  inherited(aDevice)
{
}





// device specific radio packet handling
void EnoceanRpsCardKeyHandler::handleRadioPacket(Esp3PacketPtr aEsp3PacketPtr)
{
  bool isInserted = false;
  // extract payload data
  uint8_t data = aEsp3PacketPtr->radioUserData()[0];
  if (device.getEEProfile()==0xF60402) {
    // Key Card Activated Switch ERP2
    // - just evaluate DB0.2 "State of card"
    isInserted = data & 0x04; // Bit2
  }
  else {
    // Asssume ERP1 Key Card Activated Switch
    uint8_t status = aEsp3PacketPtr->radioStatus();
    isInserted = (status & status_NU)!=0 && data==0x70;
  }
  // report data for this binary input
  BinaryInputBehaviourPtr bb = boost::dynamic_pointer_cast<BinaryInputBehaviour>(behaviour);
  if (bb) {
    LOG(LOG_INFO,"Enocean Key Card Switch %08X reports state: %s\n", device.getAddress(), isInserted ? "inserted" : "extracted");
    bb->updateInputState(isInserted); // report the status
  }
}


string EnoceanRpsCardKeyHandler::shortDesc()
{
  return "Key Card Switch";
}


#pragma mark - Smoke Detector

EnoceanRpsSmokeDetectorHandler::EnoceanRpsSmokeDetectorHandler(EnoceanDevice &aDevice) :
inherited(aDevice)
{
}



// AlphaEOS GUARD + Eltako FRW
//                          DATA 	STATUS
//  Alarm - Ein             10		30
//  Alarm ­ Aus              00 		20
//  Batterie - ok 7.5 - 9V 	00 		20
//  Batterie - fail (<7.5V) 30 		30


// device specific radio packet handling
void EnoceanRpsSmokeDetectorHandler::handleRadioPacket(Esp3PacketPtr aEsp3PacketPtr)
{
  // extract payload data
  uint8_t data = aEsp3PacketPtr->radioUserData()[0];
  BinaryInputBehaviourPtr bb = boost::dynamic_pointer_cast<BinaryInputBehaviour>(behaviour);
  if (isBatteryStatus) {
    // battery status channel
    bool lowBat = (data & 0x30)==0x30;
    LOG(LOG_INFO,"Enocean Smoke Detector %08X reports state: Battery %s\n", device.getAddress(), lowBat ? "LOW" : "ok");
    bb->updateInputState(lowBat);
  }
  else {
    // smoke alarm status
    bool smokeAlarm = (data & 0x30)==0x10;
    LOG(LOG_INFO,"Enocean Smoke Detector %08X reports state: %s\n", device.getAddress(), smokeAlarm ? "SMOKE ALARM" : "no alarm");
    bb->updateInputState(smokeAlarm);
  }
}


string EnoceanRpsSmokeDetectorHandler::shortDesc()
{
  return "Smoke Detector";
}





#pragma mark - EnoceanRPSDevice


bool EnoceanRPSDevice::getProfileVariants(ApiValuePtr aApiObjectValue)
{
  // F6-02-xx, F6-04-01, F6-04-02, F6-05-C0 are interchangeable
  if (
    (getEEProfile() & eep_ignore_type_mask)==0xF60200 || // dual rocker
    getEEProfile()==0xF60401 || getEEProfile()==0xF60402 || // key card switch
    getEEProfile()==0xF605C0 // smoke detector Eltako FRW or alphaEOS GUARD
  ) {
    // two-way rocker, key card switch, smoke detector
    aApiObjectValue->add(string_format("%d",0xF602FF), aApiObjectValue->newString("dual rocker switch"));
    aApiObjectValue->add(string_format("%d",0xF60401), aApiObjectValue->newString("key card activated switch"));
    aApiObjectValue->add(string_format("%d",0xF60402), aApiObjectValue->newString("key card activated switch ERP2"));
    aApiObjectValue->add(string_format("%d",0xF605C0), aApiObjectValue->newString("Smoke detector FRW/GUARD"));
    return true;
  }
  return false; // no variants
}


bool EnoceanRPSDevice::setProfileVariant(EnoceanProfile aProfile)
{
  // check if changeable profile code
  if (aProfile==0xF602FF || aProfile==0xF60401 || aProfile==0xF60402 || aProfile==0xF605C0) {
    if (aProfile==getEEProfile()) return true; // we already have that profile -> NOP
    // change profile now
    switchToProfile(aProfile);
    return true;
  }
  return false; // invalid profile
}
