/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

���������΢����Э�鷢����δ��������ɲ���Ӧ�����κ���ҵ�����

****************************************************************/
#include "../include/BiArray.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//����ʵ������ʽ�ļ�����ع���
namespace FCadLib{

CBiArray::CBiArray()
{
	m_pData=NULL;
	m_Degree=0;
}

CBiArray::~CBiArray()
{
	if(m_pData!=NULL) delete[] m_pData;
}

//��������ʽ����,ά��Ϊdegree,����������������degree
//pData��������degree*degree��
bool CBiArray::SetData(double *pData, int degree)
{
	if(degree<=0) return false;
	if(pData==NULL) return false;

	if(m_pData!=NULL) { delete[] m_pData; m_pData=NULL;}
	m_pData=new double[degree*degree];
	if(m_pData==NULL) return false;

	for(int i=0;i<degree;i++)
	{
		for(int j=0;j<degree;j++)
		{
			m_pData[i*degree+j]=pData[i*degree+j];
		}
	}
	m_Degree=degree;
	return true;
}

void CBiArray::operator *=(double x)
{
	if(m_pData==NULL) return;
	for(int i=0;i<m_Degree;i++)
	{
		for(int j=0;j<m_Degree;j++)
		{
			m_pData[i*m_Degree+j]*=x;
		}
	}
}

void CBiArray::operator /=(double x)
{
	if(m_pData==NULL) return;
	for(int i=0;i<m_Degree;i++)
	{
		for(int j=0;j<m_Degree;j++)
		{
			m_pData[i*m_Degree+j]/=x;
		}
	}
}

void CBiArray::operator -=(double x)
{
	if(m_pData==NULL) return;
	for(int i=0;i<m_Degree;i++)
	{
		for(int j=0;j<m_Degree;j++)
		{
			m_pData[i*m_Degree+j]-=x;
		}
	}
}

void CBiArray::operator +=(double x)
{
	if(m_pData==NULL) return;
	for(int i=0;i<m_Degree;i++)
	{
		for(int j=0;j<m_Degree;j++)
		{
			m_pData[i*m_Degree+j]+=x;
		}
	}
}

int CBiArray::GetDegree()
{
	return m_Degree;
}

double* CBiArray::GetData()
{
	return m_pData;
}

void CBiArray::operator +=(CBiArray &A)
{
	if(m_Degree!=A.GetDegree()) return ;
	if(m_Degree<=0) return ;
	double *pData=A.GetData();
	for(int i=0;i<m_Degree;i++)
	{
		for(int j=0;j<m_Degree;j++)
		{
			m_pData[i*m_Degree+j]+=pData[i*m_Degree+j];
		}
	}

}

void CBiArray::operator -=(CBiArray &A)
{
	if(m_Degree!=A.GetDegree()) return ;
	if(m_Degree<=0) return ;
	double *pData=A.GetData();
	for(int i=0;i<m_Degree;i++)
	{
		for(int j=0;j<m_Degree;j++)
		{
			m_pData[i*m_Degree+j]-=pData[i*m_Degree+j];
		}
	}
}

bool CBiArray::operator ==(CBiArray &A)
{

	if(m_Degree!=A.GetDegree()) return false;
	if(m_Degree==0) return true;
	double *pData=A.GetData();
	for(int i=0;i<m_Degree;i++)
	{
		for(int j=0;j<m_Degree;j++)
		{
			if(m_pData[i*m_Degree+j]!=pData[i*m_Degree+j])
				return false;
		}
	}
	return true;
}

bool CBiArray::operator !=(CBiArray &A)
{
	if(*this==A) return false;
	else return true;
}

CBiArray CBiArray::operator +(CBiArray &A)
{
	CBiArray newA;
	newA.Clone(*this);
	newA+=A;
	return newA;
}
//����һ��Ҫ����&A��ʽ,�����A��ʽ�ͻ�ı�A,��֪��Ϊʲô
void CBiArray::Clone(CBiArray &A)
{
	if(m_Degree==A.GetDegree())
	{
		if(m_Degree==0) return;
		//ASSERT(m_pData!=NULL);
	}
	else
	{
		m_Degree=A.GetDegree();

		if(m_pData!=NULL) {delete[] m_pData;m_pData=NULL;}
		
		if(m_Degree==0)  return ;
		
		m_pData=new double[m_Degree*m_Degree];
	}

	double *pData=A.GetData();

	for(int i=0;i<m_Degree;i++)
	{
		for(int j=0;j<m_Degree;j++)
		{
			m_pData[i*m_Degree+j]=pData[i*m_Degree+j];
		}
	}
}

CBiArray CBiArray::operator -(CBiArray &A)
{
	CBiArray newA;
	newA.Clone(*this);
	newA-=A;
	return newA;
}


//���ó��ȱ任�ķ���,��������ȫ���0����,��ͬ��_GetValue�Ļ�е����
double CBiArray::GetValue()
{
	if(m_Degree<=0) return 0;
	if(m_Degree==1) return m_pData[0];
	if(m_Degree==2) return m_pData[0]*m_pData[3]-m_pData[1]*m_pData[2];
//������Լ�һ�����εļ���
	int i,j;
	double outmulti=1;	//����ʽ�������
	
	for(i=0;i<m_Degree-1;i++)
	{
		//���ϱ�֤��Ϊ0
		if(m_pData[i*m_Degree+i]==0)	//�����Ƿ����õȺ�?
		{
			//���������ҵ�һ����Ϊ0����
			for(j=i+1;j<m_Degree;j++)
			{
				if(m_pData[j*m_Degree+i]!=0)
					break;
			}
			
			if(j==m_Degree) return 0;	//��һ��ȫ0
			SwapLine(i,j);
			outmulti=-outmulti;
		}
		
		//�����������Ѿ���Ϊ0,��i������ȫ���0
		for(j=i+1;j<m_Degree;j++)
		{
			if(m_pData[j*m_Degree+i]!=0)
			{
				double ratio=-(m_pData[j*m_Degree+i])/m_pData[i*m_Degree+i];
				AddLineToLine(i,ratio,j);
			}
		}
	}
	
	double Value=1;
	//�Խ����ϳ˻�
	for(i=0;i<m_Degree;i++)
	{
		Value*=m_pData[i*m_Degree+i];
	}
	return Value*outmulti;
}
//���г�����,�൱������ʽ������
void CBiArray::RatioLine(int nLine, double ratio)
{
	for(int i=0;i<m_Degree;i++)
	{
		m_pData[nLine*m_Degree+i]*=ratio;
	}
}
//��nLine1�г�ratio�ӵ�nLine2����ȥ,����ʽ����
void CBiArray::AddLineToLine(int nLine1, double ratio,int nLine2)
{
	for(int i=0;i<m_Degree;i++)
	{
		m_pData[nLine2*m_Degree+i]+=m_pData[nLine1*m_Degree+i]*ratio;
	}
}
//����������,��������ʽ����,����ʽ����
void CBiArray::SwapLine(int nLine1, int nLine2)
{
	double swap;
	for(int i=0;i<m_Degree;i++)
	{
		swap=m_pData[nLine1*m_Degree+i];
		m_pData[nLine1*m_Degree+i]=m_pData[nLine2*m_Degree+i];
		m_pData[nLine2*m_Degree+i]=swap;
	}
}
}