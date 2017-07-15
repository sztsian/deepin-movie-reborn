#ifndef _DMR_PLAYLIST_MODEL_H
#define _DMR_PLAYLIST_MODEL_H 

#include <QtWidgets>
#include <libffmpegthumbnailer/videothumbnailer.h>

namespace dmr {
using namespace ffmpegthumbnailer;
class PlayerEngine;

struct MovieInfo {
    QString title;
    QString fileType;
    QString resolution;
    QString filePath;
    QString creation;

    qint64 fileSize;
    qint64 duration;
    int width, height;

    static struct MovieInfo parseFromFile(const QFileInfo& fi);
    QString durationStr() const {
        auto secs = duration % 60;
        auto minutes = duration / 60;
        return QString("%1:%2").arg(minutes).arg(secs);
    }

    QString sizeStr() const {
        auto K = 1024;
        auto M = 1024 * K;
        auto G = 1024 * M;
        if (fileSize > G) {
            return QString(QT_TR_NOOP("%1G")).arg(fileSize / G);
        } else if (fileSize > M) {
            return QString(QT_TR_NOOP("%1M")).arg(fileSize / M);
        } else if (fileSize > K) {
            return QString(QT_TR_NOOP("%1K")).arg(fileSize / K);
        }
        return QString(QT_TR_NOOP("%1")).arg(fileSize);
    }
};


struct PlayItemInfo {
    QFileInfo info;
    QPixmap thumbnail;
    struct MovieInfo mi;
};


class PlaylistModel: public QObject {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int current READ current WRITE changeCurrent NOTIFY currentChanged)

public:
    friend class PlayerEngine;

    PlaylistModel(PlayerEngine* engine);
    void clear();
    void remove(int pos);
    void append(const QFileInfo&);

    void playNext();
    void playPrev();

    int count() const;
    const QList<PlayItemInfo>& items() const { return _infos; }
    int current() const;
    const PlayItemInfo& currentInfo() const;

    void switchPosition(int p1, int p2);

public slots:
    void changeCurrent(int);

signals:
    void countChanged();
    void currentChanged();
    void itemRemoved(int);

private:
    int _count {0};
    int _current {-1};
    QList<PlayItemInfo> _infos;
    VideoThumbnailer _thumbnailer;
    PlayerEngine *_engine {nullptr};

    struct PlayItemInfo calculatePlayInfo(const QFileInfo& fi);
};

}

#endif /* ifndef _DMR_PLAYLIST_MODEL_H */
