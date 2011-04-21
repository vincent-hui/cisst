/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id$

  Author(s):  Min Yang Jung, Anton Deguet
  Created on: 2010-09-01

  (C) Copyright 2011 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---

*/

#include <cisstCommon/cmnPortability.h>
#include <cisstOSAbstraction/osaSleep.h>
#include <cisstMultiTask/mtsManagerLocal.h>
#include <cisstMultiTask/mtsComponent.h>
#include <cisstMultiTask/mtsInterfaceRequired.h>

#include "mtsTestComponents.h"

class mtsTestConfigurationManager: public mtsComponent
{
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_ALLOW_DEFAULT);
public:

    mtsInterfaceRequired * InterfaceToComponentManager;
    mtsTestConfigurationManager(void):
        mtsComponent("configuration_manager")
    {
        InterfaceToComponentManager = EnableDynamicComponentManagement();
    }

    // ManagerComponentServices->RequestComponentCreate(processName, componentType, componentName)
    // ManagerComponentServices->RequestComponentConnect(processName, componentName, interfaceRequiredName,
    //                                                   processName, componentName, interfaceProvidedName))

};

CMN_DECLARE_SERVICES_INSTANTIATION(mtsTestConfigurationManager);
CMN_IMPLEMENT_SERVICES(mtsTestConfigurationManager);

int main(int CMN_UNUSED(argc), char ** CMN_UNUSED(argv))
{
    std::string command;

    // get local component manager instance
    mtsManagerLocal * componentManager;
    std::cin >> command;
    if (command == "connect") {
        try {
            componentManager = mtsManagerLocal::GetInstance("localhost", "configuration_manager");
        } catch (...) {
            std::cout << "failed to initialize local component manager" << std::endl;
            return 1;
        }
    } else {
        std::cout << "wrong command" << std::endl;
        return 1;
    }
    // send message to acknowledge connection
    std::cout << "configuration_manager connected" << std::endl;

    mtsTestConfigurationManager * configurationManager =  new mtsTestConfigurationManager;

    // create and start local components
    std::cin >> command;
    if (command == "start") {
        if (!componentManager->AddComponent(configurationManager)) {
            std::cout << "failed to add configuration manager component" << std::endl;
            return 1;
        }
        // create the tasks, i.e. find the commands
        componentManager->CreateAll();
        if (!componentManager->WaitForStateAll(mtsComponentState::READY, StateTransitionMaximumDelay)) {
            std::cout << "failed to reach state READY for configuration_manager" << std::endl;
            return 1;
        }
        // make sure the configuration manager is connected to the component manager
        if (!configurationManager->InterfaceToComponentManager->GetConnectedInterface()) {
            std::cout << "configuration manager not connected to component manager" << std::endl;
            return 1;
        }
        // start the periodic Run
        componentManager->StartAll();
        if (!componentManager->WaitForStateAll(mtsComponentState::ACTIVE, StateTransitionMaximumDelay)) {
            std::cout << "failed to reach state ACTIVE for configuration_manager" << std::endl;
            return 1;
        }

        // send message to confirm everything seems fine
        std::cout << "start succeeded" << std::endl;
    } else {
        std::cout << "wrong command" << std::endl;
        return 1;
    }

    // normal operations
    bool stop = false;
    std::string process1Name, process2Name, component1Name, component2Name, componentType;
    while (!stop) {
        std::cin >> command;
        if (command == std::string("stop")) {
            stop = true;
        } else if (command == std::string("ping")) {
            std::cout << "ok" << std::endl;
        } else if (command == std::string("createComponent")) {
            std::cin >> process1Name;
            std::cin >> componentType;
            std::cin >> component1Name;
            std::cout << "need to implement dynamic creation for " << process1Name << " " << componentType << " " << component1Name << std::endl;
        } else {
            std::cout << "unknown command \"" << command << "\"" << std::endl;
        }
        osaSleep(10.0 * cmn_ms);
    }

    // stop component manager
    componentManager->KillAll();
    if (!componentManager->WaitForStateAll(mtsComponentState::FINISHED, StateTransitionMaximumDelay)) {
        std::cout << "failed to reach state FINISHED for configuration_manager" << std::endl;
        return 1;
    }

    componentManager->Cleanup();
    std::cout << "stop succeeded" << std::endl;

    // wait to be killed by pipe
    while (true) {
        osaSleep(1.0 * cmn_hour);
    }

    return 0;
}

