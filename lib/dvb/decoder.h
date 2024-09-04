#ifndef __decoder_h
#define __decoder_h

#include <lib/base/object.h>
#include <lib/dvb/demux.h>
#ifdef HAVE_RASPBERRYPI
#include <omx.h>
#include <lib/dvb/omxdecoder.h>
#endif

class eSocketNotifier;

class eDVBAudio: public iObject
{
	DECLARE_REF(eDVBAudio);
private:
	ePtr<eDVBDemux> m_demux;
	int m_fd, m_fd_demux, m_dev, m_is_freezed;
public:
	enum { aMPEG, aAC3, aDTS, aAAC, aAACHE, aLPCM, aDTSHD, aDDP, aDRA, aAC4 };
	eDVBAudio(eDVBDemux *demux, int dev);
	enum { aMonoLeft, aStereo, aMonoRight };
	void setChannel(int channel);
	void stop();
#ifdef HAVE_RASPBERRYPI
	int startPid(int pid, int type, bool mode);
#else
	int startPid(int pid, int type);
#endif
	void flush();
	void freeze();
	void unfreeze();
	int getPTS(pts_t &now);
	virtual ~eDVBAudio();
#if defined(DMAMLOGIC)
	void setSTCValidState(int state);
#endif
};

class eDVBVideo: public iObject, public sigc::trackable
{
	DECLARE_REF(eDVBVideo);
private:
#ifdef HAVE_RASPBERRYPI
	cOmx *m_omx;
#endif
	ePtr<eDVBDemux> m_demux;
	int m_fd, m_fd_demux, m_dev;
#if defined(HAVE_FCC_ABILITY)
	bool m_fcc_enable;
#endif
	static int m_close_invalidates_attributes;
	int m_is_slow_motion, m_is_fast_forward, m_is_freezed;
	ePtr<eSocketNotifier> m_sn;
	void video_event(int what);
#if defined(DMAMLOGIC)
	int m_fd_amvideoPoll;
	ePtr<eSocketNotifier> m_sn_amvideoPoll;
	void amvideo_event(int);
#endif
#if SIGCXX_MAJOR_VERSION == 3
	sigc::signal<void(struct iTSMPEGDecoder::videoEvent)> m_event;
#else
	sigc::signal1<void, struct iTSMPEGDecoder::videoEvent> m_event;
#endif
	int m_width, m_height, m_framerate, m_aspect, m_progressive, m_gamma;
	static int readApiSize(int fd, int &xres, int &yres, int &aspect);
public:
	enum { UNKNOWN = -1, MPEG2, MPEG4_H264, VC1 = 3, MPEG4_Part2, VC1_SM, MPEG1, H265_HEVC, AVS = 16, AVS2 = 40 };
#if defined(HAVE_FCC_ABILITY)
	eDVBVideo(eDVBDemux *demux, int dev, bool fcc_enable=false);
#else
	eDVBVideo(eDVBDemux *demux, int dev);
#endif
	void stop();
#ifdef HAVE_RASPBERRYPI
	int startPid(int pid, int type=MPEG2, bool is_pvr=false);
#else
	int startPid(int pid, int type=MPEG2);
#endif
	void flush();
	void freeze();
	int setSlowMotion(int repeat);
	int setFastForward(int skip);
	void unfreeze();
	int getPTS(pts_t &now);
	virtual ~eDVBVideo();
#if SIGCXX_MAJOR_VERSION == 3
	RESULT connectEvent(const sigc::slot<void(struct iTSMPEGDecoder::videoEvent)> &event, ePtr<eConnection> &conn);
#else
	RESULT connectEvent(const sigc::slot1<void, struct iTSMPEGDecoder::videoEvent> &event, ePtr<eConnection> &conn);
#endif
	int getWidth();
	int getHeight();
	int getProgressive();
	int getFrameRate();
	int getAspect();
	int getGamma();
};

class eDVBPCR: public iObject
{
	DECLARE_REF(eDVBPCR);
private:
	ePtr<eDVBDemux> m_demux;
	int m_fd_demux, m_dev;
public:
	eDVBPCR(eDVBDemux *demux, int dev);
	int startPid(int pid);
	void stop();
#if defined(DMAMLOGIC)
	void restart();
#endif
	virtual ~eDVBPCR();
};

class eDVBTText: public iObject
{
	DECLARE_REF(eDVBTText);
private:
	ePtr<eDVBDemux> m_demux;
	int m_fd_demux, m_dev;
public:
	eDVBTText(eDVBDemux *demux, int dev);
	int startPid(int pid);
	void stop();
	virtual ~eDVBTText();
};

