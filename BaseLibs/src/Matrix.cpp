/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢����Э�鷢����δ��������ɲ���Ӧ�����κ���ҵ�����

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

	double *pNewData=new double[nNewRows*nNewCols];	//�µ����ݴ洢��

	for(int i=0;i<nNewRows;i++)
	{
		for(int j=0;j<nNewCols;j++)
		{
			double cell=0;	//�¾���ĵ�λֵ
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
//���б任�ķ������,����n*2n�ľ���,���ұ����óɵ�λ����
//ͨ���б任����߱�ɵ�λ�������ұ߾��������
bool CMatrix::Reverse()
{
	//���������Ƿ���
	if(m_nCols!=m_nRows) return false;
	if(m_pData==NULL) return false;
	if(m_nCols==1) 
	{
		m_pData[0]=1/m_pData[0];
		return true;
	}

	int i,j;
	double *pNewMatrix=new double[m_nRows*2*m_nCols];
	//����ԭʼ����
	for(i=0;i<m_nRows;i++)
	{
		for(j=0;j<m_nCols;j++)
		{
			//����ԭʼ����
			pNewMatrix[i*(2*m_nCols)+j]=m_pData[i*m_nCols+j];
			//�����ұ�Ϊ��λ����
			if(j==i) pNewMatrix[i*(2*m_nCols)+j+m_nCols]=1;
			else pNewMatrix[i*(2*m_nCols)+j+m_nCols]=0;
		}
	}
	
	delete[] m_pData;
	m_pData=pNewMatrix;
	m_nCols=2*m_nCols;
//�任,�Ȱ���������Ǳ�0
	for(i=0;i<m_nRows-1;i++)
	{
		//���ϱ�֤��Ϊ0
		if(m_pData[i*m_nCols+i]==0)	//�����Ƿ����õȺ�?
		{
			//���������ҵ�һ����Ϊ0����
			for(j=i+1;j<m_nRows;j++)
			{
				if(m_pData[j*m_nCols+i]!=0)
					break;
			}
			
			if(j==m_nRows) return false;	//�����Ҳ���һ����Ϊ0��

			SwapLine(i,j);

		}
		
		//�����������Ѿ���Ϊ0,��i������ȫ���0
		for(j=i+1;j<m_nRows;j++)
		{
			if(m_pData[j*m_nCols+i]!=0)
			{
				double ratio=-(m_pData[j*m_nCols+i])/m_pData[i*m_nCols+i];
				AddLineToLine(i,ratio,j);
			}
		}
	}

//������������Ϊ0,���������������Ϊ0
	//�����һ�п�ʼѭ�����ڶ������ϵ���
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
//���߹�һ
	for(i=0;i<m_nRows;i++)
	{
		if(m_pData[i*m_nCols+i]!=1.0)
		{
			RatioLine(i,1/m_pData[i*m_nCols+i]);
		}
	}
	
//�����ұߵķ���Ϊ�����
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