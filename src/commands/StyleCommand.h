#ifndef STYLECOMMAND_H
#define STYLECOMMAND_H

#include <QUndoCommand>
#include "style/ShapeStyle.h"

class ShapeBase;

class StyleCommand : public QUndoCommand
{
public:
    StyleCommand(ShapeBase *shape,
                 const ShapeStyle &oldStyle, const ShapeStyle &newStyle,
                 QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

    int id() const override { return 1002; }
    bool mergeWith(const QUndoCommand *other) override;

private:
    ShapeBase *m_shape = nullptr;
    ShapeStyle m_oldStyle;
    ShapeStyle m_newStyle;
};

#endif // STYLECOMMAND_H
