#include "nettexteditor.h"
#include <QKeyEvent>
#include <QClipboard>
#include <printf.h>
#include <QGuiApplication>

void NetTextEditor::keyReleaseEvent(QKeyEvent *e) {
    int key = e->key();
    pressedKeys.remove(key);
}

void NetTextEditor::keyPressEvent(QKeyEvent *e) {
    QChar key = e->key();
    pressedKeys.insert(e->key());

    if(key.isLetterOrNumber() || key.isSymbol()){
        QString delta;
        int position = this->textCursor().position();

        delta = "c " + QString::number(position) + " ";

        if(pressedKeys.contains(Qt::Key::Key_Control) && key == Qt::Key::Key_V){
            QClipboard *clipboard = QGuiApplication::clipboard();
            QString originalText = clipboard->text();
            delta += originalText;
        }
        else{
            if(!pressedKeys.contains(Qt::Key::Key_Shift) && key.isLetter()){
                key = key.toLower();
            }
            delta += key;
        }
        emit gotChange(delta);
    }
    else if(key == Qt::Key::Key_Delete){
        QString delta;
        int position = this->textCursor().position();

        delta = "c " + QString::number(position) + " \b";

        emit gotChange(delta);

    }
}
