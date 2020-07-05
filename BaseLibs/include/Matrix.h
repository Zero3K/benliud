// Matrix.h: interface for the CMatrix class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _MATRIX_H
#define _MATRIX_H

//æÿ’Û‘ÀÀ„¿‡
#include <stdio.h>
#include "BiArray.h"
namespace FCadLib{

class CMatrix  
{
public:
	double operator [](int row);
	CMatrix(CMatrix &M);
	bool Reverse();
	CMatrix operator*(CMatrix &M);
	void Clone(CMatrix &M);
	double* GetData();
	int GetCols();
	int GetRows();
	bool SetData(double *pData,int nRows,int nCols);
	void AddLineToLine(int nLine1, double ratio,int nLine2);
	void SwapLine(int nLine1, int nLine2);
	void RatioLine(int nLine, double ratio);
	CMatrix();
	virtual ~CMatrix();
	void operator *=(double x);
	void operator /=(double x);
	void operator +=(double x);
	void operator -=(double x);
	void operator +=(CMatrix &M);
	void operator -=(CMatrix &M);
	void operator *=(CMatrix &M);
	CMatrix& operator =(CMatrix &M);

private:
	int m_nRows;
	int m_nCols;
	double *m_pData;
};
}
#endif 