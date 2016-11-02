//  File:   PrinterStatus.cpp
//  The data structure used to communicate status from the print engine 
//  to UI components
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <http://www.gnu.org/licenses/>.

#include <exception>

#define RAPIDJSON_ASSERT(x)                         \
  if (x);                                            \
  else throw std::exception();  

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <PrinterStatus.h>
#include <Logger.h>
#include <Shared.h>
#include <Settings.h>
#include <SparkStatus.h>

using namespace rapidjson;

// Constructor
PrinterStatus::PrinterStatus() :
_state(PrinterOnState),
_change(NoChange),
_UISubState(NoUISubState),
_isError(false),
_errorCode(Success),
_errno(0),
_numLayers(0),
_currentLayer(0),
_estimatedSecondsRemaining(0),
_temperature(0.0),
_printRating(Unknown),
_usbDriveFileName(""),
_jobID(""),
_canLoadPrintData(false),
_canUpgradeProjector(false)
{
    GetUUID(_localJobUniqueID); 
}

// Gets the name of a print engine state machine state
const char* PrinterStatus::GetStateName(PrintEngineState state)
{
    static bool initialized = false;
    static const char* stateNames[MaxPrintEngineState];
    if (!initialized)
    {
        // initialize the array of state names
        stateNames[PrinterOnState] = PRINTER_ON_STATE;
        stateNames[DoorClosedState] = DOOR_CLOSED_STATE;
        stateNames[InitializingState] = INITIALIZING_STATE;
        stateNames[DoorOpenState] = DOOR_OPEN_STATE;
        stateNames[HomingState] = HOMING_STATE;
        stateNames[HomeState] = HOME_STATE;
        stateNames[ErrorState] = ERROR_STATE;
        stateNames[MovingToStartPositionState] = MOVING_TO_START_POSITION_STATE;
        stateNames[InitializingLayerState] = INITIALIZING_LAYER_STATE;
        stateNames[PressingState] = PRESSING_STATE;
        stateNames[PressDelayState] = PRESS_DELAY_STATE;
        stateNames[UnpressingState] = UNPRESSING_STATE;
        stateNames[PreExposureDelayState] = PRE_EXPOSURE_DELAY_STATE;
        stateNames[ExposingState] = EXPOSING_STATE;
        stateNames[PrintingLayerState] = PRINTING_LAYER_STATE;
        stateNames[MovingToPauseState] = MOVING_TO_PAUSE_STATE;
        stateNames[PausedState] = PAUSED_STATE;
        stateNames[MovingToResumeState] = MOVING_TO_RESUME_STATE;
        stateNames[SeparatingState] = SEPARATING_STATE;
        stateNames[ApproachingState] = APPROACHING_STATE;
        stateNames[GettingFeedbackState] = GETING_FEEDBACK_STATE;
        stateNames[ConfirmCancelState] = CONFIRM_CANCEL_STATE;
        stateNames[AwaitingCancelationState] = AWAITING_CANCELATION_STATE;
        stateNames[ShowingVersionState] = SHOWING_VERSION_STATE;
        stateNames[CalibratingState] = CALIBRATING_STATE;
        stateNames[RegisteringState] = REGISTERING_STATE;
        stateNames[UnjammingState] = UNJAMMING_STATE;
        stateNames[JammedState] = JAMMED_STATE;
        stateNames[DemoModeState] = DEMO_MODE_STATE;
        stateNames[ConfirmUpgradeState] = CONFIRM_UPGRADE_STATE;
        stateNames[UpgradingProjectorState] = UPGRADING_PROJECTOR_STATE;
        stateNames[UpgradeCompleteState] = UPGRADE_COMPLETE_STATE;
        
        initialized = true;
    }
    
    if (state <= UndefinedPrintEngineState || state >= MaxPrintEngineState)
    {
        Logger::HandleError(UnknownPrintEngineState, false, NULL, state);
        return "";                                                              
    }
    return stateNames[state];
}

