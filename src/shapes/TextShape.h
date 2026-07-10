#ifndef TEXTSHAPE_H
#define TEXTSHAPE_H

#include "ShapeBase.h"
#include <QFont>
#include <QString>

class TextShape : public ShapeBase
{
public:
    enum { Type = QGraphicsItem::UserType + 6 };
    int type() const override { return Type; }

    explicit TextShape(QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    void setP2(const QPointF &p2) override;
    bool isFinished() const override { return m_finished; }
    void setFinished(bool f) { m_finished = f; }

    // 文字属性
    QString text() const { return m_text; }
    void setText(const QString &text);

    QFont font() const { return m_font; }
    void setFont(const QFont &font);

    // 序列化
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &obj) override;

private:
    QString m_text;
    QFont m_font;
    mutable qreal m_cachedWidth = -1;
    mutable qreal m_cachedHeight = -1;
};

#endif // TEXTSHAPE_H
