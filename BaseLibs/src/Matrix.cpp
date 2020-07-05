/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/

// Matrix.cpp: implementation of the CMatrix class.
//
//////////////////////////////////////////////////////////////////////

#include "../include/Matrix.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace FCadLib{

CMatrix::CMatrix()
{
	m_nCols=0;
	m_nRows=0;
	m_pData=NULL;
}

CMatrix::~CMatrix()
{
	if(m_pData) 
	{
		delete[] m_pData;
		m_pData=NULL;
	}
}

bool CMatrix::SetData(double *pData, int nRows, int nCols)
{
	if(nRows<=0) return false;
	if(nCols<=0) return false;
	if(pData==NULL) return false;

	if(m_pData!=NULL) { delete[] m_pData; m_pData=NULL;}
	m_pData=new double[nRows*nCols];
	if(m_pData==NULL) return false;

	for(int i=0;i<nRows;i++)
	{
		for(int j=0;j<nCols;j++)
		{
			m_pData[i*nCols+j]=pData[i*nCols+j];
		}
	}

	m_nCols=nCols;
	m_nRows=nRows;

	return true;
}

int CMatrix::GetRows()
{
	return m_nRows;
}

int CMatrix::GetCols()
{
	return m_nCols;
}

double* CMatrix::GetData()
{
	return m_pData;
}

void CMatrix::operator *=(double x)
{
	if(m_pData==NULL) return;
	for(int i=0;i<m_nRows;i++)
	{
		for(int j=0;j<m_nCols;j++)
		{
			m_pData[i*m_nCols+j]*=x;
		}
	}
}
void CMatrix::operator /=(double x)
{
	if(m_pData==NULL) return;
	for(int i=0;i<m_nRows;i++)
	{
		for(int j=0;j<m_nCols;j++)
		{
			m_pData[i*m_nCols+j]/=x;
		}
	}
}
void CMatrix::operator +=(double x)
{
	if(m_pData==NULL) return;
	for(int i=0;i<m_nRows;i++)
	{
		for(int j=0;j<m_nCols;j++)
		{
			m_pData[i*m_nCols+j]+=x;
		}
	}
}

void CMatrix::operator -=(double x)
{
	if(m_pData==NULL) return;
	for(int i=0;i<m_nRows;i++)
	{
		for(int j=0;j<m_nCols;j++)
		{
			m_pData[i*m_nCols+j]-=x;
		}
	}
}

void CMatrix::operator +=(CMatrix &M)
{
	if(m_nRows!=M.GetRows()) return ;
	if(m_nCols!=M.GetCols()) return ;

	double *pData=M.GetData();
	for(int i=0;i<m_nRows;i++)
	{
		for(int j=0;j<m_nCols;j++)
		{
			m_pData[i*m_nCols+j]+=pData[i*m_nCols+j];
		}
	}

}


void CMatrix::operator -=(CMatrix &M)
{
	if(m_nRows!=M.GetRows()) return ;
	if(m_nCols!=M.GetCols()) return ;

	double *pData=M.GetData();
	for(int i=0;i<m_nRows;i++)
	{
		for(int j=0;j<m_nCols;j++)
		{
			m_pData[i*m_nCols+j]-=pData[i*m_nCols+j];
		}
	}

}

void CMatrix::operator *=(CMatrix &M)
{
	if(m_nCols!=M.GetRows()) return ;

	double *pData=M.GetData();
	
	int nNewRows=m_nRows;
	int nNewCols=M.GetCols();

	double *pNewData=new double[nNewRows*nNewCols];	//新的数据存储区

	for(int i=0;i<nNewRows;i++)
	{
		for(int j=0;j<nNewCols;j++)
		{
			double cell=0;	//新矩阵的单位值
			for(int k=0;k<m_nCols;k++)
			{
				cell+=m_pData[i*m_nCols+k]*pData[k*nNewCols+j];
			}
			pNewData[i*nNewCols+j]=cell;
		}
	}

	delete[] m_pData;
	m_pData=pNewData;
	m_nRows=nNewRows;
	m_nCols=nNewCols;

}

CMatrix& CMatrix::operator=(CMatrix &M)
{
	Clone(M);
	return *this;
}


void CMatrix::Clone(CMatrix &M)
{
	if(m_pData) {delete[] m_pData; m_pData=NULL;}
	
	m_nCols=M.GetCols();
	m_nRows=M.GetRows();

	double *pData=M.GetData();
	m_pData=new double[m_nRows*m_nCols];

	for(int i=0;i<m_nRows;i++)
	{
		for(int j=0;j<m_nCols;j++)
		{
			m_pData[i*m_nCols+j]=pData[i*m_nCols+j];
		}
	}
}