// Gets the name of a print engine state machine UI sub-state
const char* PrinterStatus::GetSubStateName(UISubState substate)
{
    static bool initialized = false;
    static const char* substateNames[MaxUISubState];
    if (!initialized)
    {
        // initialize the array of state names
        substateNames[NoUISubState] = NO_SUBSTATE;
        substateNames[NoPrintData] = NO_PRINT_DATA_SUBSTATE;
        substateNames[DownloadingPrintData] = DOWNLOADING_PRINT_DATA_SUBSTATE;
        substateNames[PrintDownloadFailed] = PRINT_DOWNLOAD_FAILED_SUBSTATE;
        substateNames[LoadingPrintData] = LOADING_PRINT_DATA_SUBSTATE;
        substateNames[LoadedPrintData] = LOADED_PRINT_DATA_SUBSTATE;
        substateNames[PrintDataLoadFailed] = PRINT_DATA_LOAD_FAILED_SUBSTATE;
        substateNames[HavePrintData] = HAVE_PRINT_DATA_SUBSTATE;
        substateNames[PrintCanceled] = PRINT_CANCELED_SUBSTATE;
        substateNames[PrintCompleted] = PRINT_COMPLETED_SUBSTATE;
        substateNames[ClearingScreen] = CLEARING_SCREEN_SUBSTATE;
        substateNames[Registered] = REGISTERED_SUBSTATE;
        substateNames[AboutToPause] = ABOUT_TO_PAUSE_SUBSTATE;
        substateNames[WiFiConnecting] = WIFI_CONNECTING_SUBSTATE;
        substateNames[WiFiConnectionFailed] = WIFI_CONNECTION_FAILED_SUBSTATE;
        substateNames[WiFiConnected] = WIFI_CONNECTED_SUBSTATE;
        substateNames[CalibratePrompt] = CALIBRATE_PROMPT_SUBSTATE;
        substateNames[USBDriveFileFound] = USB_FILE_FOUND_SUBSTATE;
        substateNames[USBDriveError] = USB_DRIVE_ERROR_SUBSTATE;
            
        initialized = true;
    }
    
    if (substate < NoUISubState || substate >= MaxUISubState)
    {
        Logger::HandleError(UnknownPrintEngineSubState, false, NULL, substate);
        return "";                                                              
    }
    return substateNames[substate];
}

