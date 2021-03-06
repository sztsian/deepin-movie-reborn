#ifndef _DMR_TOOLBOX_PROXY_H
#define _DMR_TOOLBOX_PROXY_H 

#include <DPlatformWindowHandle>
#include <QtWidgets>

namespace Dtk
{
namespace Widget
{
    class DImageButton;
}
}

DWIDGET_USE_NAMESPACE

namespace dmr {
class PlayerEngine;
class VolumeButton;
class MainWindow;
class DMRSlider;
class ThumbnailPreview;
class SubtitlesView;

class ToolboxProxy: public QFrame {
    Q_OBJECT
public:
    ToolboxProxy(QWidget *mainWindow, PlayerEngine*);
    virtual ~ToolboxProxy();

    void updateTimeInfo(qint64 duration, qint64 pos);
    bool anyPopupShown() const;

signals:
    void requestPlay();
    void requestPause();
    void requestNextInList();
    void requesstPrevInList();

protected slots:
    void updatePosition(const QPoint& p);
    void buttonClicked(QString id);
    void updatePlayState();
    void updateFullState();
    void updateVolumeState();
    void updateMovieProgress();
    void updateButtonStates();
    void setProgress();
    void progressHoverChanged(int v);
    void updateHoverPreview(const QUrl& url, int secs);

protected:
    void paintEvent(QPaintEvent *pe) override;

private:
    void setup();

    MainWindow *_mainWindow {nullptr};
    PlayerEngine *_engine {nullptr};
    QLabel *_timeLabel {nullptr};

    DImageButton *_playBtn {nullptr};
    DImageButton *_prevBtn {nullptr};
    DImageButton *_nextBtn {nullptr};

    DImageButton *_subBtn {nullptr};
    VolumeButton *_volBtn {nullptr};
    DImageButton *_listBtn {nullptr};
    DImageButton *_fsBtn {nullptr};

    DMRSlider *_progBar {nullptr};
    ThumbnailPreview *_previewer {nullptr};
    SubtitlesView *_subView {nullptr};
    int _lastHoverValue {0};
    QTimer _previewTimer;
};
}


#endif /* ifndef _DMR_TOOLBOX_PROXY_H */
