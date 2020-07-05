// CBiArray.h: interface for the CBiArray class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CBiArray_H
#define _CBiArray_H


#include <stdlib.h>
#include <math.h>
//行列式类
namespace FCadLib{

class CBiArray  
{
public:

	double GetValue();
	CBiArray operator-(CBiArray &A);
	void Clone(CBiArray &A);
	double* GetData();
	int GetDegree();
	bool SetData(double *pData,int degree);
	void operator *=(double x);
	void operator /=(double x);
	void operator -=(double x);
	void operator +=(double x);
	void operator +=(CBiArray &A);
	void operator -=(CBiArray &A);
	bool operator ==(CBiArray &A);
	bool operator !=(CBiArray &A);
	CBiArray operator +(CBiArray &A);
	CBiArray();
	virtual ~CBiArray();
private:
	void SwapLine(int nLine1,int nLine2);
	void AddLineToLine(int nLine1,double ratio,int nLine2);
	void RatioLine(int nLine,double ratio);
	int m_Degree;
	double *m_pData;

};
}
#endif 