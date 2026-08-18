#ifndef FONTCOMMAND_H
#define FONTCOMMAND_H

#include <QUndoCommand>
#include <QFont>

class TextShape;

class FontCommand : public QUndoCommand
{
public:
    FontCommand(TextShape *shape,
                const QFont &oldFont, const QFont &newFont,
                QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

    int id() const override { return 1006; }

private:
    TextShape *m_shape = nullptr;
    QFont m_oldFont, m_newFont;
};

#endif // FONTCOMMAND_H
