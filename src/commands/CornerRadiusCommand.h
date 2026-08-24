#ifndef CORNERRADIUSCOMMAND_H
#define CORNERRADIUSCOMMAND_H

#include <QUndoCommand>

class RectShape;

// 矩形四角独立圆角调节撤销
class CornerRadiusCommand : public QUndoCommand
{
public:
    CornerRadiusCommand(RectShape *shape,
                        qreal oldTL, qreal oldTR, qreal oldBR, qreal oldBL,
                        qreal newTL, qreal newTR, qreal newBR, qreal newBL,
                        const QString &text = QString(),
                        QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

    int id() const override { return 1012; }

private:
    RectShape *m_shape = nullptr;
    qreal m_oldTL = 0, m_oldTR = 0, m_oldBR = 0, m_oldBL = 0;
    qreal m_newTL = 0, m_newTR = 0, m_newBR = 0, m_newBL = 0;
};

#endif // CORNERRADIUSCOMMAND_H
