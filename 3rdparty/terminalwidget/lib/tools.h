#ifndef TOOLS_H
#define TOOLS_H

#include <QWidget>
#include <QVBoxLayout>
#include <QString>
#include <QStringList>

QString get_kb_layout_dir();
void add_custom_color_scheme_dir(const QString& custom_dir);
const QStringList get_color_schemes_dirs();

//参考了Konsole中的TerminalHeaderBar
class TerminalHeaderBar : public QWidget {
    Q_OBJECT
public:
    explicit TerminalHeaderBar(QWidget *parent = nullptr);

private:
    QBoxLayout *m_boxLayout = nullptr;
};


#endif
