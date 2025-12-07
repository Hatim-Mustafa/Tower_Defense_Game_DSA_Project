#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
using namespace std;
using namespace sf;

#define GRID_SIZE 30

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
    bool alive = true;
public:
    Enemy(int hp, int spd, PathNode* start, int x) : health(hp), speed(spd), current(start) {
        shape.setRadius(10);
        if (x == 1) {
			shape.setFillColor(sf::Color::Red);
		}
		else {
			shape.setFillColor(sf::Color::Yellow);
        }
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

    bool isAlive() { return alive; }

    Vector2f getPosition() { return shape.getPosition(); }

    void takeDamage(int dmg) {
        health -= dmg;
        if (health <= 0) {
            alive = false;
        }
    }

    float getRadius() { return shape.getRadius(); }

    void draw(sf::RenderWindow& window) { window.draw(shape); }
};


// ========== Tower ==========

class Projectile {
    float speed;
    Enemy* target;
    int damage;
    bool alive = true;
    CircleShape shape;

public:
    Projectile(const sf::Vector2f& startPos, Enemy* targetEnemy, int dmg, float spd)
        : target(targetEnemy), damage(dmg), speed(spd)
    {
        shape.setRadius(GRID_SIZE / 4);
        shape.setFillColor(sf::Color::Blue);
        shape.setOrigin(5.f, 5.f);
        shape.setPosition(startPos);
    }

    void update(float dt) {
        if (!alive || target == nullptr || !target->isAlive())
        {
            alive = false;
            return;
        }

        Vector2f pos = shape.getPosition();
        Vector2f targetPos = target->getPosition();

        Vector2f dir = targetPos - pos;
        float length = sqrt(dir.x * dir.x + dir.y * dir.y);

        if (length < 0.001f) {
            alive = false;
            return;
        }

        dir /= length;
        shape.move(dir * speed * dt);

        // collision check
        float distance = sqrt(
            pow(shape.getPosition().x - targetPos.x, 2) +
            pow(shape.getPosition().y - targetPos.y, 2)
        );

        if (distance < target->getRadius()) {
            target->takeDamage(damage);
            alive = false;
        }
    }

    bool isAlive() { return alive; }

    void draw(RenderWindow& window) {
        window.draw(shape);
    }
};

struct UpgradeNode {
    string name;
    int cost;
    int damageBoost;
    int rangeBoost;
    float fireRateBoost;
    float speedBoost;

    UpgradeNode* left;
    UpgradeNode* right;

    UpgradeNode(string n, int c, int dmg, int rng, float rate, float spd)
        : name(n), cost(c), damageBoost(dmg), rangeBoost(rng),
        fireRateBoost(rate), left(nullptr), right(nullptr), speedBoost(spd) {
    }
};

class Tower {
public:
    Vector2i pos;
    int damage;
    int range;
    float fireRate;
    int points;
    bool isSelected = false;
    float timer = 0.0f;
    vector<Projectile> projectiles;
    float speed;
    bool showUpgradeOptions = false;

    UpgradeNode* root;
    UpgradeNode* current;

    sf::CircleShape shape;
    sf::CircleShape rangeCircle;

    Tower(Vector2i coord = Vector2i(0, 0)) {
        pos = Vector2i(coord.x, coord.y);;
        damage = 10;
        range = 4 * GRID_SIZE;
        fireRate = 3.0f;
        points = 150;
        speed = 250.f;

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
        root = new UpgradeNode("Base Tower", 0, 10, 0, 0.0f, 0);

        // First tier
        root->left = new UpgradeNode("Wizard Tower", 50, 15, 50, 0.2f, 10.f);
        root->right = new UpgradeNode("Ice Tower", 50, 8, 20, 0.3f, 10.f);

        // Wizard Tower upgrades
        root->left->left = new UpgradeNode("Arcane Explosion", 100, 25, 20, 0.15f, 5.f);
        root->left->right = new UpgradeNode("Cannon Tower", 100, 30, 40, 0.2f, 5.f);

        // Cannon Tower upgrades
        root->left->right->left = new UpgradeNode("Heavy Shot", 150, 50, 30, 0.25f, 5.f);
        root->left->right->right = new UpgradeNode("Explosive Shell", 150, 45, 50, 0.2f, 5.f);

        // Lightning Chain upgrade remains for Wizard
        root->left->left->left = new UpgradeNode("Lightning Chain", 150, 20, 30, 0.1f, 5.f);

        // Ice Tower upgrades
        root->right->left = new UpgradeNode("Deep Freeze", 100, 5, 25, 0.1f, 5.f);
        root->right->right = new UpgradeNode("Shatter", 100, 10, 15, 0.1f, 5.f);
    }

    bool upgrade(UpgradeNode* targetNode) {
        if ((current->left == targetNode || current->right == targetNode) && points >= targetNode->cost) {
            damage += targetNode->damageBoost;
            range += targetNode->rangeBoost;
            fireRate -= targetNode->fireRateBoost;
            speed += targetNode->speedBoost;

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

            // Draw single upgrade button first
            sf::RectangleShape upgradeBtn(sf::Vector2f(100, 30));
            upgradeBtn.setPosition(pos.x * GRID_SIZE, pos.y * GRID_SIZE - 35);
            upgradeBtn.setFillColor(sf::Color(100, 100, 250));

            sf::Font font;
            if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
                std::cout << "Failed to load system font!" << std::endl;
            }

            sf::Text txt;
            txt.setFont(font);
            txt.setString("Upgrade");
            txt.setCharacterSize(12);
            txt.setFillColor(sf::Color::White);
            txt.setPosition(upgradeBtn.getPosition().x + 20, upgradeBtn.getPosition().y + 5);

            window.draw(upgradeBtn);
            window.draw(txt);

            // Only show upgrade options if the upgrade button was clicked
            if (showUpgradeOptions) {
                UpgradeNode* options[2];
                int n = getAvailableUpgrades(options);

                for (int i = 0; i < n; i++) {
                    sf::RectangleShape btn(sf::Vector2f(100, 40));
                    btn.setPosition(pos.x * GRID_SIZE, pos.y * GRID_SIZE + GRID_SIZE + i * 45);
                    btn.setFillColor(sf::Color(100, 100, 250));

                    sf::Text optionTxt;
                    optionTxt.setFont(font);
                    optionTxt.setString(options[i]->name + "\nCost: " + to_string(options[i]->cost));
                    optionTxt.setCharacterSize(12);
                    optionTxt.setFillColor(sf::Color::White);
                    optionTxt.setPosition(btn.getPosition().x + 5, btn.getPosition().y + 5);

                    window.draw(btn);
                    window.draw(optionTxt);
                }
            }
        }

        for (auto& p : projectiles) {
            p.draw(window);
        }
	}
    void update(float dt, vector<Enemy*>& enemies) {
        timer -= dt;
        Enemy* target;
        if (timer <= 0) {
            target = findTarget(enemies);
            if (target != nullptr) {
                projectiles.push_back(Projectile(shape.getPosition(), target, damage, speed));
                timer = fireRate;
            }
        }

        for (auto& p : projectiles) {
            if (p.isAlive())
                p.update(dt);
        }

        projectiles.erase(
            remove_if(projectiles.begin(), projectiles.end(), [](Projectile& p) { return !p.isAlive(); }),
            projectiles.end()
        );
    }

    Enemy* findTarget(const vector<Enemy*>& enemies) {
        for (const auto& e : enemies) {
            if (!e->isAlive()) continue;

            float dist = sqrt(
                pow(e->getPosition().x - shape.getPosition().x, 2) +
                pow(e->getPosition().y - shape.getPosition().y, 2)
            );

            if (dist <= range)
                return e;
        }
        return nullptr;
    }
};

class Shop {
public:
    sf::RectangleShape shopTower;
    bool isDragging;
    Shop() {
        shopTower.setSize(sf::Vector2f(GRID_SIZE * 3, GRID_SIZE * 3));
        shopTower.setFillColor(sf::Color::Red);
        shopTower.setPosition(39 * GRID_SIZE, 0);  // shop location
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
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},

