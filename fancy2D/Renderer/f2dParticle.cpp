#include "f2dParticle.h"

#include "f2dSpriteImpl.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

f2dParticlePoolImpl::~f2dParticlePoolImpl()
{
	Clear();
}

fResult f2dParticlePoolImpl::Emitted(f2dSprite* pSprite, const fcyVec2& Center, const fcyVec2& EmittedCountRange, const f2dParticleCreationDesc& Desc)
{
	if(!pSprite)
		return FCYERR_INVAILDPARAM;

	// �������Ӵ�������
	fInt tCreateCount = (fInt)m_Randomizer.GetRandFloat(EmittedCountRange.x, EmittedCountRange.y);

	// ��������
	fuInt tPos = 0;
	for(int i = 0; i<tCreateCount; ++i)
	{
		// ���㴴��λ��
		Particle tParticle;

		tParticle.CurTime = 0;
		tParticle.Angle = 0;

		tParticle.Pos = Center;
		tParticle.Pos.x += m_Randomizer.GetRandFloat(Desc.PosRange.a.x, Desc.PosRange.b.x);
		tParticle.Pos.y += m_Randomizer.GetRandFloat(Desc.PosRange.a.y, Desc.PosRange.b.y);
		
		// ���㷨��
		tParticle.CreatePos = Center;
		fcyVec2 tNormal = tParticle.Pos - tParticle.CreatePos;
		if(tNormal.Length2() != 0.f)
			tNormal.Normalize();

		tParticle.V = tNormal * m_Randomizer.GetRandFloat(Desc.VRange.x, Desc.VRange.y);
		tParticle.RA = m_Randomizer.GetRandFloat(Desc.ARRange.x, Desc.ARRange.y);
		tParticle.TA = m_Randomizer.GetRandFloat(Desc.ATRange.x, Desc.ATRange.y);
		tParticle.Spin = m_Randomizer.GetRandFloat(Desc.SpinRange.x, Desc.SpinRange.y);
		tParticle.LifeTime = m_Randomizer.GetRandFloat(Desc.LifeTimeRange.x, Desc.LifeTimeRange.y);

		tParticle.RandomSeed = m_Randomizer.GetRandFloat();

		fInt tA = Desc.StartColor.a + (fInt)m_Randomizer.GetRandFloat(Desc.StartColorRange.x, Desc.StartColorRange.y);
		fInt tR = Desc.StartColor.r + (fInt)m_Randomizer.GetRandFloat(Desc.StartColorRange.x, Desc.StartColorRange.y);
		fInt tG = Desc.StartColor.g + (fInt)m_Randomizer.GetRandFloat(Desc.StartColorRange.x, Desc.StartColorRange.y);
		fInt tB = Desc.StartColor.b + (fInt)m_Randomizer.GetRandFloat(Desc.StartColorRange.x, Desc.StartColorRange.y);
		if(tA>255)tA = 255; if(tA<0) tA=0;
		if(tR>255)tR = 255; if(tR<0) tR=0;
		if(tG>255)tG = 255; if(tG<0) tG=0;
		if(tB>255)tB = 255; if(tB<0) tB=0;
		tParticle.StartColor = fcyColor(tA,tR,tG,tB);

		tParticle.CurColor = tParticle.StartColor;
		tParticle.EndColor = Desc.EndColor;
		
		float tV = m_Randomizer.GetRandFloat(Desc.StartScaleRange.x, Desc.StartScaleRange.y);
		tParticle.StartScale = Desc.StartScale + fcyVec2(tV,tV);
		if(tParticle.StartScale.x < 0) tParticle.StartScale.x = 0;
		if(tParticle.StartScale.y < 0) tParticle.StartScale.y = 0;
		tParticle.CurScale = tParticle.StartScale;
		tParticle.EndScale = Desc.EndScale;

		tParticle.pSprite = pSprite;
		tParticle.pSprite->AddRef();

		tParticle.bInUse = true;
		
		// ��������
		while(tPos < m_ParticlePool.size())
		{
			if(m_ParticlePool[tPos].bInUse)
				tPos++;
			else
			{
				m_ParticlePool[tPos] = tParticle;
				break;
			}
		}
		
		if(tPos >= m_ParticlePool.size())
			m_ParticlePool.push_back(tParticle);
	}

	return FCYERR_OK;
}

void f2dParticlePoolImpl::Update(fFloat ElapsedTime)
{
	// ������������
	vector<Particle>::iterator i = m_ParticlePool.begin();

	while(i != m_ParticlePool.end())
	{
		// ��������ʱ��
		i->CurTime += ElapsedTime;
		if(i->LifeTime < i->CurTime)
		{
			// �Ƴ�����
			FCYSAFEKILL(i->pSprite);

			i->bInUse = false;
			++i;
		}
		else
		{
			float k = i->CurTime / i->LifeTime;

			// ����λ��
			fcyVec2 tRA, tTA;  // ������ٶȣ�������ٶ�

			// ���㷨��
			fcyVec2 tNormal = i->Pos - i->CreatePos;
			if(tNormal.Length2() != 0.f)
			{
				tNormal.Normalize();
				tRA = tNormal * i->RA;

				// ��ת90��
				float t = tNormal.x;
				tNormal.x = -tNormal.y;
				tNormal.y = t;

				// ������ٶ�
				tTA = tNormal * i->TA;
			}

			// �ٶ�����
			i->V += (tRA + tTA) * ElapsedTime;

			// ��������
			i->Angle += i->Spin * ElapsedTime;

			// λ������
			i->Pos += i->V * ElapsedTime;
			
			// ������ɫ
			i->CurColor = fcyColor
				(
					(fInt)((i->EndColor.a - i->StartColor.a) * k + i->StartColor.a),
					(fInt)((i->EndColor.r - i->StartColor.r) * k + i->StartColor.r),
					(fInt)((i->EndColor.g - i->StartColor.g) * k + i->StartColor.g),
					(fInt)((i->EndColor.b - i->StartColor.b) * k + i->StartColor.b)
				);
			// ��������
			i->CurScale = fcyVec2
				(
					(i->EndScale.x - i->StartScale.x) * k + i->StartScale.x,
					(i->EndScale.y - i->StartScale.y) * k + i->StartScale.y
				);

			++i;
		}
	}
}
	
void f2dParticlePoolImpl::Render(f2dGraphics2D* pGraph)
{
	// ��Ⱦ��������
	vector<Particle>::iterator i = m_ParticlePool.begin();

	while(i != m_ParticlePool.end())
	{
		if(i->bInUse && i->pSprite)
		{
			fcyColor t[4];
			i->pSprite->GetColor(t);

			i->pSprite->SetColor(i->CurColor);
			
			if(i->Angle != 0.f)
				i->pSprite->Draw(pGraph, i->Pos, i->CurScale, i->Angle);
			else
				i->pSprite->Draw(pGraph, i->Pos, i->CurScale);
			
			i->pSprite->SetColor(t);
		}

		++i;
	}
}