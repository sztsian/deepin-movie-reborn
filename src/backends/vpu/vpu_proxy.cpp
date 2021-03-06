#include "config.h"

#include "vpu_proxy.h"
#include "vpu_decoder.h"
#if defined (__cplusplus)
extern "C" {
#endif

#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libavresample/avresample.h>

#define DEBUG

#if defined (__cplusplus)
}
#endif
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

namespace dmr {

VpuProxy::VpuProxy(QWidget *parent)
    :Backend(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    //setAttribute(Qt::WA_PaintOnScreen);
    setState(PlayState::Stopped);
    _lastFrame.data = 0;
}

VpuProxy::~VpuProxy()
{
    stop();
}

void VpuProxy::resizeEvent(QResizeEvent *re)
{
    if (_d) {
        _d->videoThread()->updateViewportSize(size());
    }
}

void VpuProxy::closeEvent(QCloseEvent *ce)
{
    qDebug() << __func__;
    stop();
    ce->accept();
}

void VpuProxy::paintEvent(QPaintEvent *pe)
{
    if (_lastFrame.data) {
        QPainter p(this);
        auto img = QImage(_lastFrame.data, _lastFrame.width,
                qMin(_lastFrame.height, height()), QImage::Format_RGB32);
        p.drawImage(0, 0, img);
        p.end();

        free(_lastFrame.data);
        _lastFrame.data = 0;
    }
}

void VpuProxy::video_refresh_timer() 
{
    static int drop_count = 0;

    double actual_delay, delay, sync_threshold, ref_clock, diff;

    if (_reqQuit || _d == nullptr || _d->isFinished()) 
        return;

    if(_d->frames().size() == 0) {
        QTimer::singleShot(10, this, &VpuProxy::video_refresh_timer);
    } else {
        auto vp = _d->frames().deque();
        _lastFrame = vp;

        delay = vp.pts - _frameLastPts; /* the pts from last time */
        if(delay <= 0 || delay >= 1.0) {
            /* if incorrect delay, use previous one */
            delay = _frameLastDelay;
        }
        /* save for next time */
        _frameLastDelay = delay;
        _frameLastPts = vp.pts;

        /* update delay to sync to audio */
        //ref_clock = get_audio_clock(is);
        ref_clock = _d->getClock();
        diff = vp.pts - ref_clock;

        /* Skip or repeat the frame. Take delay into account
           FFPlay still doesn't "know if this is the best guess." */
        sync_threshold = (delay > AV_SYNC_THRESHOLD) ? delay : AV_SYNC_THRESHOLD;
        if(fabs(diff) < AV_NOSYNC_THRESHOLD) {
            if(diff <= -sync_threshold) {
                delay = 0;
            } else if(diff >= sync_threshold) {
                delay = 2 * delay;
            }
        }

        _frameTimer += delay;
        /* computer the REAL delay */
        actual_delay = _frameTimer - (av_gettime() / 1000000.0);
        if(actual_delay < 0.010) {
            /* Really it should skip the picture instead */
            actual_delay = 0.010;
        }

#ifdef DEBUG
        fprintf(stderr, "%s: audio clock %f, vp.pts %f, delay %f, diff %f, actual_delay %f, _frameTimer %f\n",
                __func__, ref_clock, vp.pts, delay, diff, actual_delay, _frameTimer);
#endif

        QTimer::singleShot((int)(actual_delay * 1000 + 0.5), this, &VpuProxy::video_refresh_timer);
        
        this->update();
        _elapsed = vp.pts;
        emit elapsedChanged();
    }
}

void VpuProxy::setPlayFile(const QFileInfo& fi)
{
    _file = fi;

    if (_d == nullptr) {
        _d = new VpuMainThread(fi.absoluteFilePath());
        _d->videoThread()->updateViewportSize(size());

        connect(_d->audioThread(), &AudioDecoder::muteChanged, this, &Backend::muteChanged);
        connect(_d->audioThread(), &AudioDecoder::volumeChanged, this, &Backend::volumeChanged);
    }

    updatePlayingMovieInfo();
    emit tracksChanged();
    emit fileLoaded();
    fprintf(stderr, "%s\n", __func__);
}


void VpuProxy::setState(PlayState s)
{
    if (_state != s) {
        _state = s;
        emit stateChanged();
    }
}

const PlayingMovieInfo& VpuProxy::playingMovieInfo()
{
    return _pmf;
}

void VpuProxy::loadSubtitle(const QFileInfo& fi)
{
    if (state() == PlayState::Stopped) {
        return;
    }

}

bool VpuProxy::isSubVisible()
{
    return true;
}

void VpuProxy::toggleSubtitle()
{
    if (state() == PlayState::Stopped) {
        return;
    }

}

void VpuProxy::volumeUp()
{
    if (_d) {
        auto ad = _d->audioThread();
        auto vol = qMin(1.0, ad->getVolume() + 0.1);
        ad->setVolume(vol);
    }
}

void VpuProxy::changeVolume(int val)
{
    if (_d) {
        auto ad = _d->audioThread();
        qreal vol = qMin(qMax(val, 0), 100) / 100.0;
        ad->setVolume(vol);
    }
}

void VpuProxy::volumeDown()
{
    if (_d) {
        auto ad = _d->audioThread();
        auto vol = qMax(0.0, ad->getVolume() - 0.1);
        ad->setVolume(vol);
    }
}

int VpuProxy::volume() const
{
    auto vol = _d ? _d->audioThread()->getVolume() : 1.0;
    fprintf(stderr, "%s: vol = %f\n", __func__, vol);
    return vol * 100;
}

bool VpuProxy::muted() const
{
    return _d ? _d->audioThread()->isMuted() : false;
}

void VpuProxy::toggleMute()
{
    if (_d) _d->audioThread()->setMute(!muted());
}

void VpuProxy::play()
{
    if (!_d) return;

    _d->start();

    setState(PlayState::Playing);
    _frameTimer = (double)av_gettime() / 1000000.0;
    _frameLastDelay = 40e-3;

    pid_t tid = syscall(SYS_gettid);
    fprintf(stderr, "VpuProxy tid %d\n", tid);

    video_refresh_timer();
}

void VpuProxy::pauseResume()
{
    if (_state == PlayState::Stopped)
        return;
}

void VpuProxy::stop()
{
    fprintf(stderr, "VpuProxy stop\n");
    if (_d && !_reqQuit) {
        _reqQuit = true;
        setState(PlayState::Stopped);
        disconnect(_d, 0, 0, 0);
        _d->stop();
        int tries = 10;
        while (tries--) 
            _d->wait(1000);

        delete _d;
        _d = 0;

        if (_lastFrame.data) {
            free(_lastFrame.data);
            _lastFrame.data = 0;
        }
    }

}

QImage VpuProxy::takeScreenshot()
{
    return QImage();
}

void VpuProxy::burstScreenshot()
{
}

void VpuProxy::stopBurstScreenshot()
{
}

void VpuProxy::seekForward(int secs)
{
    if (state() == PlayState::Stopped) return;

    _d->seekForward(secs);
}

void VpuProxy::seekBackward(int secs)
{
    if (state() == PlayState::Stopped) return;

    _d->seekBackward(secs);
}

qint64 VpuProxy::duration() const
{
    return _d?_d->duration():0;
}


qint64 VpuProxy::elapsed() const
{
    return _elapsed;
}

void VpuProxy::updatePlayingMovieInfo()
{
}
}