class eTSMPEGDecoder: public sigc::trackable, public iTSMPEGDecoder
{
	DECLARE_REF(eTSMPEGDecoder);
private:
	static int m_pcm_delay;
	static int m_ac3_delay;
	static int m_audio_channel;
	std::string m_radio_pic;
	ePtr<eDVBDemux> m_demux;
	ePtr<eDVBAudio> m_audio;
	ePtr<eDVBVideo> m_video;
	ePtr<eDVBPCR> m_pcr;
	ePtr<eDVBTText> m_text;
#ifdef HAVE_RASPBERRYPI
	int m_vpid, m_vtype, m_apid, m_atype, m_pcrpid, m_textpid, m_vstreamtype;
	bool m_is_pvr, m_is_radio;
	ePlayMode m_playmode;
#else
	int m_vpid, m_vtype, m_apid, m_atype, m_pcrpid, m_textpid;
#endif
#if defined(HAVE_FCC_ABILITY)
	int m_fcc_fd;
	bool m_fcc_enable;
	int m_fcc_state;
	int m_fcc_feid;
	int m_fcc_vpid;
	int m_fcc_vtype;
	int m_fcc_pcrpid;
#endif
	enum
	{
		changeVideo = 1,
		changeAudio = 2,
		changePCR   = 4,
		changeText  = 8,
		changeState = 16,
	};
	int m_changed, m_decoder;
	int m_state;
	int m_ff_sm_ratio;
	bool m_has_audio;
	int setState();
	ePtr<eConnection> m_demux_event_conn;
	ePtr<eConnection> m_video_event_conn;

	void demux_event(int event);
	void video_event(struct videoEvent);
#if SIGCXX_MAJOR_VERSION == 3
	sigc::signal<void(struct videoEvent)> m_video_event;
#else
	sigc::signal1<void, struct videoEvent> m_video_event;
#endif
#if defined(DMAMLOGIC)
#if SIGCXX_MAJOR_VERSION == 3
	sigc::signal<void(int)> m_state_event;
#else
	sigc::signal1<void, int> m_state_event;
#endif
#endif
	int m_video_clip_fd;
	ePtr<eTimer> m_showSinglePicTimer;
	void finishShowSinglePic(); // called by timer
public:
	enum { pidNone = -1 };
	eTSMPEGDecoder(eDVBDemux *demux, int decoder);
	virtual ~eTSMPEGDecoder();
#ifdef HAVE_RASPBERRYPI
	RESULT setVideoPID(int vpid, int type, int streamtype);
	RESULT setAudioPID(int apid, int type, bool amode);
#else
	RESULT setVideoPID(int vpid, int type);
	RESULT setAudioPID(int apid, int type);
#endif
	RESULT setAudioChannel(int channel);
	int getAudioChannel();
	RESULT setPCMDelay(int delay);
	int getPCMDelay() { return m_pcm_delay; }
	RESULT setAC3Delay(int delay);
	int getAC3Delay() { return m_ac3_delay; }
	RESULT setSyncPCR(int pcrpid);
	RESULT setTextPID(int textpid);
	RESULT setSyncMaster(int who);

		/*
		The following states exist:

		 - stop: data source closed, no playback
		 - pause: data source active, decoder paused
		 - play: data source active, decoder consuming
		 - decoder fast forward: data source linear, decoder drops frames
		 - trickmode, highspeed reverse: data source fast forwards / reverses, decoder just displays frames as fast as it can
		 - slow motion: decoder displays frames multiple times
		*/
	enum {
		stateStop,
		statePause,
		statePlay,
		stateDecoderFastForward,
		stateTrickmode,
		stateSlowMotion
	};
	RESULT set(); /* just apply settings, keep state */
	RESULT play(); /* -> play */
	RESULT pause(); /* -> pause */
	RESULT setFastForward(int frames_to_skip); /* -> decoder fast forward */
	RESULT setSlowMotion(int repeat); /* -> slow motion **/
	RESULT setTrickmode(); /* -> highspeed fast forward */

	RESULT flush();
	RESULT showSinglePic(const char *filename);
	RESULT setRadioPic(const std::string &filename);
		/* what 0=auto, 1=video, 2=audio. */
	RESULT getPTS(int what, pts_t &pts);
#if SIGCXX_MAJOR_VERSION == 3
	RESULT connectVideoEvent(const sigc::slot<void(struct videoEvent)> &event, ePtr<eConnection> &connection);
#else
	RESULT connectVideoEvent(const sigc::slot1<void, struct videoEvent> &event, ePtr<eConnection> &connection);
#endif
#if defined(DMAMLOGIC)
#if SIGCXX_MAJOR_VERSION == 3
	RESULT connectStateEvent(const sigc::slot<void(int)> &event, ePtr<eConnection> &connection);
#else
	RESULT connectStateEvent(const sigc::slot1<void, int> &event, ePtr<eConnection> &connection);
#endif
	int getVideoDecoderId();
#endif
	int getVideoWidth();
	int getVideoHeight();
	int getVideoProgressive();
	int getVideoFrameRate();
	int getVideoAspect();
	int getVideoGamma();
#if defined(DMAMLOGIC)
	int getState();
	const char* getEotf();
#endif
	static RESULT setHwPCMDelay(int delay);
	static RESULT setHwAC3Delay(int delay);
#if defined(HAVE_FCC_ABILITY)
	enum 
	{
		fcc_state_stop,
		fcc_state_ready,
		fcc_state_decoding
	};

	RESULT prepareFCC(int fe_id, int vpid, int vtype, int pcrpid);
	RESULT fccStart();
	RESULT fccStop();
	RESULT fccDecoderStart();
	RESULT fccDecoderStop();
	RESULT fccUpdatePids(int fe_id, int vpid, int vtype, int pcrpid);
	RESULT fccSetPids(int fe_id, int vpid, int vtype, int pcrpid);
	RESULT fccGetFD();
	RESULT fccFreeFD();
#endif
};

#endif