            {1, 1, 1, 2, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 2, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 2, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},

            {0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
            {0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
            {0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},

            {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1},

            {1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 2, 2, 2, 1, 1, 1},
            {1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 2, 2, 2, 1, 1, 1},
            {1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 2, 2, 2, 1, 1, 1},

            {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},

            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1},

            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1},

            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1}
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
        if (gameMap->isPath(start->pos.x, start->pos.y - 3)) {
            PathNode* next = new PathNode({ start->pos.x, start->pos.y - 1 });
            start->front = next;
            PathNode* next2 = new PathNode({ start->pos.x, start->pos.y - 2 });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x, start->pos.y - 3 });
            next2->front = next3;
            buildPathNetwork(next3, movement, gameMap);
        }
        if (gameMap->isPath(start->pos.x - 3, start->pos.y)) {
            PathNode* next = new PathNode({ start->pos.x - 1, start->pos.y });
            start->left = next;
            PathNode* next2 = new PathNode({ start->pos.x - 2, start->pos.y });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x - 3, start->pos.y });
            next2->front = next3;
            buildPathNetwork(next3, "LEFT", gameMap);
        }
        if (gameMap->isPath(start->pos.x + 3, start->pos.y) &&
            (start->pos.x != 4 || start->pos.y != 10) &&
            (start->pos.x != 13 || start->pos.y != 16)) {
            PathNode* next = new PathNode({ start->pos.x + 1, start->pos.y });
            start->right = next;
            PathNode* next2 = new PathNode({ start->pos.x + 2, start->pos.y });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x + 3, start->pos.y });
            next2->front = next3;
            buildPathNetwork(next3, "RIGHT", gameMap);
        }
    }
    if (movement == "LEFT") {
        if (gameMap->isPath(start->pos.x, start->pos.y - 3)) {
            PathNode* next = new PathNode({ start->pos.x, start->pos.y - 1 });
            start->front = next;
            PathNode* next2 = new PathNode({ start->pos.x, start->pos.y - 2 });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x, start->pos.y - 3 });
            next2->front = next3;
            buildPathNetwork(next3, "UP", gameMap);
        }
        if (gameMap->isPath(start->pos.x - 3, start->pos.y)) {
            PathNode* next = new PathNode({ start->pos.x - 1, start->pos.y });
            start->left = next;
            PathNode* next2 = new PathNode({ start->pos.x - 2, start->pos.y });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x - 3, start->pos.y });
            next2->front = next3;
            buildPathNetwork(next3, movement, gameMap);
        }
        if (gameMap->isPath(start->pos.x, start->pos.y + 3) &&
            (start->pos.x != 4 || start->pos.y != 10) &&
            (start->pos.x != 13 || start->pos.y != 16)) {
            PathNode* next = new PathNode({ start->pos.x, start->pos.y + 1 });
            start->right = next;
            PathNode* next2 = new PathNode({ start->pos.x, start->pos.y + 2 });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x, start->pos.y + 3 });
            next2->front = next3;
            buildPathNetwork(next3, "DOWN", gameMap);
        }
    }
    if (movement == "RIGHT") {
        if (gameMap->isPath(start->pos.x, start->pos.y - 3)) {
            PathNode* next = new PathNode({ start->pos.x, start->pos.y - 1 });
            start->front = next;
            PathNode* next2 = new PathNode({ start->pos.x, start->pos.y - 2 });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x, start->pos.y - 3 });
            next2->front = next3;
            buildPathNetwork(next3, "UP", gameMap);
        }
        if (gameMap->isPath(start->pos.x, start->pos.y + 3)) {
            PathNode* next = new PathNode({ start->pos.x, start->pos.y + 1 });
            start->right = next;
            PathNode* next2 = new PathNode({ start->pos.x, start->pos.y + 2 });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x, start->pos.y + 3 });
            next2->front = next3;
            buildPathNetwork(next3, "DOWN", gameMap);
        }
        if (gameMap->isPath(start->pos.x + 3, start->pos.y)) {
            PathNode* next = new PathNode({ start->pos.x + 1, start->pos.y });
            start->right = next;
            PathNode* next2 = new PathNode({ start->pos.x + 2, start->pos.y });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x + 3, start->pos.y });
            next2->front = next3;
            buildPathNetwork(next3, movement, gameMap);
        }
    }
    if (movement == "DOWN") {
        if (gameMap->isPath(start->pos.x, start->pos.y + 3)) {
            PathNode* next = new PathNode({ start->pos.x, start->pos.y + 1 });
            start->right = next;
            PathNode* next2 = new PathNode({ start->pos.x, start->pos.y + 2 });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x, start->pos.y + 3 });
            next2->front = next3;
            buildPathNetwork(next3, movement, gameMap);
        }
        if (gameMap->isPath(start->pos.x - 3, start->pos.y)) {
            PathNode* next = new PathNode({ start->pos.x - 1, start->pos.y });
            start->left = next;
            PathNode* next2 = new PathNode({ start->pos.x - 2, start->pos.y });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x - 3, start->pos.y });
            next2->front = next3;
            buildPathNetwork(next3, "LEFT", gameMap);
        }
        if (gameMap->isPath(start->pos.x + 3, start->pos.y)) {
            PathNode* next = new PathNode({ start->pos.x + 1, start->pos.y });
            start->right = next;
            PathNode* next2 = new PathNode({ start->pos.x + 2, start->pos.y });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x + 3, start->pos.y });
            next2->front = next3;
            buildPathNetwork(next3, "RIGHT", gameMap);
        }
    }
}

// ========== Game Manager ==========
class GameManager {
    Map* gameMap;
    Shop shop;
    Tower dragTower;
    vector<Tower*> towers;
    vector<Enemy*> enemies;
    PathNode* pathHead;
    sf::Clock clock;
    float enemyMoveTimer;
public:
    GameManager() {
        gameMap = new Map(27, 39);
        enemyMoveTimer = 0;

        // Build the path network
        PathNode* start = new PathNode({ 31,26 });
        PathNode* second = new PathNode({ 31,25 });
        start->front = second;
        buildPathNetwork(second, "UP", gameMap);

        pathHead = start;

        // Spawn one enemy
        enemies.push_back(new Enemy(20, 1, pathHead, 1));
        enemies.push_back(new Enemy(10, 1, pathHead, 2));
    }

    void checkShopDrag(sf::Vector2f coord, bool clicked) {
        if (shop.isDragging && !clicked) {
            shop.isDragging = false;
            Vector2f coord = dragTower.shape.getPosition();
            Vector2i pos = Vector2i(round(coord.x / GRID_SIZE), round(coord.y / GRID_SIZE));
            if (gameMap->isBuildable(pos.x, pos.y)) {
                for (auto& t : towers) {
                    if (t->pos == pos) {
                        return; // Can't build on another tower
                    }
                }
                towers.push_back(new Tower(pos));
            }
        }
        if (shop.shopTower.getGlobalBounds().contains(coord))
        {
            shop.isDragging = true;
        }
    }

    void checkTowerClick(sf::Vector2f mousePos) {
        bool clickedOnTower = false;
        bool clickedOnUpgradeUI = false;

        // Check if clicking on any tower
        for (auto& t : towers) {
            if (t->shape.getGlobalBounds().contains(mousePos)) {
                clickedOnTower = true;
            }

            // Check if clicking on upgrade button
            if (t->isSelected) {
                FloatRect upgradeBtnRect(t->pos.x * GRID_SIZE, t->pos.y * GRID_SIZE - 35, 100, 30);
                if (upgradeBtnRect.contains(mousePos)) {
                    clickedOnUpgradeUI = true;
                }

                // Check if clicking on upgrade options
                if (t->showUpgradeOptions) {
                    UpgradeNode* options[2];
                    int n = t->getAvailableUpgrades(options);
                    for (int i = 0; i < n; i++) {
                        FloatRect btnRect(t->pos.x * GRID_SIZE, t->pos.y * GRID_SIZE + GRID_SIZE + i * 45, 100, 40);
                        if (btnRect.contains(mousePos)) {
                            clickedOnUpgradeUI = true;
                        }
                    }
                }
            }
        }

        if (!clickedOnTower && !clickedOnUpgradeUI) {
            for (auto& t : towers) {
                t->isSelected = false;
                t->showUpgradeOptions = false;
            }
        }
        // If clicking on a tower, select it
        else if (clickedOnTower && !clickedOnUpgradeUI) {
            // First deselect all
            for (auto& t : towers) {
                t->isSelected = false;
                t->showUpgradeOptions = false;
            }

            // Then select the clicked tower
            for (auto& t : towers) {
                if (t->shape.getGlobalBounds().contains(mousePos)) {
                    t->isSelected = true;
                    break;
                }
            }
        }
    }

    void checkUpgradeClick(sf::Vector2f mousePos) {
        for (auto& t : towers) {
            if (t->isSelected) {
                FloatRect upgradeBtnRect(t->pos.x * GRID_SIZE, t->pos.y * GRID_SIZE - 35, 100, 30);
                if (upgradeBtnRect.contains(mousePos)) {
                    t->showUpgradeOptions = !t->showUpgradeOptions;  // Toggle upgrade options
                    return;  // Return early so we don't process tower click
                }
                if (t->showUpgradeOptions) {
                    UpgradeNode* options[2];
                    int n = t->getAvailableUpgrades(options);

                    for (int i = 0; i < n; i++) {
                        FloatRect btnRect(t->pos.x * GRID_SIZE, t->pos.y * GRID_SIZE + GRID_SIZE + i * 45, 100, 40);
                        if (btnRect.contains(mousePos)) {
                            t->upgrade(options[i]);
                            t->showUpgradeOptions = false;  // Hide options after upgrading
                        }
                    }
                }
            }
        }
    }

    void update(Vector2f mousePos, float dt) {
        float dtt = clock.getElapsedTime().asSeconds();

        enemies.erase(
            remove_if(enemies.begin(), enemies.end(), [](Enemy*& e) { return !e->isAlive(); }),
            enemies.end()
        );

        if (dtt - enemyMoveTimer > 0.5f) {
            for (auto& e : enemies) e->move();
            enemyMoveTimer = dtt;
        }

        for (auto& t : towers)
            t->update(dt, enemies);

        if (shop.isDragging)
        {
            dragTower.shape.setPosition(mousePos.x - dragTower.shape.getRadius(),
                mousePos.y - dragTower.shape.getRadius());
        }
    }

    void draw(sf::RenderWindow& window) {
        gameMap->draw(window);
        shop.draw(window);
        for (auto& t : towers) t->draw(window);
        for (auto& e : enemies) e->draw(window);
        if (shop.isDragging) {
            dragTower.draw(window);
        }
    }
};

// ========== MAIN ==========
int main() {
    sf::RenderWindow window(sf::VideoMode(39 * GRID_SIZE + 3 * GRID_SIZE, 27 * GRID_SIZE), "Tower Defense - SFML Demo");
    window.setFramerateLimit(60);

    GameManager game;
    Clock clock;

    while (window.isOpen()) {
        sf::Event event;
        float dt = clock.restart().asSeconds();
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            //Mouse press: begin drag if clicked on the shop item
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                game.checkUpgradeClick(mousePos);
                game.checkTowerClick(mousePos);
                game.checkShopDrag(mousePos, true);
            }

            // Mouse release: drop tower
            if (event.type == sf::Event::MouseButtonReleased &&
                event.mouseButton.button == sf::Mouse::Left)
            {
                game.checkShopDrag({ 0,0 }, false);
            }
        }

        game.update(window.mapPixelToCoords(sf::Mouse::getPosition(window)), dt);

        window.clear();
        game.draw(window);
        window.display();
    }

    return 0;
}
