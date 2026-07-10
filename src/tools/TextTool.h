#ifndef TEXTTOOL_H
#define TEXTTOOL_H

#include "ToolBase.h"
#include <QFont>

class TextShape;

class TextTool : public ToolBase
{
    Q_OBJECT

public:
    explicit TextTool(QObject *parent = nullptr);

    void mousePressEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;

    QString name() const override { return tr("文字"); }
    QIcon icon() const override;
    QCursor cursor() const override { return Qt::IBeamCursor; }
    QString shortcut() const override { return "T"; }

    // 预设字体
    void setDefaultFont(const QFont &font) { m_defaultFont = font; }
    QFont defaultFont() const { return m_defaultFont; }

private:
    QFont m_defaultFont;
};

#endif // TEXTTOOL_H
