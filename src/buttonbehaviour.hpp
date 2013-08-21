//
//  buttonbehaviour.hpp
//  vdcd
//
//  Created by Lukas Zeller on 22.05.13.
//  Copyright (c) 2013 plan44.ch. All rights reserved.
//

#ifndef __vdcd__buttonbehaviour__
#define __vdcd__buttonbehaviour__

#include "device.hpp"

using namespace std;

namespace p44 {


  /// the persistent parameters of a device with light behaviour
  class ButtonSettings : public DsBehaviourSettings
  {
    typedef DsBehaviourSettings inherited;

  public:
    DsButtonMode buttonMode; ///< the button mode (LTMODE)
    DsButtonFunc buttonFunc; ///< the button function (LTNUM)
    DsGroup buttonGroup; ///< the group/color of the button
    bool setsLocalPriority; ///< button should set local priority
    bool callsPresent; ///< button should call "present" scene

    ButtonSettings(ParamStore &aParamStore, DsBehaviour &aBehaviour);

    /// @name PersistentParams methods which implement actual storage
    /// @{

    /// SQLIte3 table name to store these parameters to
    virtual const char *tableName();
    /// data field definitions
    virtual const FieldDefinition *getFieldDefs();
    /// load values from passed row
    virtual void loadFromRow(sqlite3pp::query::iterator &aRow, int &aIndex);
    /// bind values to passed statement
    virtual void bindToStatement(sqlite3pp::statement &aStatement, int &aIndex, const char *aParentIdentifier);
    
    /// @}
  };




  class ButtonBehaviour : public DsBehaviour
  {
    typedef DsBehaviour inherited;

    friend class Device;

  protected:

    /// @name button behaviour description
    ///   fixed constants or values set ONCE by device implementations when adding a ButtonBehaviour.
    ///   These need to remain constant after device initialisation is complete!
    /// @{

    virtual BehaviourType getType() { return behaviour_button; };

    bool supportsLocalKeyMode; ///< set if this button can act as local button
    int buttonID; ///< the ID grouping all inputs of a hardware button (which can have multiple elements)
    DsButtonType buttonType; ///< type of button
    DsButtonElement buttonElementID; ///< identifies element of a multi-input button hardware-button

    /// @}

    /// @name Button behaviour settings
    /// @{

    /// the button's persistent settings
    ButtonSettings buttonSettings;

    /// @}


    /// @name button behaviour state
    /// @{

    /// set if button is currently pressed
    bool buttonPressed;

    /// set to last click type of button
    DsClickType clickType;

    /// @}

  public:
  
    /// constructor
    ButtonBehaviour(Device &aDevice, size_t aIndex);

    /// initialisation of hardware-specific constants for this button input
    /// @note this must be called once before the device gets added to the device container. Implementation might
    ///   also derive default values for settings from this information.
    void setHardwareButtonType(int aButtonID, DsButtonType aType, DsButtonElement aElement, bool aSupportsLocalKeyMode);


    /// @name interface towards actual device hardware (or simulation)
    /// @{

    /// button action occurred
    /// @param aPressed true if action is button pressed, false if action is button released
    void buttonAction(bool aPressed);

    /// @}


    /// @name persistent settings management
    /// @{

    /// load behaviour settings from persistent DB
    /// @note this is usually called from the device container when device is added (detected)
    virtual ErrorPtr load();

    /// save unsaved behaviour settings to persistent DB
    /// @note this is usually called from the device container in regular intervals
    virtual ErrorPtr save();

    /// forget any behaviour settings stored in persistent DB
    virtual ErrorPtr forget();
    
    /// @}
    
    /// @return button element that defines the function of this button in local operation modes
    DsButtonElement localFunctionElement();

    /// description of object, mainly for debug and logging
    /// @return textual description of object, may contain LFs
    virtual string description();

  protected:

    // property access implementation for descriptor/settings/states
    virtual int numDescProps();
    virtual const PropertyDescriptor *getDescDescriptor(int aPropIndex);
    virtual int numSettingsProps();
    virtual const PropertyDescriptor *getSettingsDescriptor(int aPropIndex);
    virtual int numStateProps();
    virtual const PropertyDescriptor *getStateDescriptor(int aPropIndex);

    virtual bool accessField(bool aForWrite, JsonObjectPtr &aPropValue, const PropertyDescriptor &aPropertyDescriptor, int aIndex);

  private:

    /// @name button state machine v2.01
    /// @{

    /// button states
    typedef enum {
      S0_idle,
      S1_initialpress,
      S2_holdOrTip,
      S3_hold,
      S4_nextTipWait,
      S5_nextPauseWait,
      S6_2ClickWait,
      S7_progModeWait,
      S8_awaitrelease,
      S9_2pauseWait,
      // S10 missing
      S11_localdim,
      S12_3clickWait,
      S13_3pauseWait,
      S14_awaitrelease, // duplicate of S8
    } ButtonState;

    // state machine vars
    ButtonState state;
    int clickCounter;
    int holdRepeats;
    bool outputOn;
    bool localButtonEnabled;
    bool dimmingUp;
    MLMicroSeconds timerRef;
    bool timerPending;

    // state machine params
    static const int t_long_function_delay = 500*MilliSecond;
    static const int t_dim_repeat_time = 1000*MilliSecond;
    static const int t_click_length = 140*MilliSecond;
    static const int t_click_pause = 140*MilliSecond;
    static const int t_tip_timeout = 800*MilliSecond;
    static const int t_local_dim_timeout = 160*MilliSecond;
    static const int max_hold_repeats = 30;

    // methods
    void resetStateMachine();
    void checkStateMachine(bool aButtonChange, MLMicroSeconds aNow);
    void checkTimer(MLMicroSeconds aCycleStartTime);
    void localSwitchOutput();
    void localDim();
    void sendClick(DsClickType aClickType);
    
    /// @}

  };

  typedef boost::shared_ptr<ButtonBehaviour> ButtonBehaviourPtr;

}


#endif /* defined(__vdcd__buttonbehaviour__) */
