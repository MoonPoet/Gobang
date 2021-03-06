#include "gobangwithui.h"
#include <chrono>
#include <random>

GobangWithUI::GobangWithUI(QWidget *parent)
	: QMainWindow(parent), moves_count(0), difficulty(5)
{
	Board::BoardTable table;
	std::default_random_engine generator(time(nullptr));
	std::uniform_int_distribution<long long> dis(0, 9223372036854775807LL); //long long max
	for (auto& row : table)
		for (auto& col : row)
		{
			col.first = dis(generator);
			col.second = dis(generator);
		}
	Board::init_table(table);
	setMaximumHeight(640);
	setMinimumHeight(640);
	setMaximumSize(640, 640);
	setMinimumSize(640, 640);
	ui.setupUi(this);
	connect(ui.play_black, SIGNAL(triggered()), this, SLOT(set_black_player()));
	connect(ui.play_white, SIGNAL(triggered()), this, SLOT(set_black_computer()));
	connect(ui.AI_opponent, SIGNAL(triggered()), this, SLOT(set_opponent_AI()));
	connect(ui.human_oppenent, SIGNAL(triggered()), this, SLOT(set_opponent_player()));
	connect(ui.clear_board, SIGNAL(triggered()), this, SLOT(clear_board()));
	connect(ui.easy_difficulty, SIGNAL(triggered()), this, SLOT(set_easy_difficulty()));
	connect(ui.normal_difficulty, SIGNAL(triggered()), this, SLOT(set_normal_difficulty()));
	connect(ui.hard_difficulty, SIGNAL(triggered()), this, SLOT(set_hard_difficulty()));
	default_first_move();
}

GobangWithUI::~GobangWithUI()
{

}

void GobangWithUI::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	QPen pen;
	pen.setColor(Qt::black);
	painter.setPen(pen);
	for (int i = 40; i <= 600; i += 40)
	{
		painter.drawLine(QPoint(40, i), QPoint(600, i));
		painter.drawLine(QPoint(i, 40), QPoint(i, 600));
	}
	QBrush brush;
	brush.setStyle(Qt::SolidPattern);
	for (int i = 0; i < moves_count; ++i)
	{
		if (i % 2 == 0)
		{
			brush.setColor(Qt::black);
		}
		else
		{
			brush.setColor(Qt::white);
		}
		painter.setBrush(brush);
		painter.drawEllipse(point_state[i].first - 20, point_state[i].second - 20, 40, 40);
	}
}

void GobangWithUI::set_enabled(bool b)
{
	ui.play_black->setEnabled(b);
	ui.play_white->setEnabled(b);
	ui.easy_difficulty->setEnabled(b);
	ui.normal_difficulty->setEnabled(b);
	ui.hard_difficulty->setEnabled(b);
}

void GobangWithUI::set_black_computer()
{
	Board::set_black_computer();
	clear();
	default_first_move();
}

void GobangWithUI::set_black_player()
{
	Board::set_black_player();
	clear();
	update();
}

void GobangWithUI::set_opponent_AI()
{
	set_enabled(true);
	set_black_computer();
}

void GobangWithUI::set_opponent_player()
{
	set_enabled(false);
	Board::set_opponent_player();
	clear();
	update();
}

void GobangWithUI::clear_board()
{
	clear();
	if (board.is_opponent_AI() && Board::computer == Board::State::Black)
		default_first_move();
}

void GobangWithUI::set_easy_difficulty()
{
	difficulty = 3;//search depth
	clear_board();
}

void GobangWithUI::set_normal_difficulty()
{
	difficulty = 5;//search depth
	clear_board();
}

void GobangWithUI::set_hard_difficulty()
{
	difficulty = 7;//search depth
	clear_board();
}

void GobangWithUI::mousePressEvent(QMouseEvent* mouseEvent)
{
	const int x_pos = static_cast<float>(mouseEvent->x() / 40.0) + 0.5;
	const int y_pos = static_cast<float>(mouseEvent->y() / 40.0) + 0.5;
	if (x_pos > 0 && x_pos <= 15 && y_pos > 0 && y_pos <= 15 && board.get_state(x_pos - 1, y_pos - 1) == Board::State::Empty)
	{
		play(x_pos, y_pos);
		const auto winner = board.get_winner();
		if (winner != Board::State::Empty)
		{
			if (Board::is_opponent_AI())
			{
				if (Board::computer == winner)
					computer_win();
				else
					player_win();
			}
			else
			{
				if (winner == Board::State::White)
					player_white_win();
				else
					player_black_win();
			}
		}
		else
		{
			if (moves_count == 225)
			{
				tie();
			}
			else
			{
				if (Board::is_opponent_AI())
				{
					const auto move = negamax.search(board, difficulty);
					play(move.first + 1, move.second + 1);
					const auto winner2 = board.get_winner();
					if (winner2 == Board::computer)
					{
						computer_win();
					}
					else if (moves_count == 225)
					{
						tie();
					}
				}
			}
		}
	}
}

void GobangWithUI::clear()
{
	board.clear();
	moves_count = 0;
	update();
}

void GobangWithUI::default_first_move()
{
	board.play(7, 7, Board::State::Black);
	point_state[moves_count].first = 320;
	point_state[moves_count].second = 320;
	++moves_count;
	update();
}

void GobangWithUI::play(int x_pos, int y_pos)
{
	if (moves_count % 2 == 0)
	{
		board.play(x_pos - 1, y_pos - 1, Board::State::Black);
	}
	else
	{
		board.play(x_pos - 1, y_pos - 1, Board::State::White);
	}
	point_state[moves_count].first = x_pos * 40;
	point_state[moves_count].second = y_pos * 40;
	++moves_count;
	repaint();
}

void GobangWithUI::tie()
{
	int result = QMessageBox::question(this,
		"Game Over",
		"Tie\n Do you want to start a new game?",
		QMessageBox::Yes | QMessageBox::No);
	if (result == QMessageBox::Yes)
	{
		clear();
		if (Board::computer == Board::State::Black)
			default_first_move();
	}
	else
		close();
}

void GobangWithUI::computer_win()
{
	auto result = QMessageBox::question(
		this,
		"Game Over",
		"You lose\n Do you want to start a new game?",
		QMessageBox::Yes | QMessageBox::No);
	if (result == QMessageBox::Yes)
	{
		clear();
		if (Board::computer == Board::State::Black)
			default_first_move();
	}
	else
		close();
}

void GobangWithUI::player_win()
{
	auto result = QMessageBox::question(
		this,
		"Game Over",
		"You win\n Do you want to start a new game?",
		QMessageBox::Yes | QMessageBox::No);
	if (result == QMessageBox::Yes)
	{
		clear();
		if (Board::computer == Board::State::Black)
			default_first_move();
	}
	else
		close();
}

void GobangWithUI::player_black_win()
{
	auto result = QMessageBox::question(
		this,
		"Game Over",
		"Black win\n Do you want to start a new game?",
		QMessageBox::Yes | QMessageBox::No);
	if (result == QMessageBox::Yes)
		clear();
	else
		close();
}

void GobangWithUI::player_white_win()
{
	auto result = QMessageBox::question(
		this,
		"Game Over",
		"White win\n Do you want to start a new game?",
		QMessageBox::Yes | QMessageBox::No);
	if (result == QMessageBox::Yes)
		clear();
	else
		close();
}
