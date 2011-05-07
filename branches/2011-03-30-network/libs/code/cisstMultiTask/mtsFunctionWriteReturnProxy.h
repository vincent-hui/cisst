/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id$

  Author(s):  Min Yang Jung
  Created on: 2009-09-03

  (C) Copyright 2009 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/


/*!
  \file
  \brief Defines the function proxy objects.
*/

#ifndef _mtsFunctionWriteReturnProxy_h
#define _mtsFunctionWriteReturnProxy_h

#include <cisstMultiTask/mtsFunctionWriteReturn.h>
#include "mtsInterfaceRequiredProxy.h"
#include "mtsProxySerializer.h"

#include <cisstMultiTask/mtsExport.h>

class CISST_EXPORT mtsFunctionWriteReturnProxy: public mtsFunctionWriteReturn {
protected:
    typedef mtsFunctionWriteReturn BaseType;
    mtsInterfaceRequiredProxy * InterfaceRequired;
    mtsProxySerializer Serializer;
    mtsGenericObject * ArgumentPointer;
    mtsGenericObject * ResultPointer;
    mtsObjectIDType RemoteResultPointer;

public:
    mtsFunctionWriteReturnProxy(mtsInterfaceRequired * interfaceRequired):
        mtsFunctionWriteReturn(true /* this is a proxy class */),
        ArgumentPointer(0),
        ResultPointer(0)
    {
        this->InterfaceRequired = dynamic_cast<mtsInterfaceRequiredProxy *>(interfaceRequired);
    }

    ~mtsFunctionWriteReturnProxy()
    {
        if (this->ArgumentPointer) {
            delete this->ArgumentPointer;
        }
        if (this->ResultPointer) {
            delete this->ResultPointer;
        }
    }

    /*! Getter */
    inline mtsProxySerializer * GetSerializer(void) {
        return &(this->Serializer);
    }

    inline mtsGenericObject * GetArgumentPointer(void) const {
        return this->ArgumentPointer;
    }

    inline void SetArgumentPointer(mtsGenericObject * genericObject) {
        this->ArgumentPointer = genericObject;
    }

    inline mtsGenericObject * GetResultPointer(void) const {
        return this->ResultPointer;
    }

    inline void SetResultPointer(mtsGenericObject * genericObject) {
        this->ResultPointer = genericObject;
    }

    inline mtsObjectIDType GetRemoteResultPointer(void) const {
        return this->RemoteResultPointer;
    }

    inline void SetRemoteResultPointer(const mtsObjectIDType remoteResultPointer) {
        this->RemoteResultPointer = remoteResultPointer;
    }

    inline void SetAsLastFunction(void) {
        this->InterfaceRequired->SetLastFunction(this);
    }
};

#endif // _mtsFunctionWriteReturnProxy_h