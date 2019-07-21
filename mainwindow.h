#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DMainWindow>
#include <QVBoxLayout>
#include <QStackedWidget>

#include <functional>

DWIDGET_USE_NAMESPACE

class TabBar;
class TermWidgetPage;
class TermProperties;
class ShortcutManager;
class MainWindowPluginInterface;
class MainWindow : public DMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(TermProperties properties, QWidget *parent = nullptr);
    ~MainWindow() override;

    void addTab(TermProperties properties, bool activeTab = false);
    void closeTab(const QString &identifier);
    void focusTab(const QString &identifier);
    TermWidgetPage *currentTab();

    void forAllTabPage(const std::function<void(TermWidgetPage *)> &func);
    void setTitleBarBackgroundColor(QString color);

protected:
    void closeEvent(QCloseEvent *event) override;

protected slots:
    void onTermTitleChanged(QString title);
    void onTabTitleChanged(QString title);

private:
    void initPlugins();
    void initWindow();
    void initShortcuts();
    void initConnections();
    void initTitleBar();
    void setNewTermPage(TermWidgetPage *termPage, bool activePage = true);
    void showSettingDialog();

    QMenu *m_menu;
    TabBar *m_tabbar;
    QWidget *m_centralWidget;
    QVBoxLayout *m_centralLayout;
    QStackedWidget *m_termStackWidget;
    QString m_titlebarStyleSheet;
    ShortcutManager *m_shortcutManager;
    QList<MainWindowPluginInterface*> m_plugins;
};

#endif // MAINWINDOW_H
