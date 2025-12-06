#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
using namespace std;
using namespace sf;

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
        else if (current && current->left) {
            current = current->left;
            shape.setPosition(current->pos.x * GRID_SIZE, current->pos.y * GRID_SIZE);
        }
        else if (current && current->right) {
            current = current->right;
            shape.setPosition(current->pos.x * GRID_SIZE, current->pos.y * GRID_SIZE);
        }
        // You can add logic here to choose left/right at branches
    }

    void draw(sf::RenderWindow& window) {
        window.draw(shape);

    }
};


// ========== Tower ==========

struct UpgradeNode {
    string name;
    int cost;
    int damageBoost;
    int rangeBoost;
    float fireRateBoost;

    UpgradeNode* left;
    UpgradeNode* right;

    UpgradeNode(string n, int c, int dmg, int rng, float rate)
        : name(n), cost(c), damageBoost(dmg), rangeBoost(rng),
        fireRateBoost(rate), left(nullptr), right(nullptr) {
    }
};

class Tower {
public:
    sf::Vector2i pos;
    int damage;
    int range;
    float fireRate;
    int points;
    bool isSelected = false;
    UpgradeNode* root;
    UpgradeNode* current;

    sf::CircleShape shape;
    sf::CircleShape rangeCircle;

    Tower(sf::Vector2f coord) {
        pos = sf::Vector2i(round(coord.x / GRID_SIZE), round(coord.y / GRID_SIZE));
        damage = 10;
        range = 2 * GRID_SIZE;
        fireRate = 1.0f;
        points = 150;

        shape.setRadius(GRID_SIZE / 2);
        shape.setFillColor(sf::Color::Blue);
        shape.setPosition(pos.x * GRID_SIZE, pos.y * GRID_SIZE);

        rangeCircle.setRadius(range);
        rangeCircle.setOrigin(range, range);
        rangeCircle.setFillColor(sf::Color(0, 0, 255, 50));
        rangeCircle.setPosition(pos.x * GRID_SIZE + GRID_SIZE / 2, pos.y * GRID_SIZE + GRID_SIZE / 2);

        buildUpgradeTree();
        current = root;
    }

    void buildUpgradeTree() {
        root = new UpgradeNode("Base Tower", 0, 10, 0, 0.0f);

        // First tier
        root->left = new UpgradeNode("Wizard Tower", 50, 15, 50, 0.2f);
        root->right = new UpgradeNode("Ice Tower", 50, 8, 20, 0.3f);

        // Wizard Tower upgrades
        root->left->left = new UpgradeNode("Arcane Explosion", 100, 25, 20, 0.15f);
        root->left->right = new UpgradeNode("Cannon Tower", 100, 30, 40, 0.2f);

        // Cannon Tower upgrades
        root->left->right->left = new UpgradeNode("Heavy Shot", 150, 50, 30, 0.25f);
        root->left->right->right = new UpgradeNode("Explosive Shell", 150, 45, 50, 0.2f);

        // Lightning Chain upgrade remains for Wizard
        root->left->left->left = new UpgradeNode("Lightning Chain", 150, 20, 30, 0.1f);

        // Ice Tower upgrades
        root->right->left = new UpgradeNode("Deep Freeze", 100, 5, 25, 0.1f);
        root->right->right = new UpgradeNode("Shatter", 100, 10, 15, 0.1f);
    }


    bool upgrade(UpgradeNode* targetNode) {
        if ((current->left == targetNode || current->right == targetNode) && points >= targetNode->cost) {
            damage += targetNode->damageBoost;
            range += targetNode->rangeBoost;
            fireRate -= targetNode->fireRateBoost;

            rangeCircle.setRadius(range);
            rangeCircle.setOrigin(range, range);

            points -= targetNode->cost;
            current = targetNode;

            cout << "Upgraded to " << current->name << endl;
            return true;
        }
        return false;
    }

    int getAvailableUpgrades(UpgradeNode* options[2]) {
        int count = 0;
        if (current->left) options[count++] = current->left;
        if (current->right) options[count++] = current->right;
        return count;
    }

    bool isClicked(sf::Vector2f mousePos) {
        return shape.getGlobalBounds().contains(mousePos);
    }
    void draw(sf::RenderWindow& window) {
        window.draw(shape);

        if (isSelected) {
            window.draw(rangeCircle); // draw tower range

            // Draw upgrade buttons
            UpgradeNode* options[2];
            int n = getAvailableUpgrades(options);

            for (int i = 0; i < n; i++) {
                sf::RectangleShape btn(sf::Vector2f(100, 30));
                btn.setPosition(pos.x * GRID_SIZE, pos.y * GRID_SIZE + GRID_SIZE + i * 35);
                btn.setFillColor(sf::Color(100, 100, 250));

                sf::Font font;
                font.loadFromFile("arial.ttf"); // make sure you have a font file
                sf::Text txt;
                txt.setFont(font);
                txt.setString(options[i]->name + "\nCost: " + to_string(options[i]->cost));
                txt.setCharacterSize(14);
                txt.setFillColor(sf::Color::White);
                txt.setPosition(btn.getPosition().x + 5, btn.getPosition().y + 5);

                window.draw(btn);
                window.draw(txt);
            }
        }
    }


};


class Shop {
public:
    sf::RectangleShape shopTower;
    bool isDragging;
    Shop() {
        shopTower.setSize(sf::Vector2f(GRID_SIZE * 2, GRID_SIZE * 2));
        shopTower.setFillColor(sf::Color::Red);
        shopTower.setPosition(26 * GRID_SIZE, 0);  // shop location
        isDragging = false;
    }
    void draw(sf::RenderWindow& window) { window.draw(shopTower); }
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
        if (gameMap->isPath(start->pos.x - 2, start->pos.y)) {
            PathNode* next = new PathNode({ start->pos.x - 1, start->pos.y });
            start->left = next;
            PathNode* next2 = new PathNode({ start->pos.x - 2, start->pos.y });
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
            PathNode* next = new PathNode({ start->pos.x, start->pos.y + 1 });
            start->left = next;
            PathNode* next2 = new PathNode({ start->pos.x, start->pos.y + 2 });
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
    Shop shop;
    RectangleShape dragTower;
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

        dragTower.setSize(sf::Vector2f(GRID_SIZE, GRID_SIZE));
        dragTower.setFillColor(sf::Color::Red);
        dragTower.setPosition(26 * GRID_SIZE, 0);
    }

    void checkShopDrag(sf::Vector2f coord, bool clicked) {
        if (shop.isDragging && !clicked) {
            shop.isDragging = false;
            towers.push_back(new Tower(dragTower.getPosition()));
        }
        if (shop.shopTower.getGlobalBounds().contains(coord))
        {
            shop.isDragging = true;
        }
    }

    void update(Vector2f mousePos) {
        float dt = clock.getElapsedTime().asSeconds();
        if (dt - enemyMoveTimer > 0.5f) {
            for (auto& e : enemies) e->move();
            enemyMoveTimer = dt;
        }

        if (shop.isDragging)
        {
            dragTower.setPosition(mousePos.x - dragTower.getSize().x / 2,
                mousePos.y - dragTower.getSize().y / 2);
        }
    }

    void draw(sf::RenderWindow& window) {
        gameMap->draw(window);
        shop.draw(window);
        for (auto& t : towers) t->draw(window);
        for (auto& e : enemies) e->draw(window);
        if (shop.isDragging) {
            window.draw(dragTower);
        }
    }
    void checkTowerClick(sf::Vector2f mousePos) {
        for (auto& t : towers) t->isSelected = false; // deselect all first

        for (auto& t : towers) {
            if (t->shape.getGlobalBounds().contains(mousePos)) {
                t->isSelected = true;
                break;
            }
        }
    }

    void checkUpgradeClick(sf::Vector2f mousePos) {
        for (auto& t : towers) {
            if (t->isSelected) {
                UpgradeNode* options[2];
                int n = t->getAvailableUpgrades(options);

                for (int i = 0; i < n; i++) {
                    FloatRect btnRect(t->pos.x * GRID_SIZE, t->pos.y * GRID_SIZE + GRID_SIZE + i * 35, 100, 30);
                    if (btnRect.contains(mousePos)) {
                        t->upgrade(options[i]);
                    }
                }
            }
        }
    }


};

// ========== MAIN ==========
int main() {
    sf::RenderWindow window(sf::VideoMode(26 * GRID_SIZE + 2 * GRID_SIZE, 14 * GRID_SIZE), "Tower Defense - SFML Demo");
    window.setFramerateLimit(60);

    GameManager game;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            //Mouse press: begin drag if clicked on the shop item
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            game.checkShopDrag(mousePos, true);
            game.checkTowerClick(mousePos);
            game.checkUpgradeClick(mousePos);

            // Mouse release: drop tower
            if (event.type == sf::Event::MouseButtonReleased &&
                event.mouseButton.button == sf::Mouse::Left)
            {
                game.checkShopDrag({ 0,0 }, false);
            }
        }

        game.update(window.mapPixelToCoords(sf::Mouse::getPosition(window)));

        window.clear();
        game.draw(window);
        window.display();
    }

    return 0;
}
