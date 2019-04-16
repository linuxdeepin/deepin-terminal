#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DMainWindow>
#include <QVBoxLayout>
#include <QStackedWidget>

DWIDGET_USE_NAMESPACE

class TabBar;
class TermWidgetPage;
class MainWindow : public DMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void addTab(bool activeTab = false);

protected slots:
    void onTermTitleChanged(QString title);

private:
    void setNewTermPage(TermWidgetPage *termPage);

    TabBar *m_tabbar;
    QWidget *m_centralWidget;
    QVBoxLayout *m_centralLayout;
    QStackedWidget *m_termStackWidget;
};

#endif // MAINWINDOW_H
