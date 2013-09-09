#include "CLProtocolDataPoster.h"
#include "CLEvent.h"
#include "CLPostResultNotifier.h"
#include "CLDataPoster.h"
#include "CLLogger.h"

CLProtocolDataPoster::CLProtocolDataPoster(CLDataPoster *pDataPoster, CLPostResultNotifier *pResultNotifier, CLEvent *pEvent)
{
	m_pDataPoster = pDataPoster;
	m_pEvent = pEvent;
	m_pResultNotifer = pResultNotifier;
	m_pIOVectors = 0;
}

CLProtocolDataPoster::~CLProtocolDataPoster()
{
}

CLStatus CLProtocolDataPoster::Initialize()
{
	CLStatus s = m_pDataPoster->Initialize();
	if(!s.IsSuccess() && s.m_clErrorCode == POSTER_INITIALIZE_PENDING)
		return s;

	bool result = true;
	if(!s.IsSuccess())
	{
		result = false;
		CLLogger::WriteLogMsg("In CLProtocolDataPoster::Initialize(), m_pDataPoster->Initialize error", 0);
	}

	if(m_pResultNotifer)
	{
		CLStatus s1 = m_pResultNotifer->Notify(result);
		if(!s1.IsSuccess())
			CLLogger::WriteLogMsg("In CLProtocolDataPoster::Initialize(), m_pResultNotifer->Notify error", 0);

		delete m_pResultNotifer;
	}

	delete this;

	return s;
}

CLStatus CLProtocolDataPoster::NotifyIntialFinished(bool bResult)
{
	if(m_pResultNotifer)
	{
		CLStatus s = m_pResultNotifer->Notify(bResult);
		if(!s.IsSuccess())
			CLLogger::WriteLogMsg("In CLProtocolDataPoster::NotifyIntialFinished(), m_pResultNotifer->Notify error", 0);

		delete m_pResultNotifer;
	}

	delete this;

	return CLStatus(0, 0);
}

CLStatus CLProtocolDataPoster::Uninitialize()
{
	CLStatus s = m_pDataPoster->Uninitialize();
	if(!s.IsSuccess())
		CLLogger::WriteLogMsg("In CLProtocolDataPoster::Uninitialize(), m_pDataPoster->Uninitialize error", 0);
	
	delete this;
	
	return s;
}

CLStatus CLProtocolDataPoster::PostProtocolData(CLIOVectors *pIOVectors)
{
	CLStatus s = m_pDataPoster->PostData(pIOVectors);
	if(!s.IsSuccess() && s.m_clErrorCode == POSTER_POST_PENDING)
	{
		m_pIOVectors = pIOVectors;
		return s;
	}

	bool result = true;
	if(s.IsSuccess())
	{
		if(m_pEvent)
		{
			CLStatus s1 = m_pEvent->Set();
			if(!s1.IsSuccess())
			{
				result = false;
				CLLogger::WriteLogMsg("In CLProtocolDataPoster::PostProtocolData(), m_pEvent->Set error", 0);
			}
		}
	}
	else
	{
		result = false;
		CLLogger::WriteLogMsg("In CLProtocolDataPoster::PostProtocolData(), m_pDataPoster->PostData error", 0);
	}

	if(m_pResultNotifer)
	{
		CLStatus s2 = m_pResultNotifer->Notify(result);
		if(!s2.IsSuccess())
			CLLogger::WriteLogMsg("In CLProtocolDataPoster::PostProtocolData(), m_pResultNotifer->Notify error", 0);

		delete m_pResultNotifer;
	}

	delete pIOVectors;

	delete this;

	return s;
}

CLStatus CLProtocolDataPoster::NotifyPostFinished(bool bResult)
{
	if(m_pEvent)
	{
		CLStatus s = m_pEvent->Set();
		if(!s.IsSuccess())
		{
			bResult = false;
			CLLogger::WriteLogMsg("In CLProtocolDataPoster::NotifyPostFinished(), m_pEvent->Set error", 0);
		}
	}

	if(m_pResultNotifer)
	{
		CLStatus s1 = m_pResultNotifer->Notify(bResult);
		if(!s1.IsSuccess())
			CLLogger::WriteLogMsg("In CLProtocolDataPoster::NotifyPostFinished(), m_pResultNotifer->Notify error", 0);

		delete m_pResultNotifer;
	}

	if(m_pIOVectors)
		delete m_pIOVectors;

	delete this;

	return CLStatus(0, 0);
}