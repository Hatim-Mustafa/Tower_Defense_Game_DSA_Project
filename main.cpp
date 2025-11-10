#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

// ========== Coordinate ==========
struct Coordinate {
    int x, y;
};

// ========== Path Node (Linked List) ==========
struct PathNode {
    Coordinate pos;
    PathNode* next;
};

// ========== Enemy ==========
class Enemy {
    int health, speed;
    PathNode* current;
    sf::CircleShape shape;
public:
    Enemy(int hp, int spd, PathNode* start) : health(hp), speed(spd), current(start) {
        shape.setRadius(10);
        shape.setFillColor(sf::Color::Red);
        shape.setPosition(start->pos.x * 40, start->pos.y * 40);
    }

    void move() {
        if (current && current->next) {
            current = current->next;
            shape.setPosition(current->pos.x * 40, current->pos.y * 40);
        }
    }

    void draw(sf::RenderWindow& window) { window.draw(shape); }
};

// ========== Tower ==========
class Tower {
    Coordinate pos;
    int range;
    sf::CircleShape shape;
public:
    Tower(int x, int y, int rng) {
        pos = { x, y };
        range = rng;
        shape.setRadius(15);
        shape.setFillColor(sf::Color::Blue);
        shape.setPosition(x * 40, y * 40);
    }

    void draw(sf::RenderWindow& window) { window.draw(shape); }
};

// ========== Map ==========
class Map {
    int rows, cols;
    vector<vector<int>> grid;
    vector<sf::RectangleShape> tiles;
public:
    Map(int r, int c) : rows(r), cols(c) {
        grid.assign(rows, vector<int>(cols, 0));
        // Manually mark a simple path
        for (int i = 0; i < cols; i++) grid[5][i] = 1; // Horizontal path

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                sf::RectangleShape rect(sf::Vector2f(40, 40));
                rect.setPosition(j * 40, i * 40);
                if (grid[i][j] == 1) rect.setFillColor(sf::Color(150, 75, 0)); // path
                else rect.setFillColor(sf::Color(0, 200, 0)); // grass
                rect.setOutlineThickness(1);
                rect.setOutlineColor(sf::Color::Black);
                tiles.push_back(rect);
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        for (auto& t : tiles) window.draw(t);
    }

    bool isBuildable(int x, int y) { return grid[y][x] == 0; }
};

// ========== Game Manager ==========
class GameManager {
    Map* gameMap;
    vector<Tower*> towers;
    vector<Enemy*> enemies;
    PathNode* pathHead;
    sf::Clock clock;
    float enemyMoveTimer;
public:
    GameManager() {
        gameMap = new Map(10, 15);
        enemyMoveTimer = 0;

        // Create simple path (linked list)
        pathHead = new PathNode{ {0, 5}, nullptr };
        PathNode* cur = pathHead;
        for (int i = 1; i < 15; i++) {
            cur->next = new PathNode{ {i, 5}, nullptr };
            cur = cur->next;
        }

        // Spawn one enemy
        enemies.push_back(new Enemy(100, 1, pathHead));

        // Place one tower
        towers.push_back(new Tower(3, 3, 2));
    }

    void update() {
        float dt = clock.getElapsedTime().asSeconds();
        if (dt - enemyMoveTimer > 0.5f) {
            for (auto& e : enemies) e->move();
            enemyMoveTimer = dt;
        }
    }

    void draw(sf::RenderWindow& window) {
        gameMap->draw(window);
        for (auto& t : towers) t->draw(window);
        for (auto& e : enemies) e->draw(window);
    }
};

// ========== MAIN ==========
int main() {
    sf::RenderWindow window(sf::VideoMode(600, 400), "Tower Defense - SFML Demo");
    window.setFramerateLimit(60);

    GameManager game;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        game.update();

        window.clear();
        game.draw(window);
        window.display();
    }

    return 0;
}
