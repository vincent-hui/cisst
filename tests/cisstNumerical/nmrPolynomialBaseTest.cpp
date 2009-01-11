/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id: nmrPolynomialBaseTest.cpp,v 1.8 2007/04/26 20:12:05 anton Exp $
  
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


#include "nmrPolynomialBaseTest.h"
#include "nmrPolynomialTermPowerIndexTest.h"

#include <math.h>
#include <cisstVector/vctRandom.h>

const nmrPolynomialBaseTest::VariableIndexType nmrPolynomialBaseTest::DefaultNumVariables = 4;
const nmrPolynomialBaseTest::VariableType nmrPolynomialBaseTest::VariableValueMax = 10;
const nmrPolynomialBaseTest::PowerType nmrPolynomialBaseTest::DegreeMax = 7;
const double nmrPolynomialBaseTest::Tolerance = 1.0e-6;

void 
nmrPolynomialBaseTest::SetRandomVariables(nmrMultiVariablePowerBasis & powerBasis,
                                          cmnRandomSequence & randomGenerator)
{
	nmrMultiVariablePowerBasis::VariableIndexType i;
    const nmrMultiVariablePowerBasis::VariableIndexType numVariables = powerBasis.GetNumVariables();
	for (i = 0; i < numVariables; i++) {
		if (!powerBasis.CanSetVariable(i))
			continue;
		VariableType v = ChooseRandomVariable(randomGenerator);
		powerBasis.SetVariable(i, v);
	}
}

nmrPolynomialBaseTest::TermCounterType nmrPolynomialBaseTest::InsertRandomTerms(nmrPolynomialBase & polynomial, 
																		  TermCounterType maxNumTerms,
                                                                          cmnRandomSequence & randomGenerator)
{
	TermCounterType i;
	nmrPolynomialBase::InsertStatus insertStatus;
	TermCounterType successfulInserts = 0;

	for (i = 0; i < maxNumTerms; i++) {
		nmrPolynomialTermPowerIndex powerIndex(polynomial);
		nmrPolynomialTermPowerIndexTest::MakeRandomPowerIndex(powerIndex, randomGenerator);
		insertStatus = polynomial.SetCoefficient(powerIndex, 0);
		if (insertStatus == nmrPolynomialBase::INSERT_NEW)
			successfulInserts++;
	}

	return successfulInserts;
}


void nmrPolynomialBaseTest::TestSingleRandomTerm(nmrPolynomialBase & polynomial,
                                              cmnRandomSequence & randomGenerator)
{
	nmrPolynomialTermPowerIndex powerIndex(polynomial);
	nmrPolynomialTermPowerIndexTest::MakeRandomPowerIndex(powerIndex, randomGenerator);
	CoefficientType coefficient = ChooseRandomVariable(randomGenerator);

    CPPUNIT_ASSERT( polynomial.CanIncludeIndex(powerIndex) );

	polynomial.RemoveTerm(powerIndex);

	nmrPolynomialBase::InsertStatus insertStatus = polynomial.SetCoefficient(powerIndex, coefficient);

    CPPUNIT_ASSERT( insertStatus == nmrPolynomialBase::INSERT_NEW );

    CPPUNIT_ASSERT( polynomial.IncludesIndex(powerIndex) );

    CPPUNIT_ASSERT( polynomial.GetCoefficient(powerIndex) == coefficient );

	coefficient = ChooseRandomVariable(randomGenerator);
	insertStatus = polynomial.SetCoefficient(powerIndex, coefficient);

    CPPUNIT_ASSERT( insertStatus == nmrPolynomialBase::INSERT_REPLACE );

    CPPUNIT_ASSERT( polynomial.IncludesIndex(powerIndex) );

    CPPUNIT_ASSERT( polynomial.GetCoefficient(powerIndex) == coefficient );

	polynomial.RemoveTerm(powerIndex);

    CPPUNIT_ASSERT( !polynomial.IncludesIndex(powerIndex) );

    CPPUNIT_ASSERT( polynomial.GetCoefficient(powerIndex) == 0.0 );
}


void nmrPolynomialBaseTest::TestCompareEvaluations(nmrPolynomialBase & polynomial,
                                                   nmrMultiVariablePowerBasis & powerBasis,
                                                   cmnRandomSequence & randomGenerator)
{
    polynomial.Clear();
    unsigned long maxNumTerms = polynomial.GetMaxNumberOfTerms();
    unsigned long numInserts = maxNumTerms / 2;
    unsigned long numTerms = InsertRandomTerms(polynomial, numInserts, randomGenerator);

    vctDynamicVector<double> coefficients(numTerms, 0.0);
    vctRandom(coefficients, -VariableValueMax, +VariableValueMax);

    nmrPolynomialBase::ValueType externalCoefficientsValue =
        polynomial.EvaluateForCoefficients( powerBasis, coefficients.Pointer() );
    polynomial.RestoreCoefficients( coefficients.Pointer() );
    nmrPolynomialBase::ValueType internalCoefficientsValue =
        polynomial.Evaluate( powerBasis );

    // NOTE: Some compilers create a different order of evaluation for
    // the two expressions, even though they should be the same.  For
    // example: gcc 4.0.  Therefore, we provide some tolerance on the
    // equality of value1 and value2.
    CPPUNIT_ASSERT( fabs(externalCoefficientsValue - internalCoefficientsValue) < Tolerance );
}

