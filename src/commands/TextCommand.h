#ifndef TEXTCOMMAND_H
#define TEXTCOMMAND_H

#include <QUndoCommand>
#include <QString>

class TextShape;

class TextCommand : public QUndoCommand
{
public:
    TextCommand(TextShape *shape,
                const QString &oldText, const QString &newText,
                QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

    int id() const override { return 1004; }

private:
    TextShape *m_shape = nullptr;
    QString m_oldText, m_newText;
};

#endif // TEXTCOMMAND_H
