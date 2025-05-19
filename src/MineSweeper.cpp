#include <include/MineSweeper.h>

namespace SPR
{

	MineSweeper::MineSweeper() : m_width(0), m_height(0), m_totalMineNr(0), m_discoveredFieldsNr(0), m_data() {}

	void MineSweeper::reset(int width, int height, int mineNumber)
	{
		m_width = width;
		m_height = height;
		m_totalMineNr = mineNumber;
		m_discoveredFieldsNr = 0;	 // no fields open so zero obviously
		m_data.clear();

		const int fieldsNr = size();
		m_data.reserve(fieldsNr);
		m_data.insert(m_data.begin(), fieldsNr, GameField());

		srand(std::time(0));
	}

	void MineSweeper::populate(int xToSkip, int yToSkip)
	{
		populateMineCrew(xToSkip, yToSkip);
		populateNeighbourhood();
	}

	void MineSweeper::populateMineCrew(int xToSkip, int yToSkip)
	{
		if (m_totalMineNr >= size())
		{
			qWarning() << "Warning: Total mine number exceeds the size of the field!";
			return;
		}

		const uint64_t nomineFieldId = yToSkip * m_width + xToSkip;
		int64_t mineMade = 0;	 // just counter, starts at zero

		while (mineMade < m_totalMineNr)
		{
			const uint64_t fieldId = std::rand() % size();

			if (fieldId >= static_cast< size_t >(m_data.size()))	// Приведено к size_t
			{
				qWarning() << "Warning: Invalid field index generated!" << fieldId;
				continue;
			}

			if (m_data[fieldId].mine == 0 && fieldId != nomineFieldId)
			{
				m_data[fieldId].mine = 1;
				mineMade++;
			}
		}
	}

	bool MineSweeper::checkWinCondition() const
	{
		const int fieldsToDiscover = size() - m_discoveredFieldsNr - m_totalMineNr;
		return fieldsToDiscover == 0;	 // no fields left
	}

	void MineSweeper::populateNeighbourhood()
	{
		for (int x = 0; x < m_width; ++x)
		{
			for (int y = 0; y < m_height; ++y)
			{
				const int mineValue =
					getMine(x - 1, y - 1)											   // up
					+ getMine(x, y - 1) + getMine(x + 1, y - 1) + getMine(x - 1, y)	   // mid
					+ getMine(x, y) + getMine(x + 1, y) + getMine(x - 1, y + 1)		   // down
					+ getMine(x, y + 1) + getMine(x + 1, y + 1);

				GameField tempField = field(x, y);
				tempField.neighbours = mineValue;
				m_data[y * m_width + x] = tempField;
			}
		}
	}

	int MineSweeper::getFlag(int x, int y) const
	{
		if (isValidIndex(x, y) && fieldConst(x, y).disarmed == 1)
		{
			return 1;
		}
		return 0;
	}

	int MineSweeper::totalMineNr() const
	{
		return m_totalMineNr;
	}

	int MineSweeper::width() const
	{
		return m_width;
	}

	int MineSweeper::height() const
	{
		return m_height;
	}

	long long MineSweeper::size() const
	{
		return m_width * m_height;
	}

	int MineSweeper::countFlagsAround(int x, int y) const
	{
		return getFlag(x - 1, y - 1)											// up
			 + getFlag(x, y - 1) + getFlag(x + 1, y - 1) + getFlag(x - 1, y)	// mid
			 + getFlag(x, y) + getFlag(x + 1, y) + getFlag(x - 1, y + 1)		// down
			 + getFlag(x, y + 1) + getFlag(x + 1, y + 1);
	}

	int MineSweeper::getNeighbours(int x, int y) const
	{
		if (isValidIndex(x, y))
		{
			return fieldConst(x, y).neighbours;
		}
		return 0;
	}

	int MineSweeper::getMine(int x, int y) const
	{
		if (isValidIndex(x, y))
		{
			return fieldConst(x, y).mine;
		}
		return 0;
	}

	bool MineSweeper::getDiscovered(int x, int y) const
	{
		if (isValidIndex(x, y))
		{
			return fieldConst(x, y).discovered != FIELD_NOT_VISITED;
		}
		return true;
	}

	void MineSweeper::discover(int x, int y)
	{
		if (isValidIndex(x, y))
		{
			GameField fieldCopy = field(x, y);
			if (fieldCopy.discovered == FIELD_NOT_VISITED && fieldCopy.disarmed == FIELD_NOT_VISITED)
			{
				fieldCopy.discovered = FIELD_VISITED;
				m_data[y * m_width + x] = fieldCopy;
				m_discoveredFieldsNr++;
			}
		}
	}

	void MineSweeper::disarm(int x, int y)
	{
		if (isValidIndex(x, y))
		{
			GameField fieldCopy = field(x, y);
			if (fieldCopy.disarmed < PLAYER_NOT_SURE)
			{
				fieldCopy.disarmed++;
				m_data[y * m_width + x] = fieldCopy;
			}
			else if (fieldCopy.disarmed == PLAYER_NOT_SURE)
			{
				fieldCopy.disarmed = FIELD_NOT_VISITED;
				m_data[y * m_width + x] = fieldCopy;
			}
		}
	}

	bool MineSweeper::isValidIndex(int x, int y) const
	{
		return (x >= 0 && x < m_width && y >= 0 && y < m_height);
	}

	GameField& MineSweeper::field(int x, int y)
	{
		const int id = y * m_width + x;
		Q_ASSERT(id >= 0 && id < size());
		return m_data[id];
	}

	const GameField& MineSweeper::fieldConst(int x, int y) const
	{
		const int id = y * m_width + x;
		Q_ASSERT(id >= 0 && id < size());
		return m_data[id];
	}

	void MineSweeper::markTemporary(int x, int y)
	{
		field(x, y).isHighlighted = true;
	}

	void MineSweeper::clearHighlights()
	{
		for (int x = 0; x < width(); ++x)
		{
			for (int y = 0; y < height(); ++y)
			{
				field(x, y).isHighlighted = false;
			}
		}
	}
}	 // namespace SPR