void nmrPolynomialBaseTest::TestScalePolynomial(nmrPolynomialBase & polynomial,
                                                nmrMultiVariablePowerBasis & powerBasis,
                                                cmnRandomSequence & randomGenerator)
{
    polynomial.Clear();
    unsigned long maxNumTerms = polynomial.GetMaxNumberOfTerms();
    unsigned long numInserts = maxNumTerms / 2;
    unsigned long numTerms = InsertRandomTerms(polynomial, numInserts, randomGenerator);

    vctDynamicVector<double> coefficients(numTerms, 0.0);
    vctRandom(coefficients, -VariableValueMax, +VariableValueMax);

    polynomial.RestoreCoefficients( coefficients.Pointer() );
    nmrPolynomialBase::ValueType initialValue = polynomial.Evaluate(powerBasis);

    CoefficientType scaleFactor = cmnRandomSequence::GetInstance().ExtractRandomDouble(-VariableValueMax, +VariableValueMax);
    polynomial.Scale(scaleFactor);
    coefficients.Multiply(scaleFactor);

    nmrPolynomialBase::ValueType value1 = polynomial.Evaluate( powerBasis );
    nmrPolynomialBase::ValueType value2 = polynomial.EvaluateForCoefficients( powerBasis, coefficients.Pointer() );

    CPPUNIT_ASSERT( fabs(initialValue * scaleFactor - value1) < 1.0e-4);
    CPPUNIT_ASSERT( fabs(initialValue * scaleFactor - value2) < 1.0e-4);
    // NOTE: Some compilers create a different order of evaluation for
    // the two expressions, even though they should be the same.  For
    // example: gcc 4.0.  Therefore, we provide some tolerance on the
    // equality of value1 and value2.
    CPPUNIT_ASSERT( fabs(value1 - value2) < 1.0e-4);
}


void nmrPolynomialBaseTest::TestAddConstant(nmrPolynomialBase & polynomial,
                                            nmrMultiVariablePowerBasis & powerBasis,
                                            cmnRandomSequence & randomGenerator)
{
    polynomial.Clear();
    unsigned long maxNumTerms = polynomial.GetMaxNumberOfTerms();
    unsigned long numInserts = maxNumTerms / 2;
    unsigned long numTerms = InsertRandomTerms(polynomial, numInserts, randomGenerator);

    vctDynamicVector<double> coefficients(numTerms, 0.0);
    vctRandom(coefficients, -VariableValueMax, +VariableValueMax);

    polynomial.RestoreCoefficients(coefficients.Pointer());
    nmrPolynomialBase::ValueType initialValue = polynomial.Evaluate(powerBasis);

    CoefficientType shiftAmount = cmnRandomSequence::GetInstance().ExtractRandomDouble( -VariableValueMax, +VariableValueMax );
    polynomial.AddConstant(shiftAmount);

    nmrPolynomialBase::ValueType value1 = polynomial.Evaluate( powerBasis );

    coefficients.resize( polynomial.GetNumberOfTerms() );
    polynomial.CollectCoefficients( coefficients.Pointer() );
    polynomial.AddConstantToCoefficients( coefficients.Pointer(), -shiftAmount);
    nmrPolynomialBase::ValueType value2 =
        polynomial.EvaluateForCoefficients( powerBasis, coefficients.Pointer());

    CPPUNIT_ASSERT( fabs(initialValue + shiftAmount - value1) < Tolerance * polynomial.GetNumberOfTerms() * polynomial.GetNumVariables());
    CPPUNIT_ASSERT( fabs(value2 - initialValue) < Tolerance * polynomial.GetNumberOfTerms() * polynomial.GetNumVariables());
}


// ****************************************************************************
//                              Change History
// ****************************************************************************
//
// $Log: nmrPolynomialBaseTest.cpp,v $
// Revision 1.8  2007/04/26 20:12:05  anton
// All files in tests: Applied new license text, separate copyright and
// updated dates, added standard header where missing.
//
// Revision 1.7  2006/12/04 20:48:53  ofri
// nmrPolynomialBaseTest : Apparently, gcc 4.0 does not keep the same
// evaluation order as previous versions.  Code changed to add tolerance
// to methods that previously relied on strict equality.
//
// Revision 1.6  2006/11/20 20:33:52  anton
// Licensing: Applied new license to tests.
//
// Revision 1.5  2006/07/10 15:42:56  ofri
// cusstNumerical tests : Adapted code so that the deprecated interfaces are
// not used.  Tests passed with msvc7, gcc-3.3.
//
// Revision 1.4  2005/09/26 15:41:47  anton
// cisst: Added modelines for emacs and vi.
//
// Revision 1.3  2005/09/06 15:41:37  anton
// cisstNumerical tests: Added license.
//
// Revision 1.2  2005/06/14 17:57:34  anton
// nmrPolynomialBaseTest: In TestAddConstant(), take into account the number of
// terms and variables to calculate the tolerance.
//
// Revision 1.1  2004/10/24 03:51:03  ofri
// Renamed cisstNumerical test classes and files.  Tests run but fail for some
// cases.
//
// ****************************************************************************
