#ifndef _DMR_PLAYLIST_WIDGET_H
#define _DMR_PLAYLIST_WIDGET_H 

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
class MainWindow;

class PlaylistWidget: public QListWidget {
    Q_OBJECT
public:
    PlaylistWidget(QWidget *, PlayerEngine*);
    virtual ~PlaylistWidget();

public slots:
    void togglePopup();
    void loadPlaylist();
    void openItemInFM();
    void showItemInfo();
    void removeClickedItem();

protected:
    void contextMenuEvent(QContextMenuEvent *cme);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

protected slots:
    void updateItemStates();

private:
    PlayerEngine *_engine {nullptr};
    MainWindow *_mw {nullptr};
    QWidget *_mouseItem {nullptr};
    QWidget *_clickedItem {nullptr};
    QSignalMapper *_closeMapper {nullptr};
    QSignalMapper *_activateMapper {nullptr};
};
}

#endif /* ifndef _DMR_PLAYLIST_WIDGET_H */
