/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under MsPL.

本代码采用微软公共协议发布，未经作者许可不能应用于任何商业软件。

****************************************************************/
#include "../include/BiArray.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//本类实现行列式的计算相关功能
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

//设置行列式数据,维度为degree,即行数和列数等于degree
//pData共有数据degree*degree个
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
//参数一定要采用&A形式,如果是A形式就会改变A,不知道为什么
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


//采用初等变换的方法,把下三角全变成0计算,不同于_GetValue的机械方法
double CBiArray::GetValue()
{
	if(m_Degree<=0) return 0;
	if(m_Degree==1) return m_pData[0];
	if(m_Degree==2) return m_pData[0]*m_pData[3]-m_pData[1]*m_pData[2];
//这里可以加一个三次的计算
	int i,j;
	double outmulti=1;	//行列式外的因子
	
	for(i=0;i<m_Degree-1;i++)
	{
		//左上保证不为0
		if(m_pData[i*m_Degree+i]==0)	//这里是否能用等号?
		{
			//按行向下找第一个不为0的行
			for(j=i+1;j<m_Degree;j++)
			{
				if(m_pData[j*m_Degree+i]!=0)
					break;
			}
			
			if(j==m_Degree) return 0;	//第一列全0
			SwapLine(i,j);
			outmulti=-outmulti;
		}
		
		//到这里左上已经不为0,把i列下面全变成0
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
	//对角线上乘积
	for(i=0;i<m_Degree;i++)
	{
		Value*=m_pData[i*m_Degree+i];
	}
	return Value*outmulti;
}
//对行乘因子,相当于行列式乘因子
void CBiArray::RatioLine(int nLine, double ratio)
{
	for(int i=0;i<m_Degree;i++)
	{
		m_pData[nLine*m_Degree+i]*=ratio;
	}
}
//把nLine1行乘ratio加到nLine2行上去,行列式不变
void CBiArray::AddLineToLine(int nLine1, double ratio,int nLine2)
{
	for(int i=0;i<m_Degree;i++)
	{
		m_pData[nLine2*m_Degree+i]+=m_pData[nLine1*m_Degree+i]*ratio;
	}
}
//两行做交换,交换行列式两行,行列式反号
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