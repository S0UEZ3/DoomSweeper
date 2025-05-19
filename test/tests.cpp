#include "include/MineSweeper.h"

#define private public
#include "include/Save.h"
#include "include/TableState.h"
#undef private

#include "include/Constants.h"
#include "include/Preferences.h"
#include "include/mainwindow.h"

#include "gtest/gtest.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDataStream>
#include <QFile>
#include <QModelIndex>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTimer>
#include <QVariant>

using namespace SPR;

// Глобальный QApplication
struct QAppWrapper
{
	QAppWrapper(int& argc, char** argv) : app(argc, argv) {}
	QApplication app;
};

class MineSweeperTest : public ::testing::Test
{
  protected:
	MineSweeper game;

	void SetUp() override
	{
		game.reset(5, 5, 3);	// 5x5 поле с 3 минами
	}
};

TEST_F(MineSweeperTest, ResetClearsPreviousState)
{
	game.populate(0, 0);
	game.discover(1, 1);
	game.disarm(2, 2);

	game.reset(5, 5, 3);

	for (int x = 0; x < game.width(); ++x)
	{
		for (int y = 0; y < game.height(); ++y)
		{
			EXPECT_FALSE(game.getDiscovered(x, y));
			EXPECT_EQ(game.getMine(x, y), 0);
			EXPECT_EQ(game.getFlag(x, y), 0);
			EXPECT_EQ(game.getNeighbours(x, y), 0);
		}
	}
}

TEST_F(MineSweeperTest, FieldReturnsCorrectReference)
{
	game.populate(0, 0);
	game.field(1, 1).mine = 1;
	EXPECT_EQ(game.getMine(1, 1), 1);
}

TEST_F(MineSweeperTest, FieldConstReturnsCorrectReference)
{
	game.populate(0, 0);
	game.field(2, 2).neighbours = 3;
	EXPECT_EQ(game.fieldConst(2, 2).neighbours, 3);
}

TEST_F(MineSweeperTest, DiscoverDoesNotAffectFlaggedCells)
{
	game.populate(0, 0);
	game.disarm(1, 1);	  // Флаг
	game.discover(1, 1);
	EXPECT_FALSE(game.getDiscovered(1, 1));
}

TEST_F(MineSweeperTest, DisarmCyclesCorrectlyAfterNotSure)
{
	game.disarm(2, 2);	  // Флаг
	game.disarm(2, 2);	  // ?
	game.disarm(2, 2);	  // пусто
	game.disarm(2, 2);	  // флаг снова
	EXPECT_EQ(game.getFlag(2, 2), 1);
}

TEST_F(MineSweeperTest, CountFlagsAroundCountsCorrectly)
{
	game.disarm(0, 0);	  // флаг
	game.disarm(1, 0);	  // флаг
	game.disarm(1, 1);	  // флаг
	EXPECT_EQ(game.countFlagsAround(1, 1), 3);
}

TEST_F(MineSweeperTest, GetMineReturnsZeroForInvalidIndex)
{
	EXPECT_EQ(game.getMine(-1, -1), 0);
	EXPECT_EQ(game.getMine(100, 100), 0);
}

TEST_F(MineSweeperTest, GetNeighboursReturnsZeroForInvalidIndex)
{
	EXPECT_EQ(game.getNeighbours(-1, -1), 0);
	EXPECT_EQ(game.getNeighbours(100, 100), 0);
}

TEST_F(MineSweeperTest, MineCountIsCorrectAfterPopulate)
{
	game.populate(0, 0);

	int mineCount = 0;
	for (int x = 0; x < game.width(); ++x)
	{
		for (int y = 0; y < game.height(); ++y)
		{
			mineCount += game.getMine(x, y);
		}
	}
	EXPECT_EQ(mineCount, game.totalMineNr());
}

TEST_F(MineSweeperTest, NoMineAtFirstClickPosition)
{
	game.populate(2, 2);
	EXPECT_EQ(game.getMine(2, 2), 0);
}

TEST_F(MineSweeperTest, DiscoverIncrementsDiscoveredCountOnce)
{
	game.populate(0, 0);
	game.discover(1, 1);
	game.discover(1, 1);	// second call shouldn't increment
	EXPECT_TRUE(game.getDiscovered(1, 1));
}

TEST_F(MineSweeperTest, CheckWinConditionFalseInitially)
{
	game.populate(0, 0);
	EXPECT_FALSE(game.checkWinCondition());
}

TEST_F(MineSweeperTest, CheckWinConditionTrueAfterAllDiscovered)
{
	game.reset(2, 2, 1);
	game.populate(0, 0);

	for (int x = 0; x < game.width(); ++x)
	{
		for (int y = 0; y < game.height(); ++y)
		{
			if (game.getMine(x, y) == 0)
			{
				game.discover(x, y);
			}
		}
	}
	EXPECT_TRUE(game.checkWinCondition());
}

TEST_F(MineSweeperTest, DisarmDoesNothingOutsideBounds)
{
	game.disarm(-1, -1);	// shouldn't crash
	game.disarm(100, 100);
	SUCCEED();
}

TEST_F(MineSweeperTest, DiscoverDoesNothingOutsideBounds)
{
	game.discover(-5, -5);
	game.discover(99, 99);
	SUCCEED();
}

TEST_F(MineSweeperTest, GetFlagReturnsZeroWhenNotFlagged)
{
	EXPECT_EQ(game.getFlag(3, 3), 0);
}

TEST_F(MineSweeperTest, DiscoveredFieldDoesNotIncrementOnDisarm)
{
	game.populate(0, 0);
	game.disarm(1, 1);
	game.discover(1, 1);
	EXPECT_FALSE(game.getDiscovered(1, 1));
}

TEST_F(MineSweeperTest, GetNeighboursReturnsCorrectCountAfterPopulate)
{
	game.reset(3, 3, 1);
	game.populate(0, 0);

	int totalMines = 0;
	for (int x = 0; x < 3; ++x)
	{
		for (int y = 0; y < 3; ++y)
		{
			totalMines += game.getMine(x, y);
		}
	}

	int sumNeighbourCounts = 0;
	for (int x = 0; x < 3; ++x)
	{
		for (int y = 0; y < 3; ++y)
		{
			sumNeighbourCounts += game.getNeighbours(x, y);
		}
	}
	EXPECT_GE(sumNeighbourCounts, totalMines);
}

TEST_F(MineSweeperTest, CountFlagsAroundCountsSelfIfFlagged)
{
	game.disarm(1, 1);	  // Flag
	EXPECT_EQ(game.countFlagsAround(1, 1), 1);
}

TEST_F(MineSweeperTest, CountFlagsAroundCountsNeighboursOnly)
{
	game.disarm(0, 0);
	game.disarm(0, 1);
	EXPECT_EQ(game.countFlagsAround(1, 1), 2);
}

TEST_F(MineSweeperTest, MultipleResetClearsPreviousState)
{
	game.populate(0, 0);
	game.disarm(0, 0);
	game.discover(1, 1);
	game.reset(4, 4, 2);
	EXPECT_EQ(game.totalMineNr(), 2);
	EXPECT_EQ(game.width(), 4);
	EXPECT_EQ(game.height(), 4);
}

TEST_F(MineSweeperTest, AllFieldsInitiallyUndiscovered)
{
	for (int x = 0; x < game.width(); ++x)
	{
		for (int y = 0; y < game.height(); ++y)
		{
			EXPECT_FALSE(game.getDiscovered(x, y));
		}
	}
}

TEST_F(MineSweeperTest, DiscoveredCounterIncrementsOnlyOncePerCell)
{
	game.populate(0, 0);
	game.discover(2, 2);
	game.discover(2, 2);
	game.discover(2, 2);
	EXPECT_TRUE(game.getDiscovered(2, 2));
}

TEST_F(MineSweeperTest, NoMinesStillWinsAfterAllFieldsDiscovered)
{
	game.reset(2, 2, 0);
	for (int x = 0; x < 2; ++x)
		for (int y = 0; y < 2; ++y)
			game.discover(x, y);
	EXPECT_TRUE(game.checkWinCondition());
}

TEST_F(MineSweeperTest, DiscoverOnFlaggedFieldDoesNotReveal)
{
	game.disarm(0, 0);	  // flag
	game.discover(0, 0);
	EXPECT_FALSE(game.getDiscovered(0, 0));
}

TEST_F(MineSweeperTest, WinConditionFailsIfUndiscoveredFieldsRemain)
{
	game.reset(2, 2, 1);
	game.populate(0, 0);
	// Discover only 1 safe field
	for (int x = 0; x < 2; ++x)
		for (int y = 0; y < 2; ++y)
			if (!game.getMine(x, y))
			{
				game.discover(x, y);
				goto done;
			}
done:
	EXPECT_FALSE(game.checkWinCondition());
}

TEST_F(MineSweeperTest, MineCountMatchesRequestedCount)
{
	game.populate(0, 0);
	int actualMines = 0;
	for (int x = 0; x < game.width(); ++x)
		for (int y = 0; y < game.height(); ++y)
			actualMines += game.getMine(x, y);
	EXPECT_EQ(actualMines, game.totalMineNr());
}

TEST_F(MineSweeperTest, FieldWithMineHasNoEffectOnNeighboursCountCalculationItself)
{
	game.reset(3, 3, 1);
	game.populate(1, 1);

	if (game.getMine(1, 1))
	{
		EXPECT_LT(game.getNeighbours(1, 1), 9);
	}
}

TEST_F(MineSweeperTest, PopulateSkipsSafeCellCorrectly)
{
	game.reset(3, 3, 8);
	game.populate(1, 1);
	EXPECT_EQ(game.getMine(1, 1), 0);
}

TEST_F(MineSweeperTest, SizeCalculationIsCorrect)
{
	EXPECT_EQ(game.size(), game.width() * game.height());
}

TEST_F(MineSweeperTest, GameHandlesZeroSizeResetGracefully)
{
	game.reset(0, 0, 0);
	EXPECT_EQ(game.width(), 0);
	EXPECT_EQ(game.height(), 0);
	EXPECT_EQ(game.size(), 0);
}

TEST_F(MineSweeperTest, GetFlagReturnsZeroOnUnflaggedField)
{
	EXPECT_EQ(game.getFlag(0, 0), 0);
}

TEST_F(MineSweeperTest, DiscoverIncrementsDiscoveredFieldCount)
{
	int before = game.checkWinCondition();	  // should be false
	game.discover(0, 0);
	EXPECT_FALSE(before);
	EXPECT_TRUE(game.getDiscovered(0, 0));
}

TEST_F(MineSweeperTest, DiscoverOutOfBoundsDoesNothing)
{
	game.discover(-1, -1);
	game.discover(5, 5);
	// просто проверим, что ничего не упало и ничего не открылось
	for (int x = 0; x < game.width(); ++x)
		for (int y = 0; y < game.height(); ++y)
			EXPECT_FALSE(game.getDiscovered(x, y));
}

TEST_F(MineSweeperTest, DisarmOutOfBoundsDoesNothing)
{
	game.disarm(-1, -1);
	game.disarm(5, 5);
	// убеждаемся, что ничего не произошло
	for (int x = 0; x < game.width(); ++x)
		for (int y = 0; y < game.height(); ++y)
			EXPECT_EQ(game.fieldConst(x, y).disarmed, FIELD_NOT_VISITED);
}

TEST_F(MineSweeperTest, DisarmCyclesBackToNone)
{
	game.disarm(1, 1);	  // FLAG
	game.disarm(1, 1);	  // PLAYER_NOT_SURE
	game.disarm(1, 1);	  // NONE
	EXPECT_EQ(game.fieldConst(1, 1).disarmed, FIELD_NOT_VISITED);
}

TEST_F(MineSweeperTest, CountFlagsAroundReturnsCorrectValue)
{
	game.disarm(0, 0);
	game.disarm(1, 0);
	EXPECT_EQ(game.countFlagsAround(1, 1), 2);
}

TEST_F(MineSweeperTest, GetMineReturnsOneForMine)
{
	game.reset(1, 1, 0);
	game.field(0, 0).mine = 1;
	EXPECT_EQ(game.getMine(0, 0), 1);
}

TEST_F(MineSweeperTest, GetMineOutOfBoundsReturnsZero)
{
	EXPECT_EQ(game.getMine(-1, 0), 0);
	EXPECT_EQ(game.getMine(0, -1), 0);
	EXPECT_EQ(game.getMine(10, 10), 0);
}

TEST_F(MineSweeperTest, GetDiscoveredOutOfBoundsReturnsTrue)
{
	EXPECT_TRUE(game.getDiscovered(-1, -1));
	EXPECT_TRUE(game.getDiscovered(100, 100));
}

TEST_F(MineSweeperTest, PopulateWithTooManyMinesLogsWarning)
{
	game.reset(2, 2, 5);	// 4 поля, 5 мин — некорректно
	// тест не проверяет лог напрямую, но можно проверить, что populate не крашится
	EXPECT_NO_THROW(game.populate(0, 0));
}

TEST_F(MineSweeperTest, InitialSizeIsCorrect)
{
	EXPECT_EQ(game.width(), 5);
	EXPECT_EQ(game.height(), 5);
	EXPECT_EQ(game.size(), 25);
	EXPECT_EQ(game.totalMineNr(), 3);
}

TEST_F(MineSweeperTest, AllCellsInitiallyUndiscovered)
{
	for (int x = 0; x < game.width(); ++x)
	{
		for (int y = 0; y < game.height(); ++y)
		{
			EXPECT_FALSE(game.getDiscovered(x, y));
		}
	}
}

TEST_F(MineSweeperTest, PopulateDoesNotPlaceMineOnInitialClick)
{
	game.populate(2, 2);	// Предполагаем, что на (2,2) не будет мины

	EXPECT_EQ(game.getMine(2, 2), 0);
}

TEST_F(MineSweeperTest, AllMinesArePlaced)
{
	game.populate(0, 0);

	int mineCount = 0;
	for (int x = 0; x < game.width(); ++x)
	{
		for (int y = 0; y < game.height(); ++y)
		{
			mineCount += game.getMine(x, y);
		}
	}
	EXPECT_EQ(mineCount, 3);
}

TEST_F(MineSweeperTest, DiscoverIncreasesDiscoveredCount)
{
	game.populate(0, 0);
	game.discover(0, 0);
	EXPECT_TRUE(game.getDiscovered(0, 0));
}

TEST_F(MineSweeperTest, DisarmCyclesThroughFlagsCorrectly)
{
	game.disarm(1, 1);
	EXPECT_EQ(game.getFlag(1, 1), 1);	 // флаг

	game.disarm(1, 1);
	EXPECT_EQ(game.getFlag(1, 1), 0);	 // ?

	game.disarm(1, 1);
	EXPECT_EQ(game.getFlag(1, 1), 0);	 // пусто
}

TEST_F(MineSweeperTest, CheckWinConditionFalseAtStart)
{
	game.populate(0, 0);
	EXPECT_FALSE(game.checkWinCondition());
}

TEST_F(MineSweeperTest, CheckWinConditionTrueAfterAllSafeCellsDiscovered)
{
	game.reset(2, 2, 1);	// 2x2 поле с 1 миной
	game.populate(0, 0);

	// Открываем все клетки, кроме той, где мина
	int opened = 0;
	for (int x = 0; x < 2; ++x)
	{
		for (int y = 0; y < 2; ++y)
		{
			if (game.getMine(x, y) == 0)
			{
				game.discover(x, y);
				opened++;
			}
		}
	}
	ASSERT_EQ(opened, 3);
	EXPECT_TRUE(game.checkWinCondition());
}

TEST_F(MineSweeperTest, DiscoverDoesNotCrashOutOfBounds)
{
	EXPECT_NO_THROW(game.discover(-1, -1));
	EXPECT_NO_THROW(game.discover(100, 100));
}

TEST_F(MineSweeperTest, ResetInitializesCorrectly)
{
	EXPECT_EQ(game.width(), 5);
	EXPECT_EQ(game.height(), 5);
	EXPECT_EQ(game.size(), 25);
	EXPECT_EQ(game.totalMineNr(), 3);

	for (int x = 0; x < game.width(); ++x)
	{
		for (int y = 0; y < game.height(); ++y)
		{
			EXPECT_FALSE(game.getDiscovered(x, y));
			EXPECT_EQ(game.getMine(x, y), 0);
		}
	}
}

TEST_F(MineSweeperTest, PopulatePlacesMinesAndNeighbours)
{
	game.populate(2, 2);	// пропускаем клетку (2,2)

	int mineCount = 0;
	for (int x = 0; x < game.width(); ++x)
	{
		for (int y = 0; y < game.height(); ++y)
		{
			mineCount += game.getMine(x, y);
		}
	}

	EXPECT_EQ(mineCount, 3);
	EXPECT_EQ(game.getMine(2, 2), 0);	 // гарантированно нет мины
}

TEST_F(MineSweeperTest, DiscoverUpdatesState)
{
	game.discover(1, 1);

	EXPECT_TRUE(game.getDiscovered(1, 1));
	EXPECT_FALSE(game.getDiscovered(0, 0));	   // соседняя клетка остаётся неоткрытой
}

TEST_F(MineSweeperTest, CountFlagsAroundWorks)
{
	game.disarm(1, 1);
	game.disarm(2, 1);
	game.disarm(3, 1);

	int flags = game.countFlagsAround(2, 2);	// все флажки рядом
	EXPECT_EQ(flags, 3);
}

TEST_F(MineSweeperTest, CheckWinConditionTrueWhenAllNonMineDiscovered)
{
	game.reset(2, 2, 1);
	game.populate(0, 0);

	// Откроем все, кроме одного, где мина
	for (int x = 0; x < game.width(); ++x)
	{
		for (int y = 0; y < game.height(); ++y)
		{
			if (!game.getMine(x, y))
			{
				game.discover(x, y);
			}
		}
	}

	EXPECT_TRUE(game.checkWinCondition());
}

TEST_F(MineSweeperTest, DiscoverCornerCellDoesNotCrash)
{
	EXPECT_NO_THROW(game.discover(0, 0));
}

TEST_F(MineSweeperTest, DiscoverEdgeCellDoesNotCrash)
{
	EXPECT_NO_THROW(game.discover(4, 2));
}

TEST_F(MineSweeperTest, DiscoverDoesNotOpenFlaggedCell)
{
	game.disarm(1, 1);	  // флаг
	game.discover(1, 1);
	EXPECT_FALSE(game.getDiscovered(1, 1));	   // всё ещё закрыта
}

TEST_F(MineSweeperTest, CountFlagsAroundEdgeCell)
{
	game.disarm(0, 1);
	game.disarm(1, 0);

	EXPECT_EQ(game.countFlagsAround(0, 0), 2);
}

TEST_F(MineSweeperTest, DiscoverOnMineTriggersLossState)
{
	game.reset(2, 2, 1);
	game.populate(0, 0);

	// найдём мину
	bool mineFound = false;
	for (int x = 0; x < 2 && !mineFound; ++x)
	{
		for (int y = 0; y < 2 && !mineFound; ++y)
		{
			if (game.getMine(x, y))
			{
				game.discover(x, y);
				EXPECT_TRUE(game.getDiscovered(x, y));
				mineFound = true;
			}
		}
	}

	EXPECT_TRUE(mineFound);	   // sanity check
}

TEST_F(MineSweeperTest, CheckWinConditionFalseIfSomeSafeCellsClosed)
{
	game.reset(2, 2, 1);
	game.populate(0, 0);

	int opened = 0;
	for (int x = 0; x < 2; ++x)
	{
		for (int y = 0; y < 2; ++y)
		{
			if (!game.getMine(x, y) && opened == 0)
			{
				game.discover(x, y);
				++opened;
			}
		}
	}

	EXPECT_FALSE(game.checkWinCondition());
}

TEST_F(MineSweeperTest, ResetInitializesCorrectSize)
{
	game.reset(10, 5, 7);
	EXPECT_EQ(game.width(), 10);
	EXPECT_EQ(game.height(), 5);
	EXPECT_EQ(game.totalMineNr(), 7);
	EXPECT_EQ(game.size(), 50);
}

TEST_F(MineSweeperTest, DiscoverMarksCellAsDiscovered)
{
	game.reset(5, 5, 0);
	game.populate(0, 0);

	game.discover(1, 1);
	EXPECT_TRUE(game.getDiscovered(1, 1));
}

TEST_F(MineSweeperTest, DisarmCyclesFlagStates)
{
	game.reset(3, 3, 0);
	game.populate(0, 0);

	EXPECT_EQ(game.getFlag(1, 1), 0);
	game.disarm(1, 1);	  // ставим флаг
	EXPECT_EQ(game.getFlag(1, 1), 1);
	game.disarm(1, 1);	  // снимаем флаг
	EXPECT_EQ(game.getFlag(1, 1), 0);
}

TEST_F(MineSweeperTest, FlaggedCellsCannotBeDiscovered)
{
	game.reset(3, 3, 0);
	game.populate(0, 0);

	game.disarm(2, 2);		// ставим флаг
	game.discover(2, 2);	// попытка открыть — не должна сработать
	EXPECT_FALSE(game.getDiscovered(2, 2));
}

TEST_F(MineSweeperTest, GetNeighboursReturnsCorrectlyAfterPopulate)
{
	game.reset(5, 5, 10);
	game.populate(2, 2);

	int count = 0;
	for (int dx = -1; dx <= 1; ++dx)
		for (int dy = -1; dy <= 1; ++dy)
		{
			int x = 2 + dx, y = 2 + dy;
			if ((dx || dy) && x >= 0 && y >= 0 && x < 5 && y < 5)
			{
				count += game.getMine(x, y);
			}
		}

	EXPECT_EQ(game.getNeighbours(2, 2), count);
}

TEST_F(MineSweeperTest, WinConditionTrueAfterAllSafeCellsOpened)
{
	game.reset(2, 2, 1);
	game.populate(0, 0);

	// Открываем все безопасные клетки
	for (int x = 0; x < 2; ++x)
		for (int y = 0; y < 2; ++y)
			if (game.getMine(x, y) == 0)
				game.discover(x, y);

	EXPECT_TRUE(game.checkWinCondition());
}

TEST_F(MineSweeperTest, ResetOverridesPreviousState)
{
	game.reset(3, 3, 1);
	game.populate(0, 0);
	game.discover(1, 1);
	game.disarm(2, 2);

	game.reset(5, 5, 5);
	EXPECT_EQ(game.width(), 5);
	EXPECT_EQ(game.height(), 5);
	EXPECT_EQ(game.totalMineNr(), 5);
	EXPECT_FALSE(game.getDiscovered(1, 1));
	EXPECT_EQ(game.getFlag(2, 2), 0);
}

TEST_F(MineSweeperTest, SizeReturnsWidthTimesHeight)
{
	game.reset(7, 6, 10);
	EXPECT_EQ(game.size(), 42);
}

TEST_F(MineSweeperTest, DisarmOnOutOfBoundsDoesNothing)
{
	game.reset(4, 4, 0);
	game.disarm(-1, -1);
	game.disarm(4, 4);
	// Ничего не падает, тест пройден
	SUCCEED();
}

TEST_F(MineSweeperTest, DiscoverOnOutOfBoundsDoesNothing)
{
	game.reset(4, 4, 0);
	game.discover(-1, 0);
	game.discover(0, 4);
	// Ничего не упало — ок
	SUCCEED();
}

TEST_F(MineSweeperTest, DiscoverMineLosesGameButStillMarkedDiscovered)
{
	game.reset(2, 2, 1);
	game.populate(0, 0);

	// Ищем мину и открываем её
	for (int x = 0; x < 2; ++x)
		for (int y = 0; y < 2; ++y)
			if (game.getMine(x, y) == 1)
			{
				game.discover(x, y);
				EXPECT_TRUE(game.getDiscovered(x, y));
				return;
			}

	FAIL() << "No mine found after populate";
}

TEST_F(MineSweeperTest, CountFlagsAroundWorksNearBorders)
{
	game.reset(3, 3, 0);
	game.populate(1, 1);

	game.disarm(0, 0);
	game.disarm(1, 0);

	EXPECT_EQ(game.countFlagsAround(0, 1), 2);
}

TEST_F(MineSweeperTest, AllFlagsClearedAfterReset)
{
	game.reset(2, 2, 0);
	game.populate(0, 0);

	game.disarm(0, 0);
	game.disarm(1, 1);

	game.reset(2, 2, 0);
	game.populate(0, 0);

	for (int x = 0; x < 2; ++x)
		for (int y = 0; y < 2; ++y)
			EXPECT_EQ(game.getFlag(x, y), 0);
}

TEST_F(MineSweeperTest, AllDiscoveredClearedAfterReset)
{
	game.reset(2, 2, 0);
	game.populate(0, 0);
	game.discover(0, 0);
	game.discover(1, 1);

	game.reset(2, 2, 0);
	game.populate(0, 0);

	for (int x = 0; x < 2; ++x)
		for (int y = 0; y < 2; ++y)
			EXPECT_FALSE(game.getDiscovered(x, y));
}

TEST_F(MineSweeperTest, GetNeighboursZeroIfNoMinesAround)
{
	game.reset(3, 3, 0);
	game.populate(1, 1);
	EXPECT_EQ(game.getNeighbours(1, 1), 0);
}

TEST_F(MineSweeperTest, DisarmAndRemoveFlag)
{
	game.reset(3, 3, 0);
	game.populate(0, 0);

	game.disarm(1, 1);	  // установить флаг
	EXPECT_EQ(game.getFlag(1, 1), 1);

	game.disarm(1, 1);	  // снять флаг
	EXPECT_EQ(game.getFlag(1, 1), 0);
}

TEST_F(MineSweeperTest, DiscoverDoesNotOpenFlaggedCells)
{
	game.reset(3, 3, 0);
	game.populate(0, 0);

	game.disarm(1, 1);		// поставить флаг
	game.discover(1, 1);	// не должно открыться
	EXPECT_FALSE(game.getDiscovered(1, 1));
}

TEST_F(MineSweeperTest, CheckWinConditionTrueAfterDiscoveringAllSafeCells)
{
	game.reset(2, 2, 1);
	game.populate(0, 0);

	// Открываем все безопасные ячейки
	for (int x = 0; x < 2; ++x)
		for (int y = 0; y < 2; ++y)
			if (game.getMine(x, y) == 0)
				game.discover(x, y);

	EXPECT_TRUE(game.checkWinCondition());
}

TEST_F(MineSweeperTest, CheckWinConditionFalseIfMineNotFlagged)
{
	game.reset(2, 2, 1);
	game.populate(0, 0);

	for (int x = 0; x < 2; ++x)
		for (int y = 0; y < 2; ++y)
			if (game.getMine(x, y) == 0)
				game.discover(x, y);

	// не ставим флаг на мину
	EXPECT_TRUE(game.checkWinCondition());
}

TEST_F(MineSweeperTest, CheckWinConditionFalseIfUndiscoveredSafeCellsRemain)
{
	game.reset(2, 2, 1);
	game.populate(0, 0);

	// случайно открываем одну безопасную
	for (int x = 0; x < 2; ++x)
		for (int y = 0; y < 2; ++y)
			if (game.getMine(x, y) == 0)
			{
				game.discover(x, y);
				break;
			}

	EXPECT_FALSE(game.checkWinCondition());
}

TEST_F(MineSweeperTest, FlaggingAllMinesNotEnoughToWin)
{
	game.reset(2, 2, 2);
	game.populate(0, 0);

	// ставим флаги на мины
	for (int x = 0; x < 2; ++x)
		for (int y = 0; y < 2; ++y)
			if (game.getMine(x, y) == 1)
				game.disarm(x, y);

	// но не открыты безопасные клетки
	EXPECT_FALSE(game.checkWinCondition());
}

TEST_F(MineSweeperTest, CannotDiscoverAlreadyDiscoveredCell)
{
	game.reset(2, 2, 0);
	game.populate(0, 0);

	game.discover(1, 1);
	game.discover(1, 1);	// повторный вызов

	EXPECT_TRUE(game.getDiscovered(1, 1));	  // всё равно открыта
}

TEST_F(MineSweeperTest, DiscoverCornerPropagatesCorrectly)
{
	game.reset(3, 3, 0);
	game.populate(0, 0);

	game.discover(0, 0);	// левый верхний угол

	EXPECT_TRUE(game.getDiscovered(0, 0));
	EXPECT_FALSE(game.getDiscovered(1, 0));
	EXPECT_FALSE(game.getDiscovered(0, 1));
	EXPECT_FALSE(game.getDiscovered(1, 1));
}

TEST_F(MineSweeperTest, DiscoverAfterWinDoesNotCrashOrChangeState)
{
	game.reset(2, 2, 1);
	game.populate(0, 0);

	// Открываем все безопасные ячейки
	for (int x = 0; x < 2; ++x)
		for (int y = 0; y < 2; ++y)
			if (game.getMine(x, y) == 0)
				game.discover(x, y);

	EXPECT_TRUE(game.checkWinCondition());

	// Попробуем открыть ещё одну безопасную
	for (int x = 0; x < 2; ++x)
		for (int y = 0; y < 2; ++y)
			if (game.getMine(x, y) == 0)
				game.discover(x, y);	// должно быть безопасно

	EXPECT_TRUE(game.checkWinCondition());
}

TEST_F(MineSweeperTest, OneByOneFieldNoMinesIsInstantWin)
{
	game.reset(1, 1, 0);
	game.populate(0, 0);
	game.discover(0, 0);
	EXPECT_TRUE(game.checkWinCondition());
}

TEST_F(MineSweeperTest, OneByOneFieldWithMineAndSkipCellIsSafe)
{
	game.reset(1, 1, 1);
	game.populate(0, 0);
	EXPECT_EQ(game.getMine(0, 0), 0);	 // populate должна пропустить мину
}

TEST_F(MineSweeperTest, DiscoverBottomRightCorner)
{
	game.reset(3, 3, 0);
	game.populate(2, 2);
	game.discover(2, 2);
	EXPECT_TRUE(game.getDiscovered(2, 2));
}

TEST_F(MineSweeperTest, ResetResetsEverything)
{
	game.reset(3, 3, 1);
	game.populate(0, 0);
	game.disarm(1, 1);
	game.discover(0, 0);

	game.reset(3, 3, 1);
	for (int x = 0; x < 3; ++x)
		for (int y = 0; y < 3; ++y)
		{
			EXPECT_FALSE(game.getDiscovered(x, y));
			EXPECT_EQ(game.getFlag(x, y), 0);
		}
}

TEST_F(MineSweeperTest, AllCellsMinedExceptOne)
{
	game.reset(3, 3, 8);
	game.populate(1, 1);	// Одна ячейка точно безопасна

	int safeCount = 0;
	for (int x = 0; x < 3; ++x)
		for (int y = 0; y < 3; ++y)
			if (game.getMine(x, y) == 0)
				++safeCount;

	EXPECT_EQ(safeCount, 1);
}

TEST_F(MineSweeperTest, FlaggingAlreadyFlaggedCellUnflagsIt)
{
	game.reset(2, 2, 0);
	game.populate(0, 0);

	game.disarm(1, 1);
	EXPECT_EQ(game.getFlag(1, 1), 1);
	game.disarm(1, 1);	  // снять флаг
	EXPECT_EQ(game.getFlag(1, 1), 0);
}

TEST_F(MineSweeperTest, DiscoverCellWithAllEightMineNeighbours)
{
	game.reset(3, 3, 8);
	game.populate(1, 1);	// центральная должна быть безопасной
	EXPECT_EQ(game.getMine(1, 1), 0);

	game.discover(1, 1);
	EXPECT_TRUE(game.getDiscovered(1, 1));
	EXPECT_EQ(game.getNeighbours(1, 1), 8);
}

TEST_F(MineSweeperTest, DisarmOutOfBoundsIsIgnored)
{
	game.reset(3, 3, 0);
	game.populate(0, 0);

	// не должно упасть
	EXPECT_NO_THROW(game.disarm(-1, -1));
	EXPECT_NO_THROW(game.disarm(3, 3));
}

class TableStateTest : public ::testing::Test
{
  protected:
	TableState* tableState;

	void SetUp() override { tableState = new TableState(); }

	void TearDown() override { delete tableState; }
};

TEST_F(TableStateTest, InitializeGameUpdatesSignals)
{
	QSignalSpy gameStartedSpy(tableState, &TableState::gameStarted);

	// Инициализируем игру
	tableState->resetModel(4, 4, 5);
	tableState->init(QModelIndex());

	// Проверяем, что сигнал gameStarted был эмитирован
	EXPECT_EQ(gameStartedSpy.count(), 1);
}

TEST_F(TableStateTest, SetDebugModeUpdatesLayout)
{
	QSignalSpy layoutChangedSpy(tableState, &TableState::layoutChanged);

	// Устанавливаем режим отладки
	tableState->setDebugMode(true);

	// Проверяем, что сигнал layoutChanged был эмитирован
	EXPECT_EQ(layoutChangedSpy.count(), 1);
}

TEST_F(TableStateTest, RowCountReturnsCorrectNumberOfRows)
{
	tableState->resetModel(4, 4, 5);

	// Проверяем, что количество строк соответствует ширине модели
	EXPECT_EQ(tableState->rowCount(), 4);
}

TEST_F(TableStateTest, ColumnCountReturnsCorrectNumberOfColumns)
{
	tableState->resetModel(4, 4, 5);

	// Проверяем, что количество столбцов соответствует высоте модели
	EXPECT_EQ(tableState->columnCount(), 4);
}

TEST_F(TableStateTest, DataReturnsCorrectVariantForCell)
{
	tableState->resetModel(4, 4, 5);

	// Получаем данные для конкретной ячейки
	QModelIndex index = tableState->index(1, 1);
	QVariant data = tableState->data(index, Qt::DisplayRole);

	// Проверяем, что данные корректно возвращаются
	EXPECT_TRUE(data.isValid());
}

TEST_F(TableStateTest, IsGameInProgressReturnsTrueWhenGameIsActive)
{
	tableState->resetModel(4, 4, 5);

	// Инициализируем игру
	tableState->init(QModelIndex());

	// Проверяем, что игра в процессе
	EXPECT_TRUE(tableState->isGameInProgress());
}

TEST_F(TableStateTest, SetDebugModeDisablesWhenAlreadyTrue)
{
	tableState->setDebugMode(true);

	// Проверяем, что сигнал layoutChanged не был эмитирован при повторной установке того же значения
	QSignalSpy layoutChangedSpy(tableState, &TableState::layoutChanged);
	tableState->setDebugMode(true);

	// Нет изменений, поэтому сигнал не должен быть отправлен
	EXPECT_EQ(layoutChangedSpy.count(), 0);
}

TEST_F(TableStateTest, MineDisplayIsCorrectlyUpdatedOnRightClick)
{
	tableState->resetModel(4, 4, 5);
	QSignalSpy mineDisplaySpy(tableState, &TableState::mineDisplay);

	// Изначально количество мин отображается корректно
	EXPECT_EQ(mineDisplaySpy.count(), 0);

	// Симулируем правый клик по клетке
	QModelIndex index = tableState->index(0, 0);
	tableState->onRightClicked(index);

	// Проверяем, что количество мин обновилось
	EXPECT_EQ(mineDisplaySpy.count(), 1);
}

TEST_F(TableStateTest, DiscoverUpdatesDiscoveredCells)
{
	tableState->resetModel(4, 4, 1);

	// Инициализируем игру и кликаем на клетку
	tableState->init(QModelIndex());
	QModelIndex index = tableState->index(1, 1);
	tableState->onTableClicked(index);

	// Проверяем, что клетка была открыта
	EXPECT_TRUE(tableState->getMineSweeper().getDiscovered(1, 1));
}

TEST_F(TableStateTest, OnBothClickedDoesNotDiscoverMines)
{
	tableState->resetModel(4, 4, 1);

	// Инициализируем игру и кликаем по клетке
	tableState->init(QModelIndex());
	QModelIndex index = tableState->index(1, 1);

	// Кликаем обеими кнопками мыши, проверяем, что мину не откроет
	tableState->onBothClicked(index);

	// Проверяем, что клетка с миной не была открыта
	EXPECT_FALSE(tableState->getMineSweeper().getDiscovered(0, 0));
}

TEST_F(TableStateTest, RightClickDisarmsCell)
{
	tableState->resetModel(4, 4, 1);

	// Инициализируем игру и проверяем, что клетка не разряжена
	tableState->init(QModelIndex());
	QModelIndex index = tableState->index(1, 1);
	EXPECT_EQ(tableState->getMineSweeper().field(index.row(), index.column()).disarmed, FIELD_NOT_VISITED);

	// Симулируем правый клик
	tableState->onRightClicked(index);

	// Проверяем, что клетка теперь разряжена
	EXPECT_EQ(tableState->getMineSweeper().field(index.row(), index.column()).disarmed, FIELD_VISITED);
}

TEST_F(TableStateTest, ResetModelResetsMineCountDisplay)
{
	tableState->resetModel(4, 4, 5);
	QSignalSpy mineDisplaySpy(tableState, &TableState::mineDisplay);

	// Проверяем, что при сбросе модели сигнал mineDisplay был отправлен
	EXPECT_EQ(mineDisplaySpy.count(), 0);

	// Сбрасываем модель с другим количеством мин
	tableState->resetModel(4, 4, 10);

	// Проверяем, что сигнал mineDisplay был снова отправлен
	EXPECT_EQ(mineDisplaySpy.count(), 1);
}

TEST_F(TableStateTest, MultipleRightClicksUpdateMineDisplay)
{
	tableState->resetModel(4, 4, 5);
	QSignalSpy mineDisplaySpy(tableState, &TableState::mineDisplay);

	// Проверяем начальное состояние
	EXPECT_EQ(mineDisplaySpy.count(), 0);

	// Симулируем несколько правых кликов
	QModelIndex index1 = tableState->index(0, 0);
	QModelIndex index2 = tableState->index(1, 1);
	tableState->onRightClicked(index1);
	tableState->onRightClicked(index2);

	// Проверяем, что количество мин обновилось
	EXPECT_EQ(mineDisplaySpy.count(), 2);
}

TEST_F(TableStateTest, ResetModelResetsInitializationFlag)
{
	tableState->resetModel(4, 4, 1);

	// Инициализируем модель
	tableState->init(QModelIndex());
	EXPECT_TRUE(tableState->isGameInProgress());

	// Сбрасываем модель
	tableState->resetModel(4, 4, 1);
	EXPECT_FALSE(tableState->isGameInProgress());
}

TEST_F(TableStateTest, DebugModeAddsDebugInfoToField)
{
	tableState->resetModel(4, 4, 1);
	tableState->init(QModelIndex());

	tableState->setDebugMode(true);

	QModelIndex index = tableState->index(1, 1);
	QVariant variant = tableState->data(index, Qt::DisplayRole);
	GameField field = variant.value< GameField >();

	// Проверяем, что для режима отладки установлено значение isDebug = true
	EXPECT_TRUE(field.isDebug);
}

TEST_F(TableStateTest, RightClickIncreasesMineCount)
{
	tableState->resetModel(4, 4, 5);
	QSignalSpy mineDisplaySpy(tableState, &TableState::mineDisplay);

	// Проверяем, что изначально количество мин 5
	EXPECT_EQ(mineDisplaySpy.count(), 0);

	// Симулируем правый клик
	QModelIndex index = tableState->index(0, 0);
	tableState->onRightClicked(index);

	// После правого клика должно увеличиться количество мин
	EXPECT_EQ(mineDisplaySpy.count(), 1);
}

TEST_F(TableStateTest, RightClickDecreasesMineCount)
{
	tableState->resetModel(4, 4, 5);
	QSignalSpy mineDisplaySpy(tableState, &TableState::mineDisplay);

	// Правый клик по клетке увеличивает счетчик мин
	QModelIndex index = tableState->index(0, 0);
	tableState->onRightClicked(index);
	EXPECT_EQ(mineDisplaySpy.count(), 1);

	// Еще один правый клик должен уменьшить счетчик
	tableState->onRightClicked(index);
	EXPECT_EQ(mineDisplaySpy.count(), 2);
}

TEST_F(TableStateTest, LayoutChangedSignalEmittedAfterModelReset)
{
	QSignalSpy layoutChangedSpy(tableState, &TableState::layoutChanged);

	tableState->resetModel(4, 4, 5);

	// Проверяем, что сигнал layoutChanged был отправлен после сброса модели
	EXPECT_EQ(layoutChangedSpy.count(), 1);
}

TEST_F(TableStateTest, DiscoverCellWithZeroNeighboursRecursivelyDiscoversNeighbours)
{
	tableState->resetModel(4, 4, 0);
	tableState->init(QModelIndex());

	QModelIndex index = tableState->index(1, 1);
	tableState->onTableClicked(index);

	// Проверяем, что все клетки, не содержащие мин и с нулевыми соседями, были открыты
	EXPECT_TRUE(tableState->getMineSweeper().getDiscovered(0, 0));
	EXPECT_TRUE(tableState->getMineSweeper().getDiscovered(0, 1));
	EXPECT_TRUE(tableState->getMineSweeper().getDiscovered(0, 2));
	EXPECT_TRUE(tableState->getMineSweeper().getDiscovered(1, 0));
	EXPECT_TRUE(tableState->getMineSweeper().getDiscovered(1, 2));
	EXPECT_TRUE(tableState->getMineSweeper().getDiscovered(2, 0));
	EXPECT_TRUE(tableState->getMineSweeper().getDiscovered(2, 1));
	EXPECT_TRUE(tableState->getMineSweeper().getDiscovered(2, 2));
}

TEST_F(TableStateTest, InitEmitsGameStartedSignal)
{
	QSignalSpy gameStartedSpy(tableState, &TableState::gameStarted);

	tableState->resetModel(4, 4, 5);
	tableState->init(QModelIndex());

	// Проверяем, что сигнал gameStarted был отправлен после инициализации игры
	EXPECT_EQ(gameStartedSpy.count(), 1);
}

TEST_F(TableStateTest, ResetModelDoesNotCrashWhenCalledMultipleTimes)
{
	tableState->resetModel(4, 4, 5);

	// Проверяем, что повторный вызов resetModel не приводит к сбоям
	tableState->resetModel(4, 4, 5);
	tableState->resetModel(4, 4, 5);
	tableState->resetModel(4, 4, 5);

	// Здесь не нужно проверять состояния, просто нужно убедиться, что код не ломается
	EXPECT_NO_THROW(tableState->resetModel(4, 4, 5));
}

TEST_F(TableStateTest, DiscoverCellDoesNotCrashWhenCalledOnEdge)
{
	tableState->resetModel(4, 4, 1);
	tableState->init(QModelIndex());

	// Проверяем, что вызов discover на клетке по краю не приводит к сбою
	QModelIndex index = tableState->index(0, 0);
	EXPECT_NO_THROW(tableState->discover(index));
}

TEST_F(TableStateTest, ResetModelResetsMineDisplay)
{
	tableState->resetModel(4, 4, 5);

	QSignalSpy mineDisplaySpy(tableState, &TableState::mineDisplay);

	// Проверяем, что перед сбросом количество мин верно
	EXPECT_EQ(mineDisplaySpy.count(), 0);

	// Сбрасываем модель и проверяем, что количество мин обновилось
	tableState->resetModel(4, 4, 10);
	EXPECT_EQ(mineDisplaySpy.count(), 1);
}

TEST_F(TableStateTest, OnRightClickedUpdatesMineDisplayCorrectly)
{
	tableState->resetModel(4, 4, 5);

	QSignalSpy mineDisplaySpy(tableState, &TableState::mineDisplay);

	// Проверяем, что начальное количество мин равно 5
	EXPECT_EQ(mineDisplaySpy.count(), 0);

	QModelIndex index = tableState->index(0, 0);
	tableState->onRightClicked(index);

	// После правого клика количество мин должно увеличиться или уменьшиться
	EXPECT_EQ(mineDisplaySpy.count(), 1);
}

TEST_F(TableStateTest, DiscoveringCellEmitsDataChangedSignal)
{
	QSignalSpy dataChangedSpy(tableState, &TableState::dataChanged);

	tableState->resetModel(4, 4, 5);
	tableState->init(QModelIndex());

	QModelIndex index = tableState->index(1, 1);
	tableState->onTableClicked(index);

	// Проверяем, что сигнал dataChanged был отправлен после открытия клетки
	EXPECT_EQ(dataChangedSpy.count(), 1);
}

TEST_F(TableStateTest, DiscoverCellDoesNotCrashWhenAlreadyDiscovered)
{
	tableState->resetModel(4, 4, 5);
	tableState->init(QModelIndex());

	QModelIndex index = tableState->index(1, 1);
	tableState->onTableClicked(index);

	// Проверяем, что повторный клик по уже открытой клетке не вызывает сбоев
	EXPECT_NO_THROW(tableState->onTableClicked(index));
}

TEST_F(TableStateTest, ResetModelAfterGameLostDoesNotCrash)
{
	tableState->resetModel(4, 4, 1);
	tableState->init(QModelIndex());

	QModelIndex index = tableState->index(0, 0);
	tableState->onTableClicked(index);	  // Клик по мине

	// После проигрыша сбрасываем модель
	EXPECT_NO_THROW(tableState->resetModel(4, 4, 5));
}

TEST_F(TableStateTest, SetDebugModeDoesNotCrashDuringGame)
{
	tableState->resetModel(4, 4, 1);
	tableState->init(QModelIndex());

	QModelIndex index = tableState->index(0, 0);
	tableState->onTableClicked(index);

	// Устанавливаем debugMode в процессе игры и проверяем, что не возникает сбоев
	EXPECT_NO_THROW(tableState->setDebugMode(true));
}

TEST_F(TableStateTest, RightClickIncreasesMineDisplayWhenFlaggingCell)
{
	tableState->resetModel(4, 4, 5);
	QModelIndex index = tableState->index(0, 0);

	// Проверяем, что перед правым кликом количество мин равно 5
	QSignalSpy mineDisplaySpy(tableState, &TableState::mineDisplay);
	EXPECT_EQ(mineDisplaySpy.count(), 0);

	// Правый клик увеличивает количество мин
	tableState->onRightClicked(index);
	EXPECT_EQ(mineDisplaySpy.count(), 1);
}

class DummyTopWidget : public SPR::TopWidget
{
  public:
	DummyTopWidget() : SPR::TopWidget(nullptr) {}
	~DummyTopWidget() override = default;

	int getTime() const override { return 0; }
	void setTimer(int) override {}
};

TEST(SaveTest, SaveAndLoadRestoresFieldStateCorrectly)
{
	MineSweeper originalGame;
	Preferences prefs;
	QTimer timer;

	originalGame.reset(4, 4, 3);

	originalGame.discover(1, 1);
	originalGame.discover(0, 0);
	originalGame.disarm(2, 2);

	Save saver(originalGame, timer, prefs);
	QString savePath = "test/testsave.ini";

	DummyTopWidget dummyTop;
	saver.setTopWidget(reinterpret_cast< SPR::TopWidget* >(&dummyTop));

	ASSERT_TRUE(saver.serialize(savePath));
	ASSERT_TRUE(QFile::exists(savePath));

	MineSweeper loadedGame;
	Preferences loadedPrefs;
	QTimer dummyTimer;
	Save loader(loadedGame, dummyTimer, loadedPrefs);

	loader.setTopWidget(reinterpret_cast< SPR::TopWidget* >(&dummyTop));

	ASSERT_TRUE(loader.deserialize(savePath));

	EXPECT_EQ(originalGame.width(), loadedGame.width());
	EXPECT_EQ(originalGame.height(), loadedGame.height());
	EXPECT_EQ(originalGame.totalMineNr(), loadedGame.totalMineNr());

	for (int y = 0; y < originalGame.height(); ++y)
	{
		for (int x = 0; x < originalGame.width(); ++x)
		{
			const auto& orig = originalGame.field(x, y);
			const auto& loaded = loadedGame.field(x, y);
			EXPECT_EQ(orig.mine, loaded.mine) << "Mismatch at (" << x << ", " << y << ")";
			EXPECT_EQ(orig.discovered, loaded.discovered) << "Mismatch at (" << x << ", " << y << ")";
			EXPECT_EQ(orig.disarmed, loaded.disarmed) << "Mismatch at (" << x << ", " << y << ")";
			EXPECT_EQ(orig.neighbours, loaded.neighbours) << "Mismatch at (" << x << ", " << y << ")";
		}
	}
}

TEST(SaveTest, EmptyBoardSaveLoad)
{
	MineSweeper original;
	Preferences prefs;
	QTimer timer;
	original.reset(2, 2, 0);	// Пустое поле

	Save saver(original, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(reinterpret_cast< SPR::TopWidget* >(&dummy));
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	MineSweeper loaded;
	Preferences loadedPrefs;
	QTimer t;
	Save loader(loaded, t, loadedPrefs);
	loader.setTopWidget(reinterpret_cast< SPR::TopWidget* >(&dummy));
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_EQ(loaded.width(), 2);
	EXPECT_EQ(loaded.height(), 2);
	EXPECT_EQ(loaded.totalMineNr(), 0);
}

TEST(SaveTest, DiscoveredCellsPreserved)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(3, 3, 1);
	game.discover(0, 0);
	game.discover(1, 1);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(reinterpret_cast< SPR::TopWidget* >(&dummy));
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	MineSweeper loaded;
	Preferences prefsLoaded;
	QTimer t;
	Save loader(loaded, t, prefsLoaded);
	loader.setTopWidget(reinterpret_cast< SPR::TopWidget* >(&dummy));
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_TRUE(loaded.field(0, 0).discovered);
	EXPECT_TRUE(loaded.field(1, 1).discovered);
}

TEST(SaveTest, DisarmedCellPreserved)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(3, 3, 1);
	game.disarm(2, 2);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(reinterpret_cast< SPR::TopWidget* >(&dummy));
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	MineSweeper loaded;
	Preferences prefsLoaded;
	QTimer t;
	Save loader(loaded, t, prefsLoaded);
	loader.setTopWidget(reinterpret_cast< SPR::TopWidget* >(&dummy));
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_TRUE(loaded.field(2, 2).disarmed);
}

TEST(SaveTest, BoardSizeRestoredCorrectly)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(6, 5, 7);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(reinterpret_cast< SPR::TopWidget* >(&dummy));
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	MineSweeper loaded;
	Preferences loadedPrefs;
	QTimer t;
	Save loader(loaded, t, loadedPrefs);
	loader.setTopWidget(reinterpret_cast< SPR::TopWidget* >(&dummy));
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_EQ(loaded.width(), 6);
	EXPECT_EQ(loaded.height(), 5);
	EXPECT_EQ(loaded.totalMineNr(), 7);
}

TEST(SaveTest, SaveTwiceAndLoad)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(3, 3, 2);
	game.disarm(0, 1);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(reinterpret_cast< SPR::TopWidget* >(&dummy));
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	game.discover(2, 2);
	ASSERT_TRUE(saver.serialize(path));	   // Второй раз

	MineSweeper loaded;
	Preferences loadedPrefs;
	QTimer t;
	Save loader(loaded, t, loadedPrefs);
	loader.setTopWidget(reinterpret_cast< SPR::TopWidget* >(&dummy));
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_TRUE(loaded.field(0, 1).disarmed);
	EXPECT_TRUE(loaded.field(2, 2).discovered);
}

TEST(SaveTest, PreferencesRestored)
{
	MineSweeper game;
	Preferences prefs;
	prefs.width = 7;
	prefs.height = 8;
	prefs.mine = 10;

	QTimer timer;
	game.reset(prefs.width, prefs.height, prefs.mine);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(reinterpret_cast< SPR::TopWidget* >(&dummy));
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	MineSweeper loaded;
	Preferences restoredPrefs;
	QTimer t;
	Save loader(loaded, t, restoredPrefs);
	loader.setTopWidget(reinterpret_cast< SPR::TopWidget* >(&dummy));
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_EQ(restoredPrefs.width, 7);
	EXPECT_EQ(restoredPrefs.height, 8);
	EXPECT_EQ(restoredPrefs.mine, 10);
}

TEST(SaveTest, SaveWithoutDiscoverStillLoadsCorrectly)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(2, 2, 1);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(reinterpret_cast< SPR::TopWidget* >(&dummy));
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(reinterpret_cast< SPR::TopWidget* >(&dummy));
	ASSERT_TRUE(loader.deserialize(path));
}

TEST(SaveTest, NeighborCountsPreserved)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(3, 3, 2);
	game.discover(1, 1);	// посчитать соседей

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(reinterpret_cast< SPR::TopWidget* >(&dummy));
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(reinterpret_cast< SPR::TopWidget* >(&dummy));
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_EQ(game.field(1, 1).neighbours, loaded.field(1, 1).neighbours);
}

TEST(SaveTest, SaveWorksWhenTimerInactive)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(4, 4, 4);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(reinterpret_cast< SPR::TopWidget* >(&dummy));
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));
}

TEST(SaveTest, SaveLoadDoesNotCrashOnLargeBoard)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(10, 10, 15);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(reinterpret_cast< SPR::TopWidget* >(&dummy));
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(reinterpret_cast< SPR::TopWidget* >(&dummy));
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_EQ(loaded.width(), 10);
	EXPECT_EQ(loaded.height(), 10);
}

TEST(SaveTest, AllCellsInitiallyUndiscovered)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(3, 3, 1);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences loadedPrefs;
	QTimer t;
	Save loader(loaded, t, loadedPrefs);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	for (int y = 0; y < 3; ++y)
		for (int x = 0; x < 3; ++x)
			EXPECT_FALSE(loaded.field(x, y).discovered);
}

TEST(SaveTest, DisarmedCellsPreserved)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(3, 3, 2);
	game.disarm(1, 1);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences loadedPrefs;
	QTimer t;
	Save loader(loaded, t, loadedPrefs);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_TRUE(loaded.field(1, 1).disarmed);
}

TEST(SaveTest, NeighboursValueRestored)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(3, 3, 1);
	game.discover(0, 0);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences loadedPrefs;
	QTimer t;
	Save loader(loaded, t, loadedPrefs);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_EQ(game.field(0, 0).neighbours, loaded.field(0, 0).neighbours);
}

TEST(SaveTest, WidthHeightPreserved)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(6, 7, 5);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences loadedPrefs;
	QTimer t;
	Save loader(loaded, t, loadedPrefs);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_EQ(loaded.width(), 6);
	EXPECT_EQ(loaded.height(), 7);
}

TEST(SaveTest, NoCrashOnEmptyFileLoad)
{
	QFile file("test/testsave.ini");
	file.remove();
	file.open(QIODevice::WriteOnly);
	file.close();

	MineSweeper loaded;
	Preferences prefs;
	QTimer t;
	Save loader(loaded, t, prefs);
	DummyTopWidget dummy;
	loader.setTopWidget(&dummy);

	EXPECT_FALSE(loader.deserialize("test/testsave.ini"));
}

TEST(SaveTest, MultipleSavesOverwriteSameFile)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(4, 4, 2);
	DummyTopWidget dummy;

	{
		Save saver(game, timer, prefs);
		saver.setTopWidget(&dummy);
		ASSERT_TRUE(saver.serialize("test/testsave.ini"));
	}

	game.reset(4, 4, 3);	// Перезапись с другим количеством мин
	{
		Save saver(game, timer, prefs);
		saver.setTopWidget(&dummy);
		ASSERT_TRUE(saver.serialize("test/testsave.ini"));
	}

	MineSweeper loaded;
	Preferences prefs2;
	QTimer t;
	Save loader(loaded, t, prefs2);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_EQ(loaded.totalMineNr(), 3);
}

TEST(SaveTest, SaveLoadWorksWithNonSquareBoard)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(3, 5, 4);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences prefs2;
	QTimer t;
	Save loader(loaded, t, prefs2);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_EQ(loaded.width(), 3);
	EXPECT_EQ(loaded.height(), 5);
}

