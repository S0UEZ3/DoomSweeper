#ifndef CFIELDDELEGATE_H
#define CFIELDDELEGATE_H

#include "Constants.h"
#include "GameField.h"

#include <QAbstractItemDelegate>
#include <QApplication>
#include <QModelIndex>
#include <QStylePainter>

namespace SPR
{

	struct GameField;

	class ActiveDelegate : public QAbstractItemDelegate
	{
	  public:
		ActiveDelegate(QObject *parent = nullptr);

		virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
		virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

	  protected:
		virtual void getStyleForField(const GameField &field, QStyleOptionButton &buttonStyle) const;
		virtual void getGeneralsForField(const GameField &field, const QStyleOptionViewItem &option, QStyleOptionButton &buttonStyle) const;

		int m_fieldSize;
	};

}	 // namespace SPR

#endif	  // CFIELDDELEGATE_H
