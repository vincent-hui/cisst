/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id: mtsFaultDetector.cpp 3303 2012-01-02 16:33:23Z mjung5 $

  Author(s):  Min Yang Jung
  Created on: 2012-02-07

  (C) Copyright 2012 Johns Hopkins University (JHU), All Rights Reserved.

  --- begin cisst license - do not edit ---

  This software is provided "as is" under an open source license, with
  no warranty.  The complete license can be found in license.txt and
  http://www.cisst.org/cisst/license.txt.

  --- end cisst license ---
*/

#include <cisstCommon/cmnThrow.h>
#include <cisstMultiTask/mtsFaultDetectorThresholding.h>

CMN_IMPLEMENT_SERVICES(mtsFaultDetectorThresholding);

mtsFaultDetectorThresholding::mtsFaultDetectorThresholding(void)
    : mtsFaultDetectorBase("FaultDetector-Thresholding"),
      SumOfSamples(0.0), SumOfSquaredSamples(0.0), SampleCount(0),
      Sigma(0.0), Mean(0.0), K(0),
      UCL(0.0), LCL(0.0)
{
    this->Enable(false);
}

mtsFaultDetectorThresholding::mtsFaultDetectorThresholding(const std::string & signalName, 
                                                           double average, 
                                                           size_t samplingCount,
                                                           size_t k)
    : mtsFaultDetectorBase("FaultDetector-Thresholding"),
      SumOfSamples(0.0), SumOfSquaredSamples(0.0), SampleCount(samplingCount),
      Sigma(0.0), Mean(average), K(k),
      UCL(0.0), LCL(0.0)
{
    // Define inputs
    this->AddInputSignal(signalName, mtsMonitorFilterBase::SignalElement::SCALAR);

    // Define outputs
    std::stringstream ss;
    ss << "FDDTh:" << signalName << ":" << this->FilterUID;
    AddOutputSignal(ss.str(), mtsMonitorFilterBase::SignalElement::SCALAR);
}

mtsFaultDetectorThresholding::~mtsFaultDetectorThresholding()
{
}

void mtsFaultDetectorThresholding::CheckFault(bool debug)
{
    if (!this->IsEnabled()) return;

    // Fetch new sample from state table
    double timestamp;
    double x = StateTable->GetNewValueScalar(InputSignals[0]->GetStateDataId(), timestamp);

    static size_t samplingCount = SampleCount;

    if (samplingCount > 0) {
        SumOfSamples += x;
        SumOfSquaredSamples += x * x;
        if (--samplingCount == 0) {
            double avg = SumOfSamples / (double) SampleCount;
            // Determine statistic variables for thresholding
            Sigma = sqrt( (SumOfSquaredSamples / (double) SampleCount) - (avg * avg) );
            UCL = Mean + K * Sigma;
            LCL = Mean - K * Sigma;

            CMN_LOG_CLASS_RUN_VERBOSE << "CheckFault: Finished collecting " << SampleCount << " samples" << std::endl;
            CMN_LOG_CLASS_RUN_VERBOSE << "avg = " << avg << ", sigma = " << Sigma << ", UCL = " << UCL << ", LCL = " << LCL << std::endl;

            if (debug)
                std::cout << "sampling: " << samplingCount << " / " << SampleCount << std::endl
                        << ", avg = " << avg << ", std = " << Sigma << std::endl;
        }
        return;
    }

    // in control
    if (LCL <= x && x <= UCL)
        return;

    // fault occurs: out of control sample found
    CMN_LOG_CLASS_RUN_WARNING << "CheckFault: OUT OF LIMIT SAMPLE!!! input: " << x << ", time: " << timestamp << std::endl;
}

void mtsFaultDetectorThresholding::ToStream(std::ostream & outputStream) const
{
    outputStream << "Fault Detector Name: " << this->GetFilterName()
                 << ", " << (this->IsEnabled() ? "Enabled" : "Disabled") << ", "
                 << ", Input: \"" << InputSignals[0]->GetName() << "\""
                 << ", Output: \"" << OutputSignals[0]->GetName() << "\""
                 << ", Sigma: " << Sigma << ", Mean: " << Mean << ", K: " << K
                 << ", UCL: " << UCL << ", LCL: " << LCL;
}
