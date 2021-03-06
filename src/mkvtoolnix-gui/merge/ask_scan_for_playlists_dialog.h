#ifndef MTX_MKVTOOLNIX_GUI_MERGE_ASK_SCAN_FOR_PLAYLISTS_DIALOG_H
#define MTX_MKVTOOLNIX_GUI_MERGE_ASK_SCAN_FOR_PLAYLISTS_DIALOG_H

#include "common/common_pch.h"

#include "mkvtoolnix-gui/merge/source_file.h"

#include <QDialog>

namespace mtx { namespace gui { namespace Merge {

namespace Ui {
class AskScanForPlaylistsDialog;
}

class AskScanForPlaylistsDialog : public QDialog {
  Q_OBJECT;
protected:
  std::unique_ptr<Ui::AskScanForPlaylistsDialog> ui;

public:
  explicit AskScanForPlaylistsDialog(QWidget *parent = nullptr);
  ~AskScanForPlaylistsDialog();

  bool ask(SourceFile const &file, unsigned int otherFiles);
};

}}}

#endif // MTX_MKVTOOLNIX_GUI_MERGE_ASK_SCAN_FOR_PLAYLISTS_DIALOG_H
