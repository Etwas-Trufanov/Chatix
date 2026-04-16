#ifndef CHATIXMAINWINDOW_H
#define CHATIXMAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class ChatixMainWindow;
}
QT_END_NAMESPACE

class ChatixMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChatixMainWindow(QWidget *parent = nullptr);
    ~ChatixMainWindow() override;

private:
    Ui::ChatixMainWindow *ui;
};
#endif // CHATIXMAINWINDOW_H
