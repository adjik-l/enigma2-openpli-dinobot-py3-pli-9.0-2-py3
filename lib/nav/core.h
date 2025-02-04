#ifndef __nav_core_h
#define __nav_core_h

#include <lib/base/object.h>
#include <lib/service/iservice.h>
#include <connection.h>
#include <map>
#include <set>
#if defined(HAVE_FCC_ABILITY)
#include <lib/dvb/fcc.h>
#endif

class eNavigation: public iObject, public sigc::trackable
{
	static eNavigation *instance;
	DECLARE_REF(eNavigation);
	int m_decoder;
	ePtr<iServiceHandler> m_servicehandler;

	ePtr<iPlayableService> m_runningService;
#if SIGCXX_MAJOR_VERSION == 3
	sigc::signal<void(int)> m_event;
#else
	sigc::signal1<void,int> m_event;
#endif
	ePtr<eConnection> m_service_event_conn;
	void serviceEvent(iPlayableService* service, int event);

	std::map<ePtr<iRecordableService>, ePtr<eConnection>, std::less<iRecordableService*> > m_recordings;
	std::map<ePtr<iRecordableService>, eServiceReference, std::less<iRecordableService*> > m_recordings_services;
	std::set<ePtr<iRecordableService>, std::less<iRecordableService*> > m_simulate_recordings;

#if SIGCXX_MAJOR_VERSION == 3
	sigc::signal<void(ePtr<iRecordableService>,int)> m_record_event;
#else
	sigc::signal2<void,ePtr<iRecordableService>,int> m_record_event;
#endif
	void recordEvent(iRecordableService* service, int event);
#if defined(HAVE_FCC_ABILITY)
	friend class eFCCServiceManager;
	ePtr<eFCCServiceManager> m_fccmgr;
#endif
public:

	RESULT playService(const eServiceReference &service);
#if SIGCXX_MAJOR_VERSION == 3
	RESULT connectEvent(const sigc::slot<void(int)> &event, ePtr<eConnection> &connection);
	RESULT connectRecordEvent(const sigc::slot<void(ePtr<iRecordableService>,int)> &event, ePtr<eConnection> &connection);
/*	int connectServiceEvent(const sigc::slot<void(iPlayableService*,int> &event, ePtr<eConnection)> &connection); */
#else
	RESULT connectEvent(const sigc::slot1<void,int> &event, ePtr<eConnection> &connection);
	RESULT connectRecordEvent(const sigc::slot2<void,ePtr<iRecordableService>,int> &event, ePtr<eConnection> &connection);
/*	int connectServiceEvent(const sigc::slot1<void,iPlayableService*,int> &event, ePtr<eConnection> &connection); */
#endif
	RESULT getCurrentService(ePtr<iPlayableService> &service);
	RESULT stopService(void);

	RESULT recordService(const eServiceReference &ref, ePtr<iRecordableService> &service, bool simulate=false);
	RESULT stopRecordService(ePtr<iRecordableService> &service);
	void getRecordings(std::vector<ePtr<iRecordableService> > &recordings, bool simulate=false);
	std::map<ePtr<iRecordableService>, eServiceReference, std::less<iRecordableService*> > getRecordingsServices() { return m_recordings_services; }

	RESULT pause(int p);
	eNavigation(iServiceHandler *serviceHandler, int decoder = 0);
	static eNavigation *getInstance() { return instance; }
	virtual ~eNavigation();
};

#endif
