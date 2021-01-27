#ifndef NETTEXTEDITOR_H
#define NETTEXTEDITOR_H

#include <QTextEdit>
#include <QSet>

class NetTextEditor : public QTextEdit {
    Q_OBJECT
    using QTextEdit::QTextEdit;

public:
    void keyReleaseEvent(QKeyEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

private:
    QSet<int> pressedKeys;

signals:
    void gotChange(QString change);
};

#endif // NETTEXTEDITOR_H
