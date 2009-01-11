/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id: nmrBernsteinPolynomialLineIntegralTest.cpp,v 1.7 2007/04/26 20:12:05 anton Exp $
  
  Author(s):  Ofri Sadowsky
  Created on: 2004-10-21
  
  (C) Copyright 2004-2007 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/


#include "nmrBernsteinPolynomialLineIntegralTest.h"

#include "nmrPolynomialBaseTest.h"
#include "nmrBernsteinPolynomialTest.h"

#include <math.h>

const nmrBernsteinPolynomialLineIntegralTest::VariableIndexType 
nmrBernsteinPolynomialLineIntegralTest::DefaultNumVariables = 4;

const nmrBernsteinPolynomialLineIntegralTest::PowerType 
nmrBernsteinPolynomialLineIntegralTest::DegreeMax = 6;

const double nmrBernsteinPolynomialLineIntegralTest::Tolerance = 1.0e-5;

void nmrBernsteinPolynomialLineIntegralTest::setUp()
{
    RandomIntegrationPoint(IntegrationPoint0, RandomGenerator);
    RandomIntegrationPoint(IntegrationPoint1, RandomGenerator);
}

void 
nmrBernsteinPolynomialLineIntegralTest::
RandomIntegrationPoint(IntegrationPointType & integrationPoint,
                       cmnRandomSequence & randomGenerator)
{
    const unsigned int numVariables = integrationPoint.size();
    nmrPolynomialBaseTest::RandomVariables( &(integrationPoint[0]), numVariables, randomGenerator );
    VariableType sum = integrationPoint.SumOfElements();
    while (sum < 1) {
        integrationPoint += VariableType(1);
        sum += numVariables;
    }

    integrationPoint /= sum;
}

void nmrBernsteinPolynomialLineIntegralTest::TestScalarIntegration(nmrBernsteinPolynomial & integrand,
                                                                nmrBernsteinPolynomialLineIntegral & integrator,
                                                                IntegrationPointType & p0,
                                                                IntegrationPointType & p1,
                                                                cmnRandomSequence & randomGenerator)
{
    CoefficientType coefficient = nmrPolynomialBaseTest::ChooseRandomVariable(randomGenerator);
    nmrBernsteinPolynomialTest::MakeScalarPolynomial(integrand, coefficient);
    integrator.UpdateIntegrationTableau();

    const ValueType p1p0distance = (p1 - p0).Norm();
    const ValueType expectedResult = coefficient * p1p0distance;

    const ValueType valueFrom0To1 = integrator.EvaluateForSegment( &(p0[0]), &(p1[0]), p1p0distance );
    const ValueType valueFrom1To0 = integrator.EvaluateForSegment( &(p1[0]), &(p0[0]), p1p0distance );

    const double error1 = fabs( (valueFrom0To1 - expectedResult) );
    const double errorScale1 = (fabs(expectedResult) >= 1.0) ? fabs(expectedResult) : 1.0;
    const double relativeError1 = error1 / errorScale1;

    const double error2 = fabs( (valueFrom1To0 - expectedResult) );
    const double errorScale2 = (fabs(expectedResult) >= 1.0) ? fabs(expectedResult) : 1.0;
    const double relativeError2 = error2 / errorScale2;

    CPPUNIT_ASSERT( relativeError1 <= Tolerance );
    CPPUNIT_ASSERT( relativeError2 <= Tolerance );
}



void nmrBernsteinPolynomialLineIntegralTest::TestMidpointIntegration(nmrBernsteinPolynomial & integrand,
                                                                  nmrBernsteinPolynomialLineIntegral & integrator,
                                                                  IntegrationPointType & p0,
                                                                  IntegrationPointType & p1,
                                                                  cmnRandomSequence & randomGenerator)
{
    unsigned long maxTerms = integrand.GetMaxNumberOfTerms();
    integrand.Clear();
    nmrPolynomialBaseTest::InsertRandomTerms(integrand, int(sqrt(static_cast<double>(maxTerms))) + 1, randomGenerator);
    nmrDynAllocPolynomialContainerTest::SetRandomCoefficients(integrand, randomGenerator);

    double midPointRatio = randomGenerator.ExtractRandomFloat();
    IntegrationPointType midPoint = p0 * (1 - midPointRatio) + p1 * midPointRatio;

    const ValueType p0p1Distance = (p1 - p0).Norm();
    const ValueType p0midDistance = (midPoint - p0).Norm();
    const ValueType midp1Distance = (p1 - midPoint).Norm();

    ValueType valueP0P1 = integrator.EvaluateForSegment( &(p0[0]), &(p1[0]), p0p1Distance );

    ValueType valueP0MidPoint = integrator.EvaluateForSegment( &(p0[0]), &(midPoint[0]), p0midDistance );
    ValueType valueMidPointP1 = integrator.EvaluateForSegment( &(midPoint[0]), &(p1[0]), midp1Distance );

    ValueType sumValues = valueP0MidPoint + valueMidPointP1;

    double error = fabs( (sumValues - valueP0P1) );
    double errorScale = (fabs(valueP0P1) >= 1.0) ? fabs(valueP0P1) : 1.0;
    double relativeError = error / errorScale;

    CPPUNIT_ASSERT( relativeError <= Tolerance );
}

CPPUNIT_TEST_SUITE_REGISTRATION(nmrBernsteinPolynomialLineIntegralTest);


// ****************************************************************************
//                              Change History
// ****************************************************************************
//
// $Log: nmrBernsteinPolynomialLineIntegralTest.cpp,v $
// Revision 1.7  2007/04/26 20:12:05  anton
// All files in tests: Applied new license text, separate copyright and
// updated dates, added standard header where missing.
//
// Revision 1.6  2006/11/20 20:33:52  anton
// Licensing: Applied new license to tests.
//
// Revision 1.5  2005/09/26 15:41:47  anton
// cisst: Added modelines for emacs and vi.
//
// Revision 1.4  2005/09/06 15:41:37  anton
// cisstNumerical tests: Added license.
//
// Revision 1.3  2004/10/27 17:57:21  ofri
// cisstNumericalTest: Following ticket #82, I erased the use of valarray, and
// instead I am using vctDynamicVector.  Huh!
//
// Revision 1.2  2004/10/27 02:46:32  anton
// cisstNumericalTests: I am unsure of this modification.  It compiles with .net
// 2003 and the tests succeeded but I feel there should be a more systematic
// approach to this problem.
//
// Revision 1.1  2004/10/24 03:51:03  ofri
// Renamed cisstNumerical test classes and files.  Tests run but fail for some
// cases.
//
// ****************************************************************************
