#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

#define GRID_SIZE 40

// ========== Coordinate ==========
struct Coordinate {
    int x, y;
};

// ========== Path Node (Linked List) ==========
struct PathNode {
    Coordinate pos;
    PathNode* front;
    PathNode* left;
    PathNode* right;
    PathNode(Coordinate p) : pos(p), front(nullptr), left(nullptr), right(nullptr) {}
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
        shape.setPosition(start->pos.x * GRID_SIZE, start->pos.y * GRID_SIZE);
    }

    void move() {
        if (current && current->front) {
            current = current->front;
            shape.setPosition(current->pos.x * GRID_SIZE, current->pos.y * GRID_SIZE);
        }
        else if (current&& current->left) {
			current = current->left;
			shape.setPosition(current->pos.x * GRID_SIZE, current->pos.y * GRID_SIZE);
		}
		else if (current && current->right) {
			current = current->right;
			shape.setPosition(current->pos.x * GRID_SIZE, current->pos.y * GRID_SIZE);
        }
        // You can add logic here to choose left/right at branches
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
        shape.setPosition(x * GRID_SIZE, y * GRID_SIZE);
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
        // Manually mark the path based on your image
        grid = {
            {1, 1, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
            {0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
			{1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1},
			{1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1},
            {1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 2, 2, 1, 1},
            {1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 2, 2, 1, 1},
            {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
            {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1},
        };

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                sf::RectangleShape rect(sf::Vector2f(GRID_SIZE, GRID_SIZE));
                rect.setPosition(j * GRID_SIZE, i * GRID_SIZE);
                if (grid[i][j] == 0) rect.setFillColor(sf::Color(150, 150, 150)); // path
				else if (grid[i][j] == 2) rect.setFillColor(sf::Color(0, 0, 0)); // special path
                else rect.setFillColor(sf::Color(0, 200, 0)); // grass
                tiles.push_back(rect);
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        for (auto& t : tiles) window.draw(t);
    }

    bool isBuildable(int x, int y) {
        if (x < 0 || x >= cols || y < 0 || y >= rows) return false;
        return grid[y][x] == 1;
    }

    bool isPath(int x, int y) {
        if (x < 0 || x >= cols || y < 0 || y >= rows) return false;
        return grid[y][x] == 0;
    }
};

void buildPathNetwork(PathNode* start, string movement, Map* gameMap) {
    // Implement path network building logic here if needed
	if (movement == "UP") {
        if (gameMap->isPath(start->pos.x, start->pos.y - 2)) {
            PathNode* next = new PathNode({ start->pos.x, start->pos.y - 1 });
            start->front = next;
            PathNode* next2 = new PathNode({ start->pos.x, start->pos.y - 2 });
            next->front = next2;
            buildPathNetwork(next2, movement, gameMap);
        }
        if (gameMap->isPath(start->pos.x -2, start->pos.y)) {
            PathNode* next = new PathNode({ start->pos.x - 1, start->pos.y });
            start->left = next;
            PathNode* next2 = new PathNode({ start->pos.x -2, start->pos.y});
            next->front = next2;
            buildPathNetwork(next2, "LEFT", gameMap);
        }
        if (gameMap->isPath(start->pos.x + 2, start->pos.y) && 
            (start->pos.x != 3 || start->pos.y != 4) && 
            (start->pos.x != 9 || start->pos.y != 8)) {
            PathNode* next = new PathNode({ start->pos.x + 1, start->pos.y });
            start->right = next;
            PathNode* next2 = new PathNode({ start->pos.x + 2, start->pos.y });
            next->front = next2;
            buildPathNetwork(next2, "RIGHT", gameMap);
        }
    }
    if (movement == "LEFT") {
        if (gameMap->isPath(start->pos.x, start->pos.y - 2)) {
            PathNode* next = new PathNode({ start->pos.x, start->pos.y - 1 });
            start->right = next;
            PathNode* next2 = new PathNode({ start->pos.x, start->pos.y - 2 });
            next->front = next2;
            buildPathNetwork(next2, "UP", gameMap);
        }
        if (gameMap->isPath(start->pos.x - 2, start->pos.y)) {
            PathNode* next = new PathNode({ start->pos.x - 1, start->pos.y });
            start->front = next;
            PathNode* next2 = new PathNode({ start->pos.x - 2, start->pos.y });
            next->front = next2;
            buildPathNetwork(next2, movement, gameMap);
        }
        if (gameMap->isPath(start->pos.x, start->pos.y + 2) && 
            (start->pos.x != 3 || start->pos.y != 4) &&
            (start->pos.x != 9 || start->pos.y != 8)) {
            PathNode* next = new PathNode({ start->pos.x, start->pos.y + 1});
            start->left = next;
            PathNode* next2 = new PathNode({ start->pos.x, start->pos.y + 2});
            next->front = next2;
            buildPathNetwork(next2, "DOWN", gameMap);
        }
    }
    if (movement == "RIGHT") {
        if (gameMap->isPath(start->pos.x, start->pos.y - 2)) {
            PathNode* next = new PathNode({ start->pos.x, start->pos.y - 1 });
            start->left = next;
            PathNode* next2 = new PathNode({ start->pos.x, start->pos.y - 2 });
            next->front = next2;
            buildPathNetwork(next2, "UP", gameMap);
        }
        if (gameMap->isPath(start->pos.x, start->pos.y + 2)) {
            PathNode* next = new PathNode({ start->pos.x, start->pos.y + 1 });
            start->right = next;
            PathNode* next2 = new PathNode({ start->pos.x, start->pos.y + 2 });
            next->front = next2;
            buildPathNetwork(next2, "DOWN", gameMap);
        }
        if (gameMap->isPath(start->pos.x + 2, start->pos.y)) {
            PathNode* next = new PathNode({ start->pos.x + 1, start->pos.y });
            start->front = next;
            PathNode* next2 = new PathNode({ start->pos.x + 2, start->pos.y });
            next->front = next2;
            buildPathNetwork(next2, movement, gameMap);
        }
    }
    if (movement == "DOWN") {
        if (gameMap->isPath(start->pos.x, start->pos.y + 2)) {
            PathNode* next = new PathNode({ start->pos.x, start->pos.y + 1 });
            start->front = next;
            PathNode* next2 = new PathNode({ start->pos.x, start->pos.y + 2 });
            next->front = next2;
            buildPathNetwork(next2, movement, gameMap);
        }
        if (gameMap->isPath(start->pos.x - 2, start->pos.y)) {
            PathNode* next = new PathNode({ start->pos.x - 1, start->pos.y });
            start->right = next;
            PathNode* next2 = new PathNode({ start->pos.x - 2, start->pos.y });
            next->front = next2;
            buildPathNetwork(next2, "LEFT", gameMap);
        }
        if (gameMap->isPath(start->pos.x + 2, start->pos.y)) {
            PathNode* next = new PathNode({ start->pos.x + 1, start->pos.y });
            start->left = next;
            PathNode* next2 = new PathNode({ start->pos.x + 2, start->pos.y });
            next->front = next2;
            buildPathNetwork(next2, "RIGHT", gameMap);
        }
    }
}

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
        gameMap = new Map(14, 26);
        enemyMoveTimer = 0;

        // Build the path network
        PathNode* start = new PathNode({ 21,13 });
		PathNode* second = new PathNode({ 21,12 });
		start->front = second;
		buildPathNetwork(second, "UP", gameMap);

        pathHead = start;

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
        /* for (auto& t : towers) t->draw(window);*/
        for (auto& e : enemies) e->draw(window);
    }
};

// ========== MAIN ==========
int main() {
    sf::RenderWindow window(sf::VideoMode(26 * GRID_SIZE, 14 * GRID_SIZE), "Tower Defense - SFML Demo");
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