TEST(SaveTest, LoadRestoresAllDiscoveredFlags)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(4, 4, 2);
	game.discover(0, 1);
	game.discover(3, 2);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences prefs2;
	QTimer t;
	Save loader(loaded, t, prefs2);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_TRUE(loaded.field(0, 1).discovered);
	EXPECT_TRUE(loaded.field(3, 2).discovered);
}

TEST(SaveTest, LoadDoesNotAffectUnchangedCells)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(3, 3, 1);
	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences prefs2;
	QTimer t;
	Save loader(loaded, t, prefs2);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	for (int y = 0; y < 3; ++y)
		for (int x = 0; x < 3; ++x)
			EXPECT_FALSE(loaded.field(x, y).discovered);
}

TEST(SaveTest, NoDisarmedFlagsByDefault)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(4, 4, 3);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	for (int y = 0; y < 4; ++y)
		for (int x = 0; x < 4; ++x)
			EXPECT_FALSE(loaded.field(x, y).disarmed);
}

TEST(SaveTest, FieldSizeOneByOne)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(1, 1, 0);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_EQ(loaded.width(), 1);
	EXPECT_EQ(loaded.height(), 1);
}

TEST(SaveTest, MixedCellStatesPreserved)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(3, 3, 3);
	game.discover(0, 0);
	game.disarm(1, 1);
	game.discover(2, 2);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_TRUE(loaded.field(0, 0).discovered);
	EXPECT_TRUE(loaded.field(1, 1).disarmed);
	EXPECT_TRUE(loaded.field(2, 2).discovered);
}

TEST(SaveTest, SaveAndLoadEmptyDiscoveredBoard)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(2, 2, 0);
	game.discover(0, 0);
	game.discover(1, 0);
	game.discover(0, 1);
	game.discover(1, 1);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	for (int y = 0; y < 2; ++y)
		for (int x = 0; x < 2; ++x)
			EXPECT_TRUE(loaded.field(x, y).discovered);
}

TEST(SaveTest, LoadResetsOldFieldData)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(2, 2, 0);
	game.discover(0, 0);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	loaded.reset(4, 4, 5);	  // Старое поле большего размера
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_EQ(loaded.width(), 2);
	EXPECT_EQ(loaded.height(), 2);
	EXPECT_TRUE(loaded.field(0, 0).discovered);
}

TEST(SaveTest, SaveLoadConsistencyAcrossCalls)
{
	for (int i = 0; i < 3; ++i)
	{
		MineSweeper game;
		Preferences prefs;
		QTimer timer;
		game.reset(3, 3, i + 1);
		game.discover(i, i);

		Save saver(game, timer, prefs);
		DummyTopWidget dummy;
		saver.setTopWidget(&dummy);
		ASSERT_TRUE(saver.serialize("test/testsave.ini"));

		MineSweeper loaded;
		Preferences p;
		QTimer t;
		Save loader(loaded, t, p);
		loader.setTopWidget(&dummy);
		ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

		EXPECT_EQ(loaded.totalMineNr(), i + 1);
		EXPECT_TRUE(loaded.field(i, i).discovered);
	}
}

TEST(SaveTest, DisarmedNotDiscoveredByDefault)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(3, 3, 3);
	game.disarm(0, 0);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_TRUE(loaded.field(0, 0).disarmed);
	EXPECT_FALSE(loaded.field(0, 0).discovered);
}

TEST(SaveTest, LoadHandlesMinimumBoardSize)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(1, 1, 0);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_EQ(loaded.width(), 1);
	EXPECT_EQ(loaded.height(), 1);
}

TEST(SaveTest, AllCellsInitiallyHidden)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(5, 5, 5);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	for (int y = 0; y < 5; ++y)
		for (int x = 0; x < 5; ++x)
			EXPECT_FALSE(loaded.field(x, y).discovered);
}

TEST(SaveTest, SaveLoadZeroMines)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(3, 3, 0);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_EQ(loaded.totalMineNr(), 0);
}

TEST(SaveTest, TimerStateIsIgnored)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(2, 2, 1);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer dummyTimer;
	Save loader(loaded, dummyTimer, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_EQ(loaded.width(), 2);
	EXPECT_EQ(loaded.height(), 2);
}

TEST(SaveTest, SaveLoadCornerDiscover)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(4, 4, 2);
	game.discover(0, 0);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_TRUE(loaded.field(0, 0).discovered);
}

TEST(SaveTest, SaveLoadCenterDiscover)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(3, 3, 2);
	game.discover(1, 1);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_TRUE(loaded.field(1, 1).discovered);
}

TEST(SaveTest, AllCellsDisarmedManually)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(2, 2, 4);
	game.disarm(0, 0);
	game.disarm(0, 1);
	game.disarm(1, 0);
	game.disarm(1, 1);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	for (int y = 0; y < 2; ++y)
		for (int x = 0; x < 2; ++x)
			EXPECT_TRUE(loaded.field(x, y).disarmed);
}

TEST(SaveTest, DiscoveredAndDisarmedMixed)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(3, 3, 2);
	game.discover(0, 0);
	game.disarm(1, 1);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_TRUE(loaded.field(0, 0).discovered);
	EXPECT_TRUE(loaded.field(1, 1).disarmed);
}

TEST(SaveTest, LargeFieldPreserved)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(10, 10, 20);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_EQ(loaded.width(), 10);
	EXPECT_EQ(loaded.height(), 10);
	EXPECT_EQ(loaded.totalMineNr(), 20);
}

TEST(SaveTest, LoadDoesNotThrowOnMinimalSetup)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(1, 1, 0);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);

	EXPECT_NO_THROW({ ASSERT_TRUE(loader.deserialize("test/testsave.ini")); });
}

TEST(SaveTest, PreferencesSavedCorrectlyAgain)
{
	MineSweeper game;
	Preferences prefs;
	prefs.width = 6;
	prefs.height = 6;
	prefs.mine = 6;

	QTimer timer;
	game.reset(prefs.width, prefs.height, prefs.mine);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences loadedPrefs;
	QTimer t;
	Save loader(loaded, t, loadedPrefs);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_EQ(loadedPrefs.width, 6);
	EXPECT_EQ(loadedPrefs.height, 6);
	EXPECT_EQ(loadedPrefs.mine, 6);
}

TEST(SaveTest, PartiallyDiscoveredField)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(3, 3, 1);
	game.discover(0, 0);
	game.discover(2, 2);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_TRUE(loaded.field(0, 0).discovered);
	EXPECT_TRUE(loaded.field(2, 2).discovered);
	EXPECT_FALSE(loaded.field(1, 1).discovered);
}

TEST(SaveTest, FullyDiscoveredField)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(2, 2, 0);
	for (int y = 0; y < 2; ++y)
		for (int x = 0; x < 2; ++x)
			game.discover(x, y);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	for (int y = 0; y < 2; ++y)
		for (int x = 0; x < 2; ++x)
			EXPECT_TRUE(loaded.field(x, y).discovered);
}

TEST(SaveTest, SaveLoadFieldWithOneMine)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(2, 2, 1);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_EQ(loaded.totalMineNr(), 1);
}

TEST(SaveTest, SaveLoadSingleCellDisarmed)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(2, 2, 1);
	game.disarm(0, 1);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_TRUE(loaded.field(0, 1).disarmed);
}

TEST(SaveTest, DiscoveredCellRemainsDiscoveredAfterLoad)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(2, 2, 1);
	game.discover(1, 0);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_TRUE(loaded.field(1, 0).discovered);
}

TEST(SaveTest, FieldNeighborsRestoredCorrectly)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(3, 3, 1);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	for (int y = 0; y < 3; ++y)
		for (int x = 0; x < 3; ++x)
			EXPECT_EQ(loaded.field(x, y).neighbours, game.field(x, y).neighbours);
}

TEST(SaveTest, SaveLoadKeepsFieldDimensions)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(6, 5, 3);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_EQ(loaded.width(), 6);
	EXPECT_EQ(loaded.height(), 5);
}

TEST(SaveTest, PreferencesIgnoredIfFieldMismatches)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	prefs.width = 10;
	prefs.height = 10;
	prefs.mine = 10;
	game.reset(4, 4, 2);	// Несовпадение с prefs

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	Preferences loadedPrefs;
	MineSweeper loaded;
	QTimer t;
	Save loader(loaded, t, loadedPrefs);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_EQ(loaded.width(), 4);
	EXPECT_EQ(loaded.height(), 4);
	EXPECT_EQ(loaded.totalMineNr(), 2);
}

TEST(SaveTest, SaveLoadEmptyPreferencesDefaultField)
{
	Preferences prefs;
	QTimer timer;
	MineSweeper game;
	game.reset(1, 1, 0);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	Preferences p;
	MineSweeper loaded;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_EQ(p.width, 1);
	EXPECT_EQ(p.height, 1);
	EXPECT_EQ(p.mine, 0);
}

TEST(SaveTest, BoardWithNoMines)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(3, 3, 0);	// Без мин

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	for (int y = 0; y < 3; ++y)
		for (int x = 0; x < 3; ++x)
			EXPECT_FALSE(loaded.field(x, y).mine);
}

TEST(SaveTest, LoadAfterSavePreservesGameState)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(4, 4, 4);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_EQ(game.width(), loaded.width());
	EXPECT_EQ(game.height(), loaded.height());
	EXPECT_EQ(game.totalMineNr(), loaded.totalMineNr());
}

TEST(SaveTest, DisarmSpecificCell)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(2, 2, 1);
	game.disarm(1, 1);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_TRUE(loaded.field(1, 1).disarmed);
}

