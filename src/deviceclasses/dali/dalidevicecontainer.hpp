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

#ifndef __vdcd__dalidevicecontainer__
#define __vdcd__dalidevicecontainer__

#include "vdcd_common.hpp"

#include "deviceclasscontainer.hpp"

#include "dalicomm.hpp"
#include "dalidevice.hpp"

using namespace std;

namespace p44 {

  class DaliDeviceContainer;
  typedef boost::intrusive_ptr<DaliDeviceContainer> DaliDeviceContainerPtr;

  typedef std::list<DaliBusDevicePtr> DaliBusDeviceList;
  typedef boost::shared_ptr<DaliBusDeviceList> DaliBusDeviceListPtr;


  /// persistence for enocean device container
  class DaliPersistence : public SQLite3Persistence
  {
    typedef SQLite3Persistence inherited;
  protected:
    /// Get DB Schema creation/upgrade SQL statements
    virtual string dbSchemaUpgradeSQL(int aFromVersion, int &aToVersion);
  };


  class DaliDeviceContainer : public DeviceClassContainer
  {
    typedef DeviceClassContainer inherited;

		DaliPersistence db;

  public:
    DaliDeviceContainer(int aInstanceNumber, DeviceContainer *aDeviceContainerP, int aTag);

		void initialize(CompletedCB aCompletedCB, bool aFactoryReset);

    // the DALI communication object
    DaliCommPtr daliComm;

    virtual const char *deviceClassIdentifier() const;

    /// perform self test
    /// @param aCompletedCB will be called when self test is done, returning ok or error
    virtual void selfTest(CompletedCB aCompletedCB);

    /// get supported rescan modes for this device class
    /// @return a combination of rescanmode_xxx bits
    virtual int getRescanModes() const;

    /// collect and add devices to the container
    virtual void collectDevices(CompletedCB aCompletedCB, bool aIncremental, bool aExhaustive, bool aClearSettings);

    /// vdc level methods (p44 specific, JSON only, for configuring multichannel RGB(W) devices)
    virtual ErrorPtr handleMethod(VdcApiRequestPtr aRequest, const string &aMethod, ApiValuePtr aParams);

    /// @return human readable, language independent suffix to explain vdc functionality.
    ///   Will be appended to product name to create modelName() for vdcs
    virtual string vdcModelSuffix() { return "DALI"; }

    /// ungroup a previously grouped device
    /// @param aDevice the device to ungroup
    /// @param aRequest the API request that causes the ungroup, will be sent an OK when ungrouping is complete
    /// @return error if not successful
    ErrorPtr ungroupDevice(DaliDevicePtr aDevice, VdcApiRequestPtr aRequest);

    /// Get icon data or name
    /// @param aIcon string to put result into (when method returns true)
    /// - if aWithData is set, binary PNG icon data for given resolution prefix is returned
    /// - if aWithData is not set, only the icon name (without file extension) is returned
    /// @param aWithData if set, PNG data is returned, otherwise only name
    /// @return true if there is an icon, false if not
    virtual bool getDeviceIcon(string &aIcon, bool aWithData, const char *aResolutionPrefix);

  private:

    void deviceListReceived(CompletedCB aCompletedCB, DaliComm::ShortAddressListPtr aDeviceListPtr, ErrorPtr aError);
    void queryNextDev(DaliBusDeviceListPtr aBusDevices, DaliBusDeviceList::iterator aNextDev, CompletedCB aCompletedCB, ErrorPtr aError);
    void initializeNextDimmer(DaliBusDeviceListPtr aDimmerDevices, uint16_t aGroupsInUse, DaliBusDeviceList::iterator aNextDimmer, CompletedCB aCompletedCB, ErrorPtr aError);
    void createDsDevices(DaliBusDeviceListPtr aDimmerDevices, CompletedCB aCompletedCB);
    void deviceInfoReceived(DaliBusDeviceListPtr aBusDevices, DaliBusDeviceList::iterator aNextDev, CompletedCB aCompletedCB, DaliComm::DaliDeviceInfoPtr aDaliDeviceInfoPtr, ErrorPtr aError);
    void groupCollected(VdcApiRequestPtr aRequest);

    void testScanDone(CompletedCB aCompletedCB, DaliComm::ShortAddressListPtr aShortAddressListPtr, ErrorPtr aError);
    void testRW(CompletedCB aCompletedCB, DaliAddress aShortAddr, uint8_t aTestByte);
    void testRWResponse(CompletedCB aCompletedCB, DaliAddress aShortAddr, uint8_t aTestByte, bool aNoOrTimeout, uint8_t aResponse, ErrorPtr aError);

  };

} // namespace p44


#endif /* defined(__vdcd__dalidevicecontainer__) */
