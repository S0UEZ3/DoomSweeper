#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#include "Constants.h"
#include "GameField.h"

#include <QVector>
#include <QtCore>
#include <cstdlib>
#include <ctime>

namespace SPR
{

	class MineSweeper
	{
	  public:
		MineSweeper();

		int width() const;
		int height() const;
		long long size() const;

		void reset(int width, int height, int mineNumber);
		void populate(int xToSkip, int yToSkip);

		int countFlagsAround(int x, int y) const;
		int getNeighbours(int x, int y) const;
		int getMine(int x, int y) const;
		int getFlag(int x, int y) const;
		bool getDiscovered(int x, int y) const;
		int totalMineNr() const;

		void discover(int x, int y);
		void disarm(int x, int y);
		bool checkWinCondition() const;

		GameField& field(int x, int y);
		const GameField& fieldConst(int x, int y) const;

		void markTemporary(int x, int y);
		void clearHighlights();

	  private:
		void populateMineCrew(int xToSkip, int yToSkip);
		void populateNeighbourhood();
		bool isValidIndex(int x, int y) const;

		int m_width;
		int m_height;
		int m_totalMineNr;
		int m_discoveredFieldsNr;
		QVector< GameField > m_data;
	};

}	 // namespace SPR

#endif	  // MINESWEEPER_H
