// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Includes
#include "CollectionInspectionAPITypes.h"
#include "LoggingModule.h"
#include "OBDDataDecoder.h"
#include "businterfaces/ISOTPOverCANSenderReceiver.h"

namespace Aws
{
namespace IoTFleetWise
{
namespace DataInspection
{
using namespace Aws::IoTFleetWise::Platform::Linux;
using namespace Aws::IoTFleetWise::VehicleNetwork;
using namespace Aws::IoTFleetWise::DataManagement;
/**
 * @brief This class handles sending OBD PID / DTC requests to an ECU and processing corresponding responses.
 */
class OBDOverCANECU
{
public:
    OBDOverCANECU() = default;
    ~OBDOverCANECU() = default;
    OBDOverCANECU( const OBDOverCANECU & ) = delete;
    OBDOverCANECU &operator=( const OBDOverCANECU & ) = delete;
    OBDOverCANECU( OBDOverCANECU && ) = delete;
    OBDOverCANECU &operator=( OBDOverCANECU && ) = delete;

    /**
     * @brief Initializes the OBD Diagnostic Session with an ECU.
     * @param gatewayCanInterfaceName CAN IF Name where the OBD stack on the ECU
     * is running. Typically on the Gateway ECU.
     * @param obdDataDecoder OBD decoder
     * @param rxId CAN Receive ID
     * @param txId CAN Transmit ID
     * @param isExtendedId CAN ID is standard(11-bit) or extended(29-bit)
     * @param signalBufferPtr Signal Buffer shared pointer
     * @return True if initialization of ISO-TP is successful
     */
    bool init( const std::string &gatewayCanInterfaceName,
               const std::shared_ptr<OBDDataDecoder> &obdDataDecoder,
               const uint32_t rxId,
               const uint32_t txId,
               bool isExtendedId,
               SignalBufferPtr &signalBufferPtr );

    /**
     * @brief Returns the health state of the ISO-TP Connection.
     * @return True if successful. False otherwise.
     */
    bool isAlive() const;

    /**
     * @brief Get the supported PIDs from ECU
     *
     * @param sid Service Mode
     * @return true if successfully get the supported PID list from ECU
     */
    bool requestReceiveSupportedPIDs( const SID sid );

    /**
     * @brief request PIDs from ECU
     *
     * @param sid Service Mode. e.g: 1
     * @return true if software received the PID response correctly from ECU
     * @return false if software didn't receive the PID response correctly from ECU
     */
    bool requestReceiveEmissionPIDs( const SID sid );

    /**
     * @brief get DTC from ECU
     *
     * @param dtcInfo DTCInfo is a structure contains a list of DTC codes
     * @return true if software received the DTC response correctly from ECU
     * @return false if software didn't receive the DTC response correctly from ECU
     */
    bool getDTCData( DTCInfo &dtcInfo );

    /**
     * @brief Get the CAN Receiver ID
     *
     * @return CAN Receiver ID in string format
     */
    inline std::string
    getRxID() const
    {
        return mStreamRxID;
    }

    /**
     * @brief update PID to request list based on
     * 1) whether PID is supported by this ECU;
     * 2) PID has been requested by decoder dictionary
     * 3) whether PID has not been assigned to other ECU yet
     *
     * @param sid Service Mode
     * @param pidsRequestedByDecoderDict a vector of PIDs that are requested by decoder dictionary
     * @param pidAssigned a set of PID that have been assigned to an ECU
     */
    void updatePIDRequestList( const SID sid,
                               const std::vector<PID> &pidsRequestedByDecoderDict,
                               std::unordered_set<PID> &pidAssigned );

private:
    /**
     * @brief Returns the request list of PID
     * for the given SID
     * @param sid the SID e.g. Mode 1
     * @param requestedPIDs container where the result will be copied
     * @return True if successful. False if the SID was not supported.
     */
    bool getRequestedPIDs( const SID sid, std::vector<PID> &requestedPIDs ) const;

    // Issues a supported PID request to the specific ECU and SID
    bool requestSupportedPIDs( const SID sid );
    // Receives the supported PID request to the specific ECU and SID
    bool receiveSupportedPIDs( const SID sid, SupportedPIDs &supportedPIDs );

    // Request a set of PIDs from one ECU
    void requestReceivePIDs(
        size_t range, bool isRangeCountPositive, const SID sid, const SupportedPIDs &pids, EmissionInfo &info );
    bool requestPIDs( const SID sid, const std::vector<PID> &pids );
    bool receivePIDs( const SID sid, const std::vector<PID> &pids, EmissionInfo &info );

    // Request DTCs
    bool requestReceiveDTCs( const SID sid, DTCInfo &info );
    bool requestDTCs( const SID sid );
    bool receiveDTCs( const SID sid, DTCInfo &info );

    LoggingModule mLogger;
    std::shared_ptr<const Clock> mClock = ClockHandler::getClock();
    // A map contains the Supported PIDs for each service mode
    std::unordered_map<SID, SupportedPIDs> mSupportedPIDs;
    std::vector<uint8_t> mTxPDU;
    std::vector<uint8_t> mRxPDU;
    std::shared_ptr<OBDDataDecoder> mOBDDataDecoder;
    // Signal Buffer shared pointer. This is a multiple producer single consumer queue
    SignalBufferPtr mSignalBufferPtr;
    // The PIDs to request from ECU. This assignment would come from OBDOverCANModule
    std::unordered_map<SID, std::vector<PID>> mPIDsToRequest;
    // can iso-tp tranceiver
    ISOTPOverCANSenderReceiver mISOTPSenderReceiver;
    // socket can receiver ID
    std::string mStreamRxID;
    // Per J1979, maximum 6 PIDs can be requested in one message
    static constexpr size_t MAX_PID_RANGE = 6U;
};
} // namespace DataInspection
} // namespace IoTFleetWise
} // namespace Aws
