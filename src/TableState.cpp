#include <include/TableState.h>

namespace SPR
{

	TableState::TableState(QObject *parent) :
		QAbstractTableModel(parent), _model(), m_mineDisplay(0), m_initialized(false)
	{
		m_highlightClearTimer = new QTimer(this);
		m_highlightClearTimer->setSingleShot(true);
		connect(
			m_highlightClearTimer,
			&QTimer::timeout,
			this,
			[this]()
			{
				_model.clearHighlights();
				emit layoutChanged();
			});
	}

	void TableState::init(const QModelIndex &index)
	{
		_model.populate(index.row(), index.column());
		m_initialized = true;
		emit gameStarted();
		emit layoutChanged();
	}

	void TableState::setDebugMode(bool debug)
	{
		if (_debugMode != debug)
		{
			_debugMode = debug;
			emit layoutChanged();	 // Refresh the view
		}
	}

	int TableState::rowCount(const QModelIndex &parent) const
	{
		Q_UNUSED(parent);
		return _model.width();
	}

	int TableState::columnCount(const QModelIndex &parent) const
	{
		Q_UNUSED(parent);
		return _model.height();
	}

	QVariant TableState::data(const QModelIndex &index, int role) const
	{
		GameField field = _model.fieldConst(index.row(), index.column());

		QVariant variant;
		variant.setValue(field);

		// Debug mode handling
		if (_debugMode)
		{
			GameField fieldCopy = field;
			fieldCopy.isDebug = true;
			variant.setValue(fieldCopy);
		}

		if (field.isHighlighted)
		{
			field.isDebug = true;
		}

		return variant;
	}

	bool TableState::hasLost() const
	{
		for (int y = 0; y < _model.height(); ++y)
		{
			for (int x = 0; x < _model.width(); ++x)
			{
				const auto &field = _model.fieldConst(x, y);
				if (field.mine && field.discovered)
				{
					return true;	// Player clicked on a mine
				}
			}
		}
		return false;
	}

	bool TableState::isGameInProgress() const
	{
		return m_initialized &&					 // Game has started
			   !_model.checkWinCondition() &&	 // Not won
			   !hasLost();						 // Not lost
	}

	void TableState::resetModel(int width, int height, int mine)
	{
		m_initialized = false;
		m_mineDisplay = mine;
		_model.reset(width, height, mine);
		emit mineDisplay(mine);
		emit layoutChanged();
	}

	void TableState::onTableClicked(const QModelIndex &index)
	{
		if (_model.field(index.row(), index.column()).disarmed == 0)
		{
			discover(index);
		}
	}

	void TableState::discover(const QModelIndex &index)
	{
		if (!m_initialized)
		{
			init(index);
		}

		const int x = index.row();
		const int y = index.column();

		_model.discover(x, y);

		if (_model.field(x, y).neighbours == 0 && _model.field(x, y).mine == 0)
		{
			if (!_model.getDiscovered(x - 1, y - 1))
			{	 // up
				discover(index.sibling(x - 1, y - 1));
			}
			if (!_model.getDiscovered(x, y - 1))
			{
				discover(index.sibling(x, y - 1));
			}
			if (!_model.getDiscovered(x + 1, y - 1))
			{
				discover(index.sibling(x + 1, y - 1));
			}
			if (!_model.getDiscovered(x - 1, y))
			{
				discover(index.sibling(x - 1, y));
			}	 // mid
			if (!_model.getDiscovered(x + 1, y))
			{
				discover(index.sibling(x + 1, y));
			}
			if (!_model.getDiscovered(x - 1, y + 1))
			{
				discover(index.sibling(x - 1, y + 1));
			}	 // down
			if (!_model.getDiscovered(x, y + 1))
			{
				discover(index.sibling(x, y + 1));
			}
			if (!_model.getDiscovered(x + 1, y + 1))
			{
				discover(index.sibling(x + 1, y + 1));
			}
		}

		emit dataChanged(index, index);

		if (_model.field(x, y).mine && _model.field(x, y).disarmed == 0)
		{
			emit gameLost();
		}
		else if (_model.checkWinCondition())
		{
			emit gameWon();
		}
	}

	void TableState::onRightClicked(const QModelIndex &index)
	{
		const int x = index.row();
		const int y = index.column();

		if (_model.field(x, y).discovered == FIELD_NOT_VISITED)
		{
			_model.disarm(x, y);

			if (_model.field(x, y).disarmed == FIELD_VISITED)
			{
				m_mineDisplay--;
			}
			else if (_model.field(x, y).disarmed == FIELD_NOT_VISITED)
			{
				m_mineDisplay++;
			}

			emit mineDisplay(m_mineDisplay);
			emit dataChanged(index, index);
		}
	}

	void TableState::onBothClicked(const QModelIndex &index)
	{
		const int x = index.row();
		const int y = index.column();

		if (_model.getDiscovered(x, y) && _model.getNeighbours(x, y) == _model.countFlagsAround(x, y))
		{
			if (!_model.getDiscovered(x - 1, y - 1))
			{
				discover(index.sibling(x - 1, y - 1));	  // up
			}
			if (!_model.getDiscovered(x, y - 1))
			{
				discover(index.sibling(x, y - 1));
			}
			if (!_model.getDiscovered(x + 1, y - 1))
			{
				discover(index.sibling(x + 1, y - 1));
			}
			if (!_model.getDiscovered(x - 1, y))
			{
				discover(index.sibling(x - 1, y));	  // mid
			}
			if (!_model.getDiscovered(x + 1, y))
			{
				discover(index.sibling(x + 1, y));
			}
			if (!_model.getDiscovered(x - 1, y + 1))
			{
				discover(index.sibling(x - 1, y + 1));	  // down
			}
			if (!_model.getDiscovered(x, y + 1))
			{
				discover(index.sibling(x, y + 1));
			}
			if (!_model.getDiscovered(x + 1, y + 1))
			{
				discover(index.sibling(x + 1, y + 1));
			}
		}
	}

	void TableState::onMiddleClicked(const QModelIndex &index)
	{
		const int x = index.row();
		const int y = index.column();

		if (!_model.getDiscovered(x, y))
		{
			return;
		}

		_model.clearHighlights();

		if (_model.getNeighbours(x, y) != _model.countFlagsAround(x, y))
		{
			int minX = x, maxX = x;
			int minY = y, maxY = y;

			for (int dx = -1; dx <= 1; ++dx)
			{
				for (int dy = -1; dy <= 1; ++dy)
				{
					if (dx == 0 && dy == 0)
					{
						continue;
					}

					int nx = x + dx;
					int ny = y + dy;

					if (!_model.getDiscovered(nx, ny) && !_model.getFlag(nx, ny))
					{
						_model.markTemporary(nx, ny);
						minX = std::min(minX, nx);
						maxX = std::max(maxX, nx);
						minY = std::min(minY, ny);
						maxY = std::max(maxY, ny);
					}
				}
			}

			emit dataChanged(index.sibling(minX, minY), index.sibling(maxX, maxY));
			m_highlightClearTimer->start(HIGHLIGHT_TIMEOUT);
		}
		else
		{
			onBothClicked(index);
		}
	}

	MineSweeper &TableState::getMineSweeper()
	{
		return _model;
	}

	const MineSweeper &TableState::getMineSweeper() const
	{
		return _model;
	}

}	 // namespace SPR