CMatrix CMatrix::operator *(CMatrix &M)
{
	CMatrix result=*this;
//	CMatrix result;
//	result.Clone(*this);
	result*=M;
	return result;
}
//用行变换的方法求解,构造n*2n的矩阵,把右边设置成单位矩阵
//通过行变换把左边变成单位矩阵则右边就是逆矩阵
bool CMatrix::Reverse()
{
	//逆矩阵必须是方阵
	if(m_nCols!=m_nRows) return false;
	if(m_pData==NULL) return false;
	if(m_nCols==1) 
	{
		m_pData[0]=1/m_pData[0];
		return true;
	}

	int i,j;
	double *pNewMatrix=new double[m_nRows*2*m_nCols];
	//拷贝原始数据
	for(i=0;i<m_nRows;i++)
	{
		for(j=0;j<m_nCols;j++)
		{
			//拷贝原始数据
			pNewMatrix[i*(2*m_nCols)+j]=m_pData[i*m_nCols+j];
			//设置右边为单位矩阵
			if(j==i) pNewMatrix[i*(2*m_nCols)+j+m_nCols]=1;
			else pNewMatrix[i*(2*m_nCols)+j+m_nCols]=0;
		}
	}
	
	delete[] m_pData;
	m_pData=pNewMatrix;
	m_nCols=2*m_nCols;
//变换,先把左边下三角变0
	for(i=0;i<m_nRows-1;i++)
	{
		//左上保证不为0
		if(m_pData[i*m_nCols+i]==0)	//这里是否能用等号?
		{
			//按行向下找第一个不为0的行
			for(j=i+1;j<m_nRows;j++)
			{
				if(m_pData[j*m_nCols+i]!=0)
					break;
			}
			
			if(j==m_nRows) return false;	//本列找不到一个不为0的

			SwapLine(i,j);

		}
		
		//到这里左上已经不为0,把i列下面全变成0
		for(j=i+1;j<m_nRows;j++)
		{
			if(m_pData[j*m_nCols+i]!=0)
			{
				double ratio=-(m_pData[j*m_nCols+i])/m_pData[i*m_nCols+i];
				AddLineToLine(i,ratio,j);
			}
		}
	}

//到这里下三角为0,现在做左边上三角为0
	//从最后一行开始循环到第二行向上跌加
	for(i=m_nRows-1;i>0;i--)
	{
		for(j=i-1;j>=0;j--)
		{
			if(m_pData[j*m_nCols+i]!=0)
			{
				double ratio=-(m_pData[j*m_nCols+i])/m_pData[i*m_nCols+i];
				AddLineToLine(i,ratio,j);
			}
		}
	}	
//轴线归一
	for(i=0;i<m_nRows;i++)
	{
		if(m_pData[i*m_nCols+i]!=1.0)
		{
			RatioLine(i,1/m_pData[i*m_nCols+i]);
		}
	}
	
//整理右边的方阵为逆矩阵
	pNewMatrix=new double[m_nRows*m_nRows];
	for(i=0;i<m_nRows;i++)
	{
		for(j=0;j<m_nRows;j++)
		{
			pNewMatrix[i*m_nRows+j]=m_pData[i*m_nCols+j+m_nRows];
		}
	}

	delete[] m_pData;
	m_pData=pNewMatrix;
	m_nCols=m_nRows;
	return true;
}

CMatrix::CMatrix(CMatrix &M)
{
	m_nCols=0;
	m_nRows=0;
	m_pData=NULL;
	Clone(M);
}
void CMatrix::AddLineToLine(int nLine1, double ratio,int nLine2)
{
	for(int i=0;i<m_nCols;i++)
	{
		m_pData[nLine2*m_nCols+i]+=m_pData[nLine1*m_nCols+i]*ratio;
	}
}

void CMatrix::SwapLine(int nLine1, int nLine2)
{
	double swap;
	for(int i=0;i<m_nCols;i++)
	{
		swap=m_pData[nLine1*m_nCols+i];
		m_pData[nLine1*m_nCols+i]=m_pData[nLine2*m_nCols+i];
		m_pData[nLine2*m_nCols+i]=swap;
	}
}

void CMatrix::RatioLine(int nLine, double ratio)
{
	for(int i=0;i<m_nCols;i++)
	{
		m_pData[nLine*m_nCols+i]*=ratio;
	}
}

double CMatrix::operator [](int row)
{
	return 0;
}
}