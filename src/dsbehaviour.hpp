//
//  dsbehaviour.h
//  vdcd
//
//  Created by Lukas Zeller on 20.08.13.
//  Copyright (c) 2013 plan44.ch. All rights reserved.
//

#ifndef __vdcd__dsbehaviour__
#define __vdcd__dsbehaviour__

#include "deviceclasscontainer.hpp"

#include "dsid.hpp"
#include "dsdefs.h"

using namespace std;

namespace p44 {

  typedef uint8_t Brightness;
  typedef uint8_t SceneNo;

  // offset to differentiate property keys for descriptions, settings and states
  enum {
    descriptions_key_offset = 1000,
    settings_key_offset = 2000,
    states_key_offset = 3000
  };


  typedef enum {
    behaviour_undefined,
    behaviour_button,
    behaviour_binaryinput,
    behaviour_output,
    behaviour_sensor
  } BehaviourType;


  class Device;
  class DsBehaviour;


  /// DsBehaviourSettings wraps the settings for a behaviour
  class DsBehaviourSettings : public PersistentParams
  {
    typedef PersistentParams inherited;

    DsBehaviour &behaviour;

  public:
    DsBehaviourSettings(ParamStore &aParamStore, DsBehaviour &aBehaviour);

    /// load behaviour settings from store
    ErrorPtr load();
    /// save behaviour settings to store
    ErrorPtr save();

  private:

    string getDbKey();
  };



  /// a DsBehaviour represents and implements a device behaviour according to dS specs
  /// (for example: the dS Light state machine). The interface of a DsBehaviour is generic
  /// such that it can be used by different physical implementations (e.g. both DALI devices
  /// and hue devices will make use of the dS light state machine behaviour.
  class DsBehaviour : public PropertyContainer
  {
    typedef PropertyContainer inherited;
    friend class DsBehaviourSettings;

  protected:

    /// the device this behaviour belongs to
    Device &device;

    /// the index of this behaviour in the device's vector
    size_t index;

  protected:

    /// @name behaviour description, constants or variables
    ///   set by device implementations when adding a Behaviour.
    /// @{

    /// type of behaviour
    virtual BehaviourType getType() { return behaviour_undefined; };

    /// name that identifies this behaviour among others for the human user (terminal label text etc)
    string hardwareName;

    /// @}


  public:
    DsBehaviour(Device &aDevice, size_t aIndex);
    virtual ~DsBehaviour();

    /// initialisation of hardware-specific constants for this button input
    /// @param aHardwareName name to identify this functionality in hardware (like input terminal label, button label or kind etc.)
    /// @note this must be called once before the device gets added to the device container.
    void setHardwareName(const string &aHardwareName) { hardwareName = aHardwareName; };

    /// @name persistent settings management
    /// @{

    /// load behaviour parameters from persistent DB
    virtual ErrorPtr load() { return ErrorPtr(); /* NOP in base class */ };

    /// save unsaved behaviour parameters to persistent DB
    virtual ErrorPtr save() { return ErrorPtr(); /* NOP in base class */ };

    /// forget any parameters stored in persistent DB
    virtual ErrorPtr forget() { return ErrorPtr(); /* NOP in base class */ };

    /// @}

    /// textual representation of getType()
    /// @return type string
    const char *getTypeName();

    /// description of object, mainly for debug and logging
    /// @return textual description of object, may contain LFs
    virtual string description() { return ""; /* empty string, to allow chaining descriptions for behaviour hierarchies */ };

    /// short (text without LFs!) description of object, mainly for referencing it in log messages
    /// @return textual description of object
    virtual string shortDesc() { return getTypeName(); }

  protected:

    /// @name property access implementation for descriptor/settings/states
    /// @{

    /// @return number of description (readonly) properties
    virtual int numDescProps() { return 0; };

    /// @param aPropIndex the description property index
    /// @return description (readonly) property descriptor
    virtual const PropertyDescriptor *getDescDescriptor(int aPropIndex) { return NULL; };

    /// @return number of settings (read/write) properties
    virtual int numSettingsProps() { return 0; };

    /// @param aPropIndex the settings property index
    /// @return settings (read/write) property descriptor
    virtual const PropertyDescriptor *getSettingsDescriptor(int aPropIndex) { return NULL; };

    /// @return number of states (read/write) properties
    virtual int numStateProps() { return 0; };

    /// @param aPropIndex the states property index
    /// @return states (read/write) property descriptor
    virtual const PropertyDescriptor *getStateDescriptor(int aPropIndex) { return NULL; };


    /// access single field in this behaviour
    /// @param aForWrite false for reading, true for writing
    /// @param aPropertyDescriptor decriptor for a single value field/array in this behaviour.
    /// @param aPropValue JsonObject with a single value
    /// @param aIndex in case of array, the index of the element to access
    /// @return false if value could not be accessed
    virtual bool accessField(bool aForWrite, JsonObjectPtr &aPropValue, const PropertyDescriptor &aPropertyDescriptor, int aIndex);

    /// @}

  private:

    // property access basic dispatcher implementation
    virtual int numProps(int aDomain);
    virtual const PropertyDescriptor *getPropertyDescriptor(int aPropIndex, int aDomain);
    int numLocalProps(int aDomain);

  };

  typedef boost::shared_ptr<DsBehaviour> DsBehaviourPtr;


  class OutputBehaviour : public DsBehaviour
  {
    typedef DsBehaviour inherited;

  protected:

    /// @name behaviour description, constants or variables
    ///   set by device implementations when adding a Behaviour.
    /// @{

    virtual BehaviourType getType() { return behaviour_output; };

    /// @}

  public:
    OutputBehaviour(Device &aDevice, size_t aIndex)
    : inherited(aDevice, aIndex) {};

  };

  class BinaryInputBehaviour : public DsBehaviour
  {
    typedef DsBehaviour inherited;

  protected:

    /// @name behaviour description, constants or variables
    ///   set by device implementations when adding a Behaviour.
    /// @{

    virtual BehaviourType getType() { return behaviour_binaryinput; };
    
    /// @}

  public:
    BinaryInputBehaviour(Device &aDevice, size_t aIndex)
    : inherited(aDevice, aIndex) {};
    
  };


  class SensorBehaviour : public DsBehaviour
  {
    typedef DsBehaviour inherited;

  protected:

    /// @name behaviour description, constants or variables
    ///   set by device implementations when adding a Behaviour.
    /// @{

    virtual BehaviourType getType() { return behaviour_sensor; };
    
    /// @}

  public:
    SensorBehaviour(Device &aDevice, size_t aIndex)
    : inherited(aDevice, aIndex) {};
    
  };


} // namespace p44






#endif /* defined(__vdcd__dsbehaviour__) */
