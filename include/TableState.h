#ifndef TABLESTATE_H
#define TABLESTATE_H

#include "MineSweeper.h"

#include <QAbstractTableModel>
#include <QBrush>
#include <QPixmap>
#include <QSize>

namespace SPR
{
	class TableState : public QAbstractTableModel
	{
		Q_OBJECT

	  public:
		explicit TableState(QObject *parent = 0);
		~TableState() = default;

		QVariant data(const QModelIndex &index, int role) const override;
		int rowCount(const QModelIndex &parent = QModelIndex()) const override;
		int columnCount(const QModelIndex &parent = QModelIndex()) const override;
		void resetModel(int width, int height, int mine);

		MineSweeper &getMineSweeper();
		const MineSweeper &getMineSweeper() const;
		void setDebugMode(bool debug);

		bool isGameInProgress() const;
		bool hasLost() const;

	  signals:
		void gameStarted();
		void gameLost();
		void gameWon();
		void mineDisplay(int mineCount);

	  public slots:
		void onTableClicked(const QModelIndex &index);
		void onRightClicked(const QModelIndex &index);
		void onBothClicked(const QModelIndex &index);
		void onMiddleClicked(const QModelIndex &index);

	  private:
		void init(const QModelIndex &index);
		void discover(const QModelIndex &index);

		MineSweeper _model;
		int m_mineDisplay;
		bool m_initialized;
		bool _debugMode = false;
		QTimer *m_highlightClearTimer = nullptr;
	};

}	 // namespace SPR

#endif	  // TABLESTATE_H
