#include <SFML/Graphics/RectangleShape.hpp>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <random>
#include <thread>
#include <vector>

// Program's parameters
static const unsigned int WINDOW_HEIGHT = 900;
static const unsigned int WINDOW_WIDTH = 900;
static const unsigned int CELL_SIZE = 8;
static const unsigned int ROW_MAX = WINDOW_HEIGHT / CELL_SIZE;
static const unsigned int COL_MAX = WINDOW_WIDTH / CELL_SIZE;
static const unsigned int CONTROL_PANEL_HEIGHT = 20;

static const unsigned int horiz_offset = 10;
static const unsigned int vert_offset = WINDOW_HEIGHT + 10;
static const unsigned int bt_spacing = 110;

class Button {
    private:
        sf::RectangleShape shape;
        sf::Text label;
        sf::Font font;

    public:
        Button(const std::string &text, const sf::Vector2f &position, const sf::Vector2f& size) {
            shape.setPosition(position);
            shape.setSize(size);
            shape.setFillColor(sf::Color(200, 200, 200));
            shape.setOutlineThickness(1);
            shape.setOutlineColor(sf::Color(100, 100, 100));

            if (!font.loadFromFile("/usr/share/fonts-hack/woff/hack-regular-subset.woff")) {
                std::cerr << "Error loading font" << std::endl;
            }
            label.setFont(font);
            label.setString(text);
            label.setCharacterSize(12);
            label.setFillColor(sf::Color::Black);

            sf::FloatRect textBounds = label.getLocalBounds();
            label.setPosition(
                    position.x + (size.x - textBounds.width) / 2,
                    position.y + (size.y - textBounds.height) / 2
            );
        }

        void draw(sf::RenderWindow &window) {
            window.draw(shape);
            window.draw(label);
        }

        bool is_clicked(sf::Vector2i mouse_pos) {
            return shape.getGlobalBounds().contains(mouse_pos.x, mouse_pos.y);
        }
};

class GameOfLife {
    private:
        std::vector<std::vector<bool>> curr_grid;
        std::vector<std::vector<bool>> next_grid;
        unsigned int delay_ms;
        sf::RenderWindow window;
        std::mt19937 rng;
        bool is_running;
        Button start_button;
        Button reset_button;
        Button random_button;
        Button speedup_button;
        Button slowdown_button;

        void randomize() {
            std::uniform_int_distribution<int> dist(0, 1);
            for (unsigned int i = 0; i < ROW_MAX; i++) {
                for (unsigned int j = 0; j < COL_MAX; j++) {
                    curr_grid[i][j] = dist(rng);
                }
            }
        }

        void clear() {
            for (unsigned int i = 0; i < ROW_MAX; i++) {
                for (unsigned int j = 0; j < COL_MAX; j++) {
                    curr_grid[i][j] = false;
                }
            }
        }

        int count_neighbours(int row, int col) {
            int count = 0;

            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    if ((i == 0) && (j == 0))
                        continue;
                    if (this->curr_grid[(row + i + ROW_MAX) % ROW_MAX][(col + j + COL_MAX) % COL_MAX]) {
                        count++;
                    }
                }
            }
            return (count);
        }

        void update() {
            int neighbours;

            this->curr_grid[50][50] = true;
            for (int i = 0; i < ROW_MAX; i++) {
                for (int j = 0; j < COL_MAX; j++) {
                    neighbours = count_neighbours(i, j);
                    if (this->curr_grid[i][j] == true) {
                        this->next_grid[i][j] = (neighbours == 2) || (neighbours == 3);
                    }
                    else {
                        this->next_grid[i][j] = (neighbours == 3);
                    }
                }
            }
            this->curr_grid = this->next_grid;
        }

        void draw() {
            window.clear(sf::Color::White);

            // Draw grid
            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
            for (unsigned int i = 0; i < ROW_MAX; i++) {
                for (unsigned int j = 0; j < COL_MAX; j++) {
                    cell.setPosition(i * CELL_SIZE, j * CELL_SIZE);
                    cell.setFillColor(this->curr_grid[i][j] ? sf::Color::Black : sf::Color::White);
                    cell.setOutlineColor(sf::Color(128, 128, 128));
                    cell.setOutlineThickness(1);
                    window.draw(cell);
                }
            }

            // Draw buttons
            start_button.draw(window);
            reset_button.draw(window);
            random_button.draw(window);
            speedup_button.draw(window);
            slowdown_button.draw(window);

            window.display();
        }
    public:
        GameOfLife():
            curr_grid(ROW_MAX, std::vector<bool>(COL_MAX, false)),
            next_grid(ROW_MAX, std::vector<bool>(COL_MAX, false)),
            delay_ms(100),
            window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT + CONTROL_PANEL_HEIGHT), "GameOfLife"),
            rng(std::random_device{}()),
            is_running(false),
            start_button("start/pause", sf::Vector2f(horiz_offset, vert_offset), sf::Vector2f(100, 40)),
            reset_button("reset", sf::Vector2f(horiz_offset + bt_spacing, vert_offset), sf::Vector2f(100, 40)),
            random_button("random", sf::Vector2f(horiz_offset + 2 * bt_spacing, vert_offset), sf::Vector2f(100, 40)),
            speedup_button("speed+", sf::Vector2f(horiz_offset + 3 * bt_spacing, vert_offset), sf::Vector2f(100, 40)),
            slowdown_button("speed-", sf::Vector2f(horiz_offset + 4 * bt_spacing, vert_offset), sf::Vector2f(100, 40)) {
                this->window.setFramerateLimit(60);
            }

        void run() {
            while (window.isOpen()) {
                sf::Event event;
                while (window.pollEvent(event)) {
                    if (event.type == sf::Event::Closed)
                        window.close();
                    if (event.type == sf::Event::MouseButtonPressed) {
                        sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);

                        if (start_button.is_clicked(mouse_pos)) {
                            is_running = !is_running;
                        }
                        else if (reset_button.is_clicked(mouse_pos)) {
                            clear();
                        }
                        else if (random_button.is_clicked(mouse_pos)) {
                            randomize();
                        }
                        else if (speedup_button.is_clicked(mouse_pos)) {
                            if (delay_ms > 0)
                                delay_ms -= 25;
                        }
                        else if (slowdown_button.is_clicked(mouse_pos)) {
                            delay_ms += 25;
                        }
                        else if (mouse_pos.x < WINDOW_HEIGHT && mouse_pos.y < WINDOW_WIDTH) {
                            unsigned int x = mouse_pos.x / CELL_SIZE;
                            unsigned int y = mouse_pos.y / CELL_SIZE;
                            if (x < ROW_MAX && y < COL_MAX) {
                                curr_grid[x][y] = !curr_grid[x][y];
                            }
                        }
                    }
                    if (event.type == sf::Event::KeyPressed) {
                        if (event.key.code == sf::Keyboard::Space)
                            is_running = !is_running;
                        if (event.key.code == sf::Keyboard::R)
                            clear();
                    }
                }
                if (is_running)
                    update();
                draw();
                std::this_thread::sleep_for(std::chrono::milliseconds(this->delay_ms));
            }
        }
};

int main() {
    GameOfLife game;
    game.run();
    return (0);
}
