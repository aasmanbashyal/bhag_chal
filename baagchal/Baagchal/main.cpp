#include <SFML/Graphics.hpp>
#include <iostream>
#include <bitset>
#include<fstream>
#include <vector>
#define XSHIFT 200
#define YSHIFT 150
#define XMULT 150
#define YMULT 150

typedef std::bitset<8> Pos;

class Board;

class Coordinates
{
public:
	Coordinates() :x(-10), y(-10) {} //some value which will never appear on the board
	Coordinates(const int & a, const int & b) :x(a), y(b) {}
	bool operator==(const Coordinates & rhs)
	{
		if ((this->x == rhs.x)&(this->y == rhs.y))
			return true;
		return false;
	}
	bool operator!=(const Coordinates & rhs)
	{
		if ((this->x != rhs.x) | (this->y != rhs.y))
			return true;
		return false;
	}
	Coordinates& operator=(const Coordinates & rhs)
	{
		this->x = rhs.x;
		this->y = rhs.y;
		return *this;
	}
	Coordinates& operator+=(const Coordinates & rhs)
	{
		this->x += rhs.x;
		this->y += rhs.y;
		return *this;
	}
	Coordinates& operator-=(const Coordinates & rhs)
	{
		this->x -= rhs.x;
		this->y -= rhs.y;
		return *this;
	}
	int x, y;
};

inline Coordinates operator+(Coordinates lhs, const Coordinates & rhs)
{
	lhs += rhs;
	return lhs;
}

inline Coordinates operator-(Coordinates lhs, const Coordinates & rhs)
{
	lhs -= rhs;
	return lhs;
}

typedef std::vector<Coordinates> CoordVec;

class Piece
{
public:
	Piece(Coordinates & c, std::string s) :position(c), name(s), is_selected(false), is_clicked(false) {}
	virtual void move(Coordinates & p, Board & board) {}
	virtual CoordVec possible_moves(Board & board)
	{
		return CoordVec();
	}
	virtual void die(Board & board) {}
	Coordinates position;
	bool is_selected;
	bool is_clicked;
	sf::Sprite active_sprite;
	std::string name;

	friend void display_piece(Piece *, sf::RenderWindow & win);
};

class Board
{
public:
	Board() :dead_bakhras(0) {}
	CoordVec possible_places(const Coordinates & p1)
	{
		CoordVec possible_positions;
		Pos dirs[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
		Coordinates diradds[8] = { Coordinates(-1, 0), Coordinates(-1, 1), Coordinates(0, 1),
			Coordinates(1, 1), Coordinates(1, 0), Coordinates(1, -1),
			Coordinates(0, -1), Coordinates(-1, -1) };
		for (int i = 0; i < 8; i++)
			if ((b[p1.x][p1.y] & dirs[i]).any())
				possible_positions.push_back(p1 + diradds[i]);
		return possible_positions;
	}
	bool is_game_over()
	{
		CoordVec bagh_moves;
		if (this->dead_bakhras >= 5)
		{
			winner = "Bagh";
			return true;
		}

		for (auto &i : this->pieceslist[0])
		{
			bagh_moves = i->possible_moves(*this);
			if (!bagh_moves.empty())
				return false;  // returns false unless no valid moves left for bagh
		}

		winner = "Bakhra";
		return true;
	}
	bool occupancy[5][5];
	Pos b[5][5];
	std::vector<std::vector<Piece *>> pieceslist;
	Piece * last_clicked_piece;
	sf::Sprite sprite;
	int dead_bakhras;
	std::string winner;
};


class Bagh :public Piece
{
public:
	Bagh(Coordinates & c, Board & board) :Piece(c, std::string("Bagh"))
	{
		board.occupancy[c.x][c.y] = true;
		active_sprite = Bagh::sprite;
		active_sprite.setPosition(sf::Vector2f(XSHIFT + (c.x*XMULT), YSHIFT + (c.y*YMULT)));
		Bagh::count++;
	}
	
	void move(Coordinates & p, Board & board)
	{
		CoordVec bakhra_places;
		CoordVec available_pos = board.possible_places(this->position);
		for (auto &k : available_pos)
		{
			for (auto &l : board.pieceslist[1])
			{
				if (l->position == k) // meaning there is a bakhra there
				{
					bakhra_places = board.possible_places(l->position);
					for (auto &m : bakhra_places)
					{
						if (((l->position - this->position) == (m - l->position)) & !(board.occupancy[m.x][m.y])) // valid jumping for Bagh
						{
							if (m == p)
							{
								// kill the bakhra
								l->die(board);

							}
						}
					}
				}
			}
		}

		board.occupancy[this->position.x][this->position.y] = false;
		board.occupancy[p.x][p.y] = true;
		this->position = p;
	}
	CoordVec possible_moves(Board & board)
	{
		CoordVec pos_pos, bakhra_places;
		CoordVec available_pos = board.possible_places(this->position);
		for (auto &k : available_pos)
		{
			if (!(board.occupancy[k.x][k.y]))
			{
				pos_pos.push_back(k);
			}
			else
			{
				for (auto &l : board.pieceslist[1])
				{
					if (l->position == k) // meaning there is a bakhra there
					{
						bakhra_places = board.possible_places(l->position);
						for (auto &m : bakhra_places)
						{
							if (((l->position - this->position) == (m - l->position)) & !(board.occupancy[m.x][m.y])) // valid jumping for Bagh
							{
								pos_pos.push_back(m);
							}
						}
					}
				}
			}
		}
		return pos_pos;
	}
	static int count;
	static sf::Sprite sprite;
	static sf::Sprite alter_sprite;
};

class Bakhra :public Piece
{
public:
	Bakhra(Coordinates & c, Board & board) :Piece(c, std::string("Bakhra"))
	{
		board.occupancy[c.x][c.y] = true;
		active_sprite = Bakhra::sprite;
		active_sprite.setPosition(sf::Vector2f(XSHIFT + (c.x*XMULT), YSHIFT + (c.y*YMULT)));
		Bakhra::count++;
	}
	void die(Board & board)
	{
		board.occupancy[this->position.x][this->position.y] = false;
		this->position = Coordinates(-10, -10);
		this->active_sprite = sf::Sprite();
		board.dead_bakhras++;
		//sound.play();
	}
	void move(Coordinates & p, Board & board)
	{
		board.occupancy[this->position.x][this->position.y] = false;
		board.occupancy[p.x][p.y] = true;
		this->position = p;
	}
	CoordVec possible_moves(Board & board)
	{
		CoordVec pos_pos;
		CoordVec available_pos = board.possible_places(this->position);
		for (auto &k : available_pos)
			if (!(board.occupancy[k.x][k.y]))
				pos_pos.push_back(k);
		return pos_pos;
	}

	static int count;
	static sf::Sprite sprite;
	static sf::Sprite alter_sprite;
};


namespace scr
{
	inline void switch_sprite(Piece * p)
	{
		if (p->name == "Bagh")
			if (p->is_selected | p->is_clicked)
				p->active_sprite = Bagh::alter_sprite;
			else
				p->active_sprite = Bagh::sprite;
		if (p->name == "Bakhra")
			if (p->is_selected | p->is_clicked)
				p->active_sprite = Bakhra::alter_sprite;
			else
				p->active_sprite = Bakhra::sprite;
	}

	void display_piece(Piece * p, sf::RenderWindow & win)
	{
		p->active_sprite.setPosition(sf::Vector2f(XSHIFT + (p->position.x*XMULT), YSHIFT + (p->position.y*YMULT)));
		win.draw(p->active_sprite);
	}

	void highlight_position(const Coordinates & p, sf::RenderWindow & window, sf::Color color = sf::Color(0, 255, 0, 200))
	{
		sf::CircleShape circle(12);
		circle.setOrigin(circle.getRadius(), circle.getRadius());
		circle.setFillColor(color);
		circle.setPosition(sf::Vector2f(XSHIFT + (p.x*XMULT), YSHIFT + (p.y*YMULT)));
		window.draw(circle);
	}

	bool click_position(const Coordinates & p, sf::RenderWindow & window, sf::Event & event)
	{
		sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);
		if (((mouse_pos.x < (XSHIFT + (p.x*XMULT) + 25))&(mouse_pos.x > (XSHIFT + (p.x*XMULT) - 25))
			&(mouse_pos.y < (YSHIFT + (p.y*YMULT) + 25))&(mouse_pos.y > (YSHIFT + (p.y*YMULT) - 25)))
			&event.mouseButton.button == sf::Mouse::Left)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool mouse_over(Piece * p, sf::RenderWindow & window)
	{
		sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);
		bool flag;
		if (p->active_sprite.getGlobalBounds().contains(mouse_pos.x, mouse_pos.y))
		{
			p->is_selected = true;
			switch_sprite(p);
			return true;
		}
		else
		{
			p->is_selected = false;
			switch_sprite(p);
			return false;
		}
	}
}

int Bagh::count = 0;
int Bakhra::count = 0;
sf::Sprite Bagh::sprite;
sf::Sprite Bakhra::sprite;
sf::Sprite Bagh::alter_sprite;
sf::Sprite Bakhra::alter_sprite;


int main()
{

	sf::RenderWindow window(sf::VideoMode(1000, 1000), "Baghchal Game");
	sf::Texture t3, t4;
	t4.loadFromFile("start.jpg");
	sf::Sprite play(t4);
	play.setPosition(420.0, 420.0);
	play.setScale(1.0, 1.0);
	sf::Font font2;
	if (!font2.loadFromFile("prstartk.ttf"))
	{
		std::cout << "Error loading file" << std::endl;
		system("pause");
	}
	sf::Text text2;
	text2.setFont(font2);
	text2.setString("BAGHCHAL");
	text2.setPosition(350.f, 50.f);
	text2.setCharacterSize(40);
	text2.setFillColor(sf::Color::White);
	text2.setStyle(sf::Text::Style::Bold);
	text2.setOutlineThickness(2);
	while (window.isOpen())
	{

		sf::Vector2i pos = sf::Mouse::getPosition(window);
		sf::Event e;
		while (window.pollEvent(e))
		{
			if (e.type == sf::Event::Closed) { window.close(); }
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				if (play.getGlobalBounds().contains(pos.x, pos.y))
				{
					Board board;


					Pos b[5][5] = { 0x38, 0x2a, 0x3e, 0x2a, 0x0e,
					 0xa8, 0xff, 0xaa, 0xff, 0x8a,
					 0xf8, 0xaa, 0xff, 0xaa, 0x8f,
					 0xa8, 0xff, 0xaa, 0xff, 0x8a,
					 0xe0, 0xa2, 0xe3, 0xa2, 0x83 };
					for (int i = 0; i < 5; i++)
					{
						for (int j = 0; j < 5; j++)
						{
							board.b[i][j] = b[i][j];
							board.occupancy[i][j] = false;
						}
					}

					Coordinates baghcoordlist[] = { Coordinates(0, 0), Coordinates(0, 4), Coordinates(4, 0), Coordinates(4, 4) };
					Coordinates bakhracoordlist[] = { Coordinates(-10, -10), Coordinates(-5, -5) };
					std::vector<Piece *> baghlist;
					std::vector<Piece *> bakhralist;

					sf::Texture piece_texture, piece_alter1_texture, board_texture;
					
					if (!piece_texture.loadFromFile("bagbakhrasprites.png"))
					{
						std::cerr << "could not load pieces texture!" << std::endl;
					}
					if (!piece_alter1_texture.loadFromFile("bagbakhrasprites_selected.png"))
					{
						std::cerr << "could not load pieces texture!" << std::endl;
					}
					if (!board_texture.loadFromFile("board.png"))
					{
						std::cerr << "could not load board texture!" << std::endl;
					}
					board_texture.setSmooth(true);
					piece_texture.setSmooth(true);
					piece_alter1_texture.setSmooth(true);

					board.sprite.setTexture(board_texture);
					board.sprite.setScale(2.f, 2.f);

					Bagh::sprite.setTexture(piece_texture);
					Bagh::sprite.setTextureRect(sf::IntRect(0, 0, 100, 100));
					Bagh::sprite.setScale(0.75f, 0.75f);
					Bagh::sprite.setOrigin(50, 50);
					Bakhra::sprite.setTexture(piece_texture);
					Bakhra::sprite.setTextureRect(sf::IntRect(100, 0, 100, 100));
					Bakhra::sprite.setScale(0.75f, 0.75f);
					Bakhra::sprite.setOrigin(50, 50);

					Bagh::alter_sprite.setTexture(piece_alter1_texture);
					Bagh::alter_sprite.setTextureRect(sf::IntRect(0, 0, 100, 100));
					Bagh::alter_sprite.setScale(0.75f, 0.75f);
					Bagh::alter_sprite.setOrigin(50, 50);
					Bakhra::alter_sprite.setTexture(piece_alter1_texture);
					Bakhra::alter_sprite.setTextureRect(sf::IntRect(100, 0, 100, 100));
					Bakhra::alter_sprite.setScale(0.75f, 0.75f);
					Bakhra::alter_sprite.setOrigin(50, 50);


					for (auto &h : baghcoordlist) //for initialization only
						baghlist.push_back(new Bagh(h, board));

					for (auto &h : bakhracoordlist)	//for initialization only
						bakhralist.push_back(new Bakhra(h, board));


					board.pieceslist =
					{
						{baghlist.begin(), baghlist.end()},
						{bakhralist.begin(), bakhralist.end()}
					};


					int counter = 1;
					int placed_bakhras = 0;
					bool turn = true; // Start with bakhra
					board.last_clicked_piece = board.pieceslist[0][0];
					CoordVec possible_points;
					sf::Font font1;
					if (!font1.loadFromFile("prstartk.ttf"))
					{
						std::cout << "Error loading file" << std::endl;
						system("pause");
					}
					sf::Text text1;
					text1.setFont(font1);
					text1.setString("BAKHRA LEFT::20");
					text1.setPosition(400.f, 50.f);
					text1.setCharacterSize(15);
					text1.setFillColor(sf::Color::White);
					text1.setStyle(sf::Text::Style::Italic);
					text1.setOutlineThickness(2);
					window.setFramerateLimit(20);

					while (window.isOpen())
					{
						if (board.is_game_over())
						{
							std::cout << "Winner is " << board.winner << std::endl;
							break;
						}
						sf::Event event;

						while (window.pollEvent(event))
						{
							if (event.type == sf::Event::Closed)
								window.close();
						}
						window.clear(sf::Color(100, 04, 23));

						board.sprite.setPosition(sf::Vector2f(XSHIFT, YSHIFT));
						window.draw(board.sprite);
						window.draw(text1);
						for (auto &i : board.pieceslist)
							for (auto &j : i)
								scr::display_piece(j, window);

						if ((turn) && (placed_bakhras < 20))
						{
							possible_points.clear();
							for (int i = 0;i < 5;i++)
								for (int j = 0;j < 5;j++)
									if (!board.occupancy[i][j])
										possible_points.push_back(Coordinates(i, j));

							for (auto &k : possible_points)
								scr::highlight_position(k, window);

							for (auto &l : possible_points)
							{
								if (scr::click_position(l, window, event))
								{
									board.pieceslist[1].push_back(new Bakhra(l, board));
									placed_bakhras++;
									turn = !turn;
									if (placed_bakhras == 1)
									{
										text1.setString("BAKHRA LEFT::19");
									}
									else if (placed_bakhras == 2)
									{
										text1.setString("BAKHRA LEFT::18");
									}
									else if (placed_bakhras == 3)
									{
										text1.setString("BAKHRA LEFT::17");
									}
									else if (placed_bakhras == 4)
									{
										text1.setString("BAKHRA LEFT::16");
									}
									else if (placed_bakhras == 5)
									{
										text1.setString("BAKHRA LEFT::15");
									}
									else if (placed_bakhras == 6)
									{
										text1.setString("BAKHRA LEFT::14");
									}
									else if (placed_bakhras == 7)
									{
										text1.setString("BAKHRA LEFT::13");
									}
									else if (placed_bakhras == 8)
									{
										text1.setString("BAKHRA LEFT::12");
									}
									else if (placed_bakhras == 9)
									{
										text1.setString("BAKHRA LEFT::11");
									}
									else if (placed_bakhras == 10)
									{
										text1.setString("BAKHRA LEFT::10");
									}
									else if (placed_bakhras == 11)
									{
										text1.setString("BAKHRA LEFT::9");
									}
									else if (placed_bakhras == 12)
									{
										text1.setString("BAKHRA LEFT::8");
									}
									else if (placed_bakhras == 13)
									{
										text1.setString("BAKHRA LEFT::7");
									}
									else if (placed_bakhras == 14)
									{
										text1.setString("BAKHRA LEFT::6");
									}
									else if (placed_bakhras == 15)
									{
										text1.setString("BAKHRA LEFT::5");
									}
									else if (placed_bakhras == 16)
									{
										text1.setString("BAKHRA LEFT::4");
									}
									else if (placed_bakhras == 17)
									{
										text1.setString("BAKHRA LEFT::3");
									}
									else if (placed_bakhras == 18)
									{
										text1.setString("BAKHRA LEFT::2");
									}
									else if (placed_bakhras = 19)
									{
										text1.setString("BAKHRA LEFT::1");
									}
									else
									{
										text1.setString("BAKHRA LEFT::0");
									}
									possible_points.clear();
								}
							}
						}
						else
						{

							for (auto &j : board.pieceslist[turn ? 1 : 0])
							{
								if (scr::mouse_over(j, window) & event.mouseButton.button == sf::Mouse::Left)
								{
									board.last_clicked_piece->is_clicked = false;
									j->is_clicked = true;
									board.last_clicked_piece = j;
									possible_points = j->possible_moves(board);
								}
							}

							for (auto &k : possible_points)
								scr::highlight_position(k, window);

							for (auto &l : possible_points)
							{
								if (scr::click_position(l, window, event))
								{
									board.last_clicked_piece->move(l, board);
									board.last_clicked_piece->is_clicked = false;
									board.last_clicked_piece->is_selected = false;
									scr::switch_sprite(board.last_clicked_piece);
									possible_points.clear();
									turn = !turn;
								}
							}
						}
						window.display();

						counter++;
					}
					char arr1[2500];
					char arr2[2500];
					std::ifstream obj("abc.txt");
					obj.getline(arr1, 2500);
					obj.close();
					std::ifstream abj("xyz.txt");
					abj.getline(arr2, 2500);
					abj.close();
					sf::Font font;
					if (!font.loadFromFile("prstartk.ttf"))
					{
						std::cout << "Error loading file" << std::endl;
						system("pause");
					}
					sf::Text text;
					text.setFont(font);
					if (board.winner == "Bagh")
					{
						text.setString(arr1);
					}
					else
					{
						text.setString(arr2);
					}
					// in pixels, not points!
					text.setPosition(350.f, 500.f);
					text.setCharacterSize(30);
					text.setFillColor(sf::Color::White);
					text.setStyle(sf::Text::Style::Italic);
					//text.setOutlineColor( sf::Color::Red );
					text.setOutlineThickness(2);
					while (window.isOpen())
					{
						sf::Event event;
						// handle all events
						while (window.pollEvent(event))
						{
							switch (event.type)
							{
							case sf::Event::Closed:
								window.close();
								break;
							}
						}
						// update the game
						window.clear(sf::Color(100, 04, 23));
						window.draw(text);
						window.display();
						system("pause");
					}
				}
				window.close();
			}
		}
		window.clear(sf::Color(100, 04, 23));
		window.draw(play);
		window.draw(text2);
		window.display();
	}
	return 0;
}