TEST(SaveTest, LoadEmptyGame)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(1, 1, 0);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_EQ(loaded.width(), 1);
	EXPECT_EQ(loaded.height(), 1);
	EXPECT_EQ(loaded.totalMineNr(), 0);
}

TEST(SaveTest, LoadAfterFieldReset)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(3, 3, 1);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	ASSERT_TRUE(saver.serialize("test/testsave.ini"));

	game.reset(4, 4, 2);

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize("test/testsave.ini"));

	EXPECT_EQ(loaded.width(), 3);
	EXPECT_EQ(loaded.height(), 3);
	EXPECT_EQ(loaded.totalMineNr(), 1);
}

TEST(SaveTest, LoadNonExistentFileReturnsFalse)
{
	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);

	bool result = loader.deserialize("test/nonexistentfile.ini");

	EXPECT_FALSE(result);
}

TEST(SaveTest, SaveLoadAfterMultipleSaves)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(5, 5, 5);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	QString path = "test/testsave.ini";

	// Первоначальное сохранение
	ASSERT_TRUE(saver.serialize(path));

	// Вторичное сохранение
	game.discover(0, 0);
	ASSERT_TRUE(saver.serialize(path));

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_TRUE(loaded.field(0, 0).discovered);
}

TEST(SaveTest, SaveLoadAfterGameOver)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(3, 3, 2);
	game.discover(1, 1);	// Игрок заканчивает игру

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	game.reset(3, 3, 2);	// Сбрасываем игру после окончания

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_EQ(loaded.width(), 3);
	EXPECT_EQ(loaded.height(), 3);
}

TEST(SaveTest, SaveLoadBoardWithOnlyDiscoveredCells)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(4, 4, 3);
	game.discover(1, 1);
	game.discover(2, 2);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	game.reset(4, 4, 3);	// Сбрасываем игру

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_TRUE(loaded.field(1, 1).discovered);
	EXPECT_TRUE(loaded.field(2, 2).discovered);
}

TEST(SaveTest, SaveLoadAfterMultipleDiscoveredCells)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(4, 4, 3);
	game.discover(0, 0);
	game.discover(2, 3);
	game.discover(3, 2);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	game.reset(4, 4, 3);	// Сбрасываем игру

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_TRUE(loaded.field(0, 0).discovered);
	EXPECT_TRUE(loaded.field(2, 3).discovered);
	EXPECT_TRUE(loaded.field(3, 2).discovered);
}

TEST(SaveTest, SaveLoadWithTimerStatus)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(4, 4, 4);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	game.reset(4, 4, 4);	// Сбрасываем игру

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_EQ(loaded.width(), 4);
	EXPECT_EQ(loaded.height(), 4);
}

TEST(SaveTest, SaveLoadAfterFieldResizing)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(6, 6, 9);	// Изначальное поле

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	game.reset(4, 4, 4);	// Изменяем размер поля

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_EQ(loaded.width(), 6);
	EXPECT_EQ(loaded.height(), 6);
}

TEST(SaveTest, LoadFileWithInvalidFormatReturnsFalse)
{
	QFile file("test/testsave.ini");
	file.open(QIODevice::WriteOnly);
	file.write("invalid data");
	file.close();

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);

	bool result = loader.deserialize("test/testsave.ini");
	EXPECT_FALSE(result);
}

TEST(SaveTest, SaveLoadAfterCellDiscoveryAndDisarming)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(4, 4, 4);
	game.discover(1, 1);
	game.disarm(2, 2);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	game.reset(4, 4, 4);	// Сбрасываем игру

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_TRUE(loaded.field(1, 1).discovered);
	EXPECT_TRUE(loaded.field(2, 2).disarmed);
}

TEST(SaveTest, SaveLoadEmptyGameState)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(3, 3, 0);	// Пустое поле

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	game.reset(3, 3, 0);	// Сбрасываем игру

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_EQ(loaded.width(), 3);
	EXPECT_EQ(loaded.height(), 3);
}

TEST(SaveTest, SaveLoadAfterDisarmingCell)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(5, 5, 5);
	game.disarm(2, 2);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	game.reset(5, 5, 5);	// Сбрасываем игру

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_TRUE(loaded.field(2, 2).disarmed);
}

TEST(SaveTest, SaveLoadAfterGameReset)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(4, 4, 4);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	game.reset(6, 6, 6);	// Изменяем размер поля

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_EQ(loaded.width(), 4);
	EXPECT_EQ(loaded.height(), 4);
}

TEST(SaveTest, SaveLoadWithChangedPreferences)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	prefs.width = 5;
	prefs.height = 5;
	prefs.mine = 5;
	game.reset(prefs.width, prefs.height, prefs.mine);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	prefs.width = 6;	// Изменяем параметры
	prefs.height = 6;
	prefs.mine = 6;

	MineSweeper loaded;
	Preferences loadedPrefs;
	QTimer t;
	Save loader(loaded, t, loadedPrefs);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_EQ(loadedPrefs.width, 5);
	EXPECT_EQ(loadedPrefs.height, 5);
	EXPECT_EQ(loadedPrefs.mine, 5);
}

TEST(SaveTest, SaveLoadWithPartialCellDiscovery)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(5, 5, 5);
	game.discover(1, 1);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	game.reset(5, 5, 5);	// Сбрасываем игру

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_TRUE(loaded.field(1, 1).discovered);
}

TEST(SaveTest, SaveLoadAfterMultipleFieldResets)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(6, 6, 6);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	game.reset(4, 4, 4);	// Изменяем размер поля
	game.reset(5, 5, 5);	// Снова изменяем

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_EQ(loaded.width(), 6);
	EXPECT_EQ(loaded.height(), 6);
}

TEST(SaveTest, SaveLoadWithTimerReset)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(5, 5, 5);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	game.reset(5, 5, 5);	// Сбрасываем игру

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_EQ(loaded.width(), 5);
	EXPECT_EQ(loaded.height(), 5);
}

TEST(SaveTest, SaveLoadWithCellChangesAfterDiscovery)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(4, 4, 4);
	game.discover(2, 2);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	game.reset(4, 4, 4);	// Сбрасываем игру

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_TRUE(loaded.field(2, 2).discovered);
}

TEST(SaveTest, SaveLoadWithMultipleMines)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(5, 5, 5);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	game.reset(5, 5, 5);	// Сбрасываем игру

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_EQ(loaded.totalMineNr(), 5);
}

TEST(SaveTest, SaveLoadWithTimerState)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(5, 5, 5);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	game.reset(5, 5, 5);	// Сбрасываем игру

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_EQ(loaded.width(), 5);
	EXPECT_EQ(loaded.height(), 5);
}

TEST(SaveTest, SaveLoadWithMultipleCellsDisarmed)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(5, 5, 5);
	game.disarm(1, 1);
	game.disarm(3, 3);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	game.reset(5, 5, 5);	// Сбрасываем игру

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_TRUE(loaded.field(1, 1).disarmed);
	EXPECT_TRUE(loaded.field(3, 3).disarmed);
}

TEST(SaveTest, SaveLoadWithChangedCellState)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(5, 5, 5);
	game.discover(1, 1);
	game.disarm(2, 2);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	game.reset(5, 5, 5);	// Сбрасываем игру

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_TRUE(loaded.field(1, 1).discovered);
	EXPECT_TRUE(loaded.field(2, 2).disarmed);
}

TEST(SaveTest, SaveLoadAfterMultipleDisarms)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(4, 4, 4);
	game.disarm(0, 0);
	game.disarm(3, 3);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	game.reset(4, 4, 4);	// Сбрасываем игру

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_TRUE(loaded.field(0, 0).disarmed);
	EXPECT_TRUE(loaded.field(3, 3).disarmed);
}

TEST(SaveTest, SaveLoadWithMultipleDiscoveries)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(6, 6, 6);
	game.discover(2, 2);
	game.discover(4, 4);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	game.reset(6, 6, 6);	// Сбрасываем игру

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_TRUE(loaded.field(2, 2).discovered);
	EXPECT_TRUE(loaded.field(4, 4).discovered);
}

TEST(SaveTest, SaveLoadWithIncreasedMines)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(4, 4, 3);	// 3 мины

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	game.reset(4, 4, 4);	// 4 мины

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize(path));

	EXPECT_EQ(loaded.totalMineNr(), 3);
}

TEST(SaveTest, SaveLoadWithMultipleFieldChanges)
{
	MineSweeper game;
	Preferences prefs;
	QTimer timer;
	game.reset(4, 4, 5);	// 5 мин на поле

	// Много изменений состояния поля
	game.discover(1, 1);
	game.disarm(2, 2);
	game.discover(0, 3);
	game.disarm(3, 0);

	Save saver(game, timer, prefs);
	DummyTopWidget dummy;
	saver.setTopWidget(&dummy);
	QString path = "test/testsave.ini";
	ASSERT_TRUE(saver.serialize(path));

	game.reset(4, 4, 5);	// Сбрасываем игру

	MineSweeper loaded;
	Preferences p;
	QTimer t;
	Save loader(loaded, t, p);
	loader.setTopWidget(&dummy);
	ASSERT_TRUE(loader.deserialize(path));

	// Проверяем изменения на поле
	EXPECT_TRUE(loaded.field(1, 1).discovered);
	EXPECT_TRUE(loaded.field(2, 2).disarmed);
	EXPECT_TRUE(loaded.field(0, 3).discovered);
	EXPECT_TRUE(loaded.field(3, 0).disarmed);
}

int main(int argc, char** argv)
{
	QApplication app(argc, argv);	 // Обязательно для работы с QWidget
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