// Returns printer status as a JSON formatted string.
std::string PrinterStatus::ToString() const
{
    std::string retVal = "";
   
    std::ostringstream json;
    json << "{" <<
            "\"" << STATE_PS_KEY           << "\": \"\"," <<
            "\"" << UISUBSTATE_PS_KEY      << "\": \"\"," <<
            "\"" << CHANGE_PS_KEY          << "\": \"\"," <<
            "\"" << IS_ERROR_PS_KEY        << "\": false," <<
            "\"" << ERROR_CODE_PS_KEY      << "\": 0," <<
            "\"" << ERRNO_PS_KEY           << "\": 0," <<
            "\"" << ERROR_MSG_PS_KEY       << "\": \"\"," <<
            "\"" << JOB_NAME_PS_KEY        << "\": \"\"," <<  
            "\"" << JOB_ID_PS_KEY          << "\": \"\"," <<  
            "\"" << LAYER_PS_KEY           << "\": 0," <<
            "\"" << TOTAL_LAYERS_PS_KEY    << "\": 0," <<
            "\"" << SECONDS_LEFT_PS_KEY    << "\": 0," <<
            "\"" << TEMPERATURE_PS_KEY     << "\": 0.0," <<
            "\"" << PRINT_RATING_PS_KEY    << "\": \"\"," <<
            "\"" << SPARK_STATE_PS_KEY     << "\": \"\"," <<
            "\"" << SPARK_JOB_STATE_PS_KEY << "\": \"\"," <<
            "\"" << LOCAL_JOB_UUID_PS_KEY  << "\": \"\"," <<
            "\"" << CAN_LOAD_PS_KEY        << "\": \"\"," <<
            "\"" << CAN_UPGRADE_PROJECTOR_PS_KEY    << "\": false" <<
            "}";
    
    try
    {
        std::string jsonString = json.str();
        Document doc;
        doc.Parse(jsonString.c_str());
        
        Value value;
        const char* state = GetStateName(_state);
        value.SetString(state, strlen(state), doc.GetAllocator());       
        doc[STATE_PS_KEY] = value; 
        
        const char* substate = GetSubStateName(_UISubState);
        value.SetString(substate, strlen(substate), doc.GetAllocator()); 
        doc[UISUBSTATE_PS_KEY] = value;
       
        if (_change == Entering)
           value.SetString(StringRef(ENTERING));
        else if (_change == Leaving)
           value.SetString(StringRef(LEAVING));
        else
            value.SetString(StringRef(NO_CHANGE));
        doc[CHANGE_PS_KEY] = value; 

        if (_printRating == Succeeded)
           value.SetString(StringRef(PRINT_SUCCESSFUL));
        else if (_printRating == Failed)
           value.SetString(StringRef(PRINT_FAILED));
        else
           value.SetString(StringRef(UNKNOWN_PRINT_FEEDBACK));
        doc[PRINT_RATING_PS_KEY] = value;         
        
        doc[IS_ERROR_PS_KEY] = _isError;        
        doc[ERROR_CODE_PS_KEY] = _errorCode; 
        doc[ERRNO_PS_KEY] = _errno; 
        value.SetString(GetLastErrorMessage().c_str(), 
                    GetLastErrorMessage().size(), doc.GetAllocator()); 
        doc[ERROR_MSG_PS_KEY] = value;       
        
        // job name comes from settings rather than PrinterStatus
        std::string ss = 
                    PrinterSettings::Instance().GetString(JOB_NAME_SETTING);
        value.SetString(ss.c_str(), ss.size(), doc.GetAllocator()); 
        doc[JOB_NAME_PS_KEY] = value;        
        
        value.SetString(_jobID.c_str(), _jobID.size(), doc.GetAllocator()); 
        doc[JOB_ID_PS_KEY] = value;        
        
        doc[LAYER_PS_KEY] = _currentLayer;
        doc[TOTAL_LAYERS_PS_KEY] = _numLayers;
        doc[SECONDS_LEFT_PS_KEY] = _estimatedSecondsRemaining;
        doc[TEMPERATURE_PS_KEY] = _temperature;
        
        // get the Spark API printer and job states
        ss = SparkStatus::GetSparkStatus(_state, _UISubState, 
                                                            _canLoadPrintData);
        value.SetString(ss.c_str(), ss.size(), doc.GetAllocator()); 
        doc[SPARK_STATE_PS_KEY] = value;
        
        // we know we're printing if we have a non-zero number of layers 
        ss = SparkStatus::GetSparkJobStatus(_state, _UISubState, 
                                                                _numLayers > 0);
        value.SetString(ss.c_str(), ss.size(), doc.GetAllocator()); 
        doc[SPARK_JOB_STATE_PS_KEY] = value;
        
        // write the UUID used by Spark for local jobs
        value.SetString(_localJobUniqueID, strlen(_localJobUniqueID), 
                                                            doc.GetAllocator()); 
        doc[LOCAL_JOB_UUID_PS_KEY] = value;
        
        doc[CAN_LOAD_PS_KEY] = _canLoadPrintData; 
        doc[CAN_UPGRADE_PROJECTOR_PS_KEY] = _canUpgradeProjector;
        
        StringBuffer buffer; 
        Writer<StringBuffer> writer(buffer);
        doc.Accept(writer);        
        retVal = std::string(buffer.GetString()) + "\n";
    }
    catch(std::exception)
    {
        Logger::HandleError(PrinterStatusToString);
    }
    return retVal; 
}

std::string _lastErrorMessage = "";

// Static method to set the one and only last error message.
void PrinterStatus::SetLastErrorMsg(std::string msg)
{
    _lastErrorMessage = msg;
}

// Static method to return the one and only last error message.
std::string PrinterStatus::GetLastErrorMessage()
{
    return _lastErrorMessage;
}

// Create a key to use for mapping the given print engine state and UI substate 
// into something else
PrinterStatusKey PrinterStatus::GetKey(PrintEngineState state, 
                                       UISubState subState)
{
    // This implementation assumes we never have more than 256 print
    // engine states or UI substates
    return state | (subState << 8);
}
