#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <stack>
#include <queue>

using namespace std;
using namespace sf;

#define GRID_SIZE 20
#define PIXEL 4

class Player; // Forward declaration
class GameManager; // Forward declaration
class Enemy; // Forward declaration

// ========== Coordinate ==========
struct Coordinate
{
    int x, y;
};

// ========== Path Node (Linked List) ==========
struct PathNode
{
    Coordinate pos;
    PathNode* front;
    PathNode* left;
    int cost;
    PathNode* right;
    PathNode(Coordinate p) : pos(p), front(nullptr), left(nullptr), right(nullptr), cost(1) {}
};

class PathCost
{
public:
    PathNode* node;
    int cost;
    PathNode* parent;

    PathCost(PathNode* n, int c, PathNode* p)
    {
        node = n;
        cost = c;
        parent = p;
    }
};

class Player
{
    int money;
    int health;
    Font font;
    Text healthText;
    ConvexShape heartShape;
    CircleShape coinIcon;
    Text moneyText;

public:
    Player() : money(400), health(200)
    {
        // Load Font
        if (!font.loadFromFile("C:/Users/Dell/OneDrive/Desktop/Hatim/DSA/DS_Proj/Clash_Regular.otf")) {
            std::cout << "Failed to load font\n";
        }


        // Setup Health Text
        healthText.setFont(font);
        healthText.setCharacterSize(30);
        healthText.setFillColor(Color::White);
        healthText.setPosition(70, 15); // Offset to right of heart

        // Setup Heart Shape
        heartShape.setPointCount(6);
        heartShape.setPoint(0, Vector2f(20, 45)); // Bottom tip
        heartShape.setPoint(1, Vector2f(0, 20));  // Left middle
        heartShape.setPoint(2, Vector2f(10, 0));  // Left top hump
        heartShape.setPoint(3, Vector2f(20, 10)); // Top V dip
        heartShape.setPoint(4, Vector2f(30, 0));  // Right top hump
        heartShape.setPoint(5, Vector2f(40, 20)); // Right middle

        heartShape.setFillColor(Color::Red);
        heartShape.setPosition(20, 10);

        coinIcon.setRadius(15);
        coinIcon.setFillColor(Color(255, 215, 0));
        coinIcon.setOutlineThickness(2);
        coinIcon.setOutlineColor(Color(184, 134, 11));
        coinIcon.setPosition(150, 18);

        moneyText.setFont(font);
        moneyText.setCharacterSize(30);
        moneyText.setFillColor(Color::White);
        moneyText.setPosition(190, 15);
    }

    void updateHealth(Enemy& E);

    void increaseMoney(int g)
    {
        if (g < 5) {
            money = money + (100 * g);
        }
        else {
            money += g;
        }
    }

    void decreaseMoney(int amount)
    {
        money -= amount;
    }

    int getHealth() { return health; }
    int getMoney() { return money; }

    void draw(RenderWindow& window)
    {
        healthText.setString(to_string(health));
        window.draw(heartShape);
        window.draw(healthText);
        window.draw(coinIcon);
        moneyText.setString(to_string(money));
        window.draw(moneyText);
    }

    void setHealth(int h) {
        health = h;
    }
};

// ========== Enemy ==========
class Enemy
{
    int health, speed;
    int oghealth;
    PathNode* current;
    CircleShape shape;
    bool alive = true;
    float moveProgress = 0.0f;
    float moveInterval;
    bool useSmart;
    stack<PathNode*> smartPath;

public:
    Enemy(int hp, PathNode* start, Color c, float moveInterval, bool smart = false) : health(hp), current(start), oghealth(hp), moveInterval(moveInterval), useSmart(smart)
    {
        shape.setRadius(8);
        shape.setFillColor(c);
        shape.setPosition(start->pos.x * GRID_SIZE, start->pos.y * GRID_SIZE);
    }

    void setSmartPath(stack<PathNode*> path)
    {
        smartPath = path;
    }

    void move()
    {
        if (useSmart && !smartPath.empty())
        {
            // Use smart path
            current = smartPath.top();
            smartPath.pop();
            shape.setPosition(current->pos.x * GRID_SIZE, current->pos.y * GRID_SIZE);
        }
        else if (!useSmart)
        {
            // Use normal path (existing logic)
            if (current && current->front)
            {
                current = current->front;
                shape.setPosition(current->pos.x * GRID_SIZE, current->pos.y * GRID_SIZE);
            }
            else if (current && current->left)
            {
                current = current->left;
                shape.setPosition(current->pos.x * GRID_SIZE, current->pos.y * GRID_SIZE);
            }
            else if (current && current->right)
            {
                current = current->right;
                shape.setPosition(current->pos.x * GRID_SIZE, current->pos.y * GRID_SIZE);
            }
        }
    }

    void update(float dt)
    {
        if (!alive)
            return;
        moveProgress += dt;
        if (moveProgress >= moveInterval /* or based on speed */)
        {
            moveProgress -= moveInterval;
            move(); // Advance to next PathNode
        }
    }

    Vector2f getPosition() { return shape.getPosition(); }

    void takeDamage(int dmg, Player& player)
    {
        health -= dmg;
        if (health <= 0)
        {
            alive = false;
            player.increaseMoney(oghealth);
        }
    }

    bool reachedEndOfPath() const
    {
        return !(current && (current->front || current->left || current->right));
    }

    bool isAlive() { return alive; }
    int gethealth() { return health; }
    float getRadius() { return shape.getRadius(); }
    void draw(RenderWindow& window) { window.draw(shape); }
    PathNode* getCurrentNode() { return current; }
};

class RedEnemy : public Enemy
{
public:
    RedEnemy(PathNode* start, bool smart = false)
        : Enemy(10, start, Color::Red, 0.15f, smart) {
    }
};

class BlueEnemy : public Enemy
{
public:
    BlueEnemy(PathNode* start, bool smart = false)
        : Enemy(25, start, Color::Blue, 0.2f, smart) {
    }
};

class GreenEnemy : public Enemy
{
public:
    GreenEnemy(PathNode* start, bool smart = false)
        : Enemy(40, start, Color::Green, 0.2f, smart) {
    }
};


void Player::updateHealth(Enemy& E)
{
    health = health - E.gethealth();
}
// ========== Tower ==========

class Projectile
{
    float speed;
    Enemy* target;
    int damage;
    bool alive = true;
    CircleShape shape;

public:
    Projectile(const  Vector2f& startPos, Enemy* targetEnemy, int dmg, float spd)
        : target(targetEnemy), damage(dmg), speed(spd)
    {
        shape.setRadius(GRID_SIZE / 4);
        shape.setFillColor(Color::Blue);
        shape.setOrigin(5.f, 5.f);
        shape.setPosition(startPos);
    }

    void update(float dt, Player& player)
    {
        if (!alive || target == nullptr || !target->isAlive())
        {
            alive = false;
            return;
        }

        Vector2f pos = shape.getPosition();
        Vector2f targetPos = target->getPosition();

        Vector2f dir = targetPos - pos;
        float length = sqrt(dir.x * dir.x + dir.y * dir.y);

        if (length < 0.001f)
        {
            alive = false;
            return;
        }

        dir /= length;
        shape.move(dir * speed * dt);

        // collision check
        float distance = sqrt(
            pow(shape.getPosition().x - targetPos.x, 2) +
            pow(shape.getPosition().y - targetPos.y, 2));

        if (distance < target->getRadius())
        {
            target->takeDamage(damage, player);
            alive = false;
        }
    }

    bool isAlive() { return alive; }

    void draw(RenderWindow& window)
    {
        window.draw(shape);
    }
};

struct UpgradeNode
{
    string name;
    int UpgradeCost;
    int damageBoost;
    int rangeBoost;
    float fireRateBoost;
    float speedBoost;

    UpgradeNode* left;
    UpgradeNode* right;

    UpgradeNode(string n, int c, int dmg, int rng, float rate, float spd)
        : name(n), UpgradeCost(c), damageBoost(dmg), rangeBoost(rng),
        fireRateBoost(rate), left(nullptr), right(nullptr), speedBoost(spd)
    {
    }
};

class Tower
{
public:
    Vector2i pos;
    int damage;
    int range;
    float fireRate;
    int TowerCost;
    bool isSelected = false;
    float timer = 0.0f;
    vector<Projectile> projectiles;
    float speed;
    bool showUpgradeOptions = false;
    bool showModeOptions = false;
    string targetingMode = "First Enemy";

    UpgradeNode* root;
    UpgradeNode* current;

    CircleShape shape;
    CircleShape rangeCircle;

    Tower(Vector2i coord = Vector2i(0, 0))
    {
        pos = Vector2i(coord.x, coord.y);
        ;
        damage = 10;
        range = 6 * GRID_SIZE;
        fireRate = 2.0f;
        TowerCost = 150;
        speed = 250.f;

        shape.setRadius(GRID_SIZE);
        shape.setFillColor(Color::Blue);
        shape.setPosition(pos.x * GRID_SIZE, pos.y * GRID_SIZE);
        shape.setOrigin(GRID_SIZE, GRID_SIZE);
        rangeCircle.setRadius(range);
        rangeCircle.setOrigin(range, range);
        rangeCircle.setFillColor(Color(0, 0, 255, 50));
        rangeCircle.setPosition(pos.x * GRID_SIZE + GRID_SIZE / 2, pos.y * GRID_SIZE + GRID_SIZE / 2);

        buildUpgradeTree();
        current = root;
    }

    void buildUpgradeTree()
    {
        root = new UpgradeNode("Base Tower", 0, 10, 0, 0.0f, 0);

        // First tier
        root->left = new UpgradeNode("Wizard Tower", 50, 12, 30, 0.5f, 10.f);
        root->right = new UpgradeNode("Ice Tower", 40, 8, 25, 0.4f, 10.f);

        // Wizard Tower upgrades
        root->left->left = new UpgradeNode("Arcane Explosion", 100, 15, 20, 0.3f, 10.f);
        root->left->right = new UpgradeNode("Cannon Tower", 100, 25, 40, 0.0f, 7.f);

        // Cannon Tower upgrades
        root->left->right->left = new UpgradeNode("Heavy Shot", 150, 20, 15, 0.25f, 5.f);
        root->left->right->right = new UpgradeNode("Explosive Shell", 200, 25, 20, 0.2f, 5.f);

        // Lightning Chain upgrade remains for Wizard
        root->left->left->left = new UpgradeNode("Lightning Chain", 150, 10, 15, 0.2f, 5.f);

        // Ice Tower upgrades
        root->right->left = new UpgradeNode("Deep Freeze", 120, 15, 25, 0.3f, 5.f);
        root->right->right = new UpgradeNode("Shatter", 100, 10, 15, 0.5f, 5.f);
    }

    bool upgrade(UpgradeNode* targetNode, Player player);

    int getTowerCost() { return TowerCost; }

    int getAvailableUpgrades(UpgradeNode* options[2])
    {
        int count = 0;
        if (current->left)
            options[count++] = current->left;
        if (current->right)
            options[count++] = current->right;
        return count;
    }

    bool isClicked(Vector2f mousePos)
    {
        return shape.getGlobalBounds().contains(mousePos);
    }

    void draw(RenderWindow& window) {
        window.draw(shape);

        if (isSelected) {
            window.draw(rangeCircle); // draw tower range

            // Draw Upgrade button
            RectangleShape upgradeBtn(Vector2f(100, 30));
            upgradeBtn.setPosition(pos.x * GRID_SIZE, pos.y * GRID_SIZE - 70); // Moved up
            upgradeBtn.setFillColor(Color(100, 100, 250));

            // Draw Mode button
            RectangleShape modeBtn(Vector2f(100, 30));
            modeBtn.setPosition(pos.x * GRID_SIZE, pos.y * GRID_SIZE - 35); // Below upgrade button
            modeBtn.setFillColor(Color(250, 100, 100)); // Different color for mode button

            sf::Font font;
            if (!font.loadFromFile("C:/Users/Dell/OneDrive/Desktop/Hatim/DSA/DS_Proj/Clash_Regular.otf")) {
                std::cout << "Failed to load font\n";
            }


            // Upgrade button text
            Text upgradeTxt;
            upgradeTxt.setFont(font);
            upgradeTxt.setString("Upgrade");
            upgradeTxt.setCharacterSize(12);
            upgradeTxt.setFillColor(Color::White);
            upgradeTxt.setPosition(upgradeBtn.getPosition().x + 20, upgradeBtn.getPosition().y + 5);

            // Mode button text
            Text modeTxt;
            modeTxt.setFont(font);
            modeTxt.setString("Mode: " + targetingMode);
            modeTxt.setCharacterSize(12);
            modeTxt.setFillColor(Color::White);
            modeTxt.setPosition(modeBtn.getPosition().x + 10, modeBtn.getPosition().y + 5);

            window.draw(upgradeBtn);
            window.draw(upgradeTxt);
            window.draw(modeBtn);
            window.draw(modeTxt);

            // Show upgrade options if upgrade button was clicked
            if (showUpgradeOptions) {
                UpgradeNode* options[2];
                int n = getAvailableUpgrades(options);

                for (int i = 0; i < n; i++) {
                    RectangleShape btn(Vector2f(100, 40));
                    btn.setPosition(pos.x * GRID_SIZE, pos.y * GRID_SIZE + GRID_SIZE + i * 45);
                    btn.setFillColor(Color(100, 100, 250));

                    Text optionTxt;
                    optionTxt.setFont(font);
                    optionTxt.setString(options[i]->name + "\nCost: " + to_string(options[i]->UpgradeCost));
                    optionTxt.setCharacterSize(12);
                    optionTxt.setFillColor(Color::White);
                    optionTxt.setPosition(btn.getPosition().x + 5, btn.getPosition().y + 5);

                    window.draw(btn);
                    window.draw(optionTxt);
                }
            }

            // Show mode options if mode button was clicked
            if (showModeOptions) {
                // First enemy option
                RectangleShape firstBtn(Vector2f(150, 30));
                firstBtn.setPosition(pos.x * GRID_SIZE - 25, pos.y * GRID_SIZE + GRID_SIZE);
                firstBtn.setFillColor(Color(100, 200, 100));

                Text firstTxt;
                firstTxt.setFont(font);
                firstTxt.setString("First Enemy");
                firstTxt.setCharacterSize(12);
                firstTxt.setFillColor(Color::White);
                firstTxt.setPosition(firstBtn.getPosition().x + 10, firstBtn.getPosition().y + 5);

                // Strongest enemy option
                RectangleShape strongBtn(Vector2f(150, 30));
                strongBtn.setPosition(pos.x * GRID_SIZE - 25, pos.y * GRID_SIZE + GRID_SIZE + 35);
                strongBtn.setFillColor(Color(200, 100, 100));

                Text strongTxt;
                strongTxt.setFont(font);
                strongTxt.setString("Strongest Enemy");
                strongTxt.setCharacterSize(12);
                strongTxt.setFillColor(Color::White);
                strongTxt.setPosition(strongBtn.getPosition().x + 10, strongBtn.getPosition().y + 5);

                // Fastest enemy option
                RectangleShape fastBtn(Vector2f(150, 30));
                fastBtn.setPosition(pos.x * GRID_SIZE - 25, pos.y * GRID_SIZE + GRID_SIZE + 70);
                fastBtn.setFillColor(Color(100, 100, 200));

                Text fastTxt;
                fastTxt.setFont(font);
                fastTxt.setString("Fastest Enemy");
                fastTxt.setCharacterSize(12);
                fastTxt.setFillColor(Color::White);
                fastTxt.setPosition(fastBtn.getPosition().x + 10, fastBtn.getPosition().y + 5);

                window.draw(firstBtn);
                window.draw(firstTxt);
                window.draw(strongBtn);
                window.draw(strongTxt);
                window.draw(fastBtn);
                window.draw(fastTxt);
            }
        }

        for (auto& p : projectiles) {
            p.draw(window);
        }
    }

    void update(float dt, vector<Enemy*>& enemies, Player& player)
    {
        timer -= dt;
        Enemy* target;
        if (timer <= 0)
        {
            target = findTarget(enemies);
            if (target != nullptr)
            {
                projectiles.push_back(Projectile(shape.getPosition(), target, damage, speed));
                timer = fireRate;
            }
        }

        for (auto& p : projectiles)
        {
            if (p.isAlive())
                p.update(dt, player);
        }

        projectiles.erase(
            remove_if(projectiles.begin(), projectiles.end(), [](Projectile& p)
                { return !p.isAlive(); }),
            projectiles.end());
    }

    Enemy* findTarget(const vector<Enemy*>& enemies)
    {
        Enemy* enemy = nullptr;
        if (targetingMode == "First Enemy") {
            for (const auto& e : enemies)
            {
                if (!e->isAlive())
                    continue;

                float dist = sqrt(
                    pow(e->getPosition().x - shape.getPosition().x, 2) +
                    pow(e->getPosition().y - shape.getPosition().y, 2));

                if (dist <= range) {
                    enemy = e;
                    break;
                }
            }
        }
        else if (targetingMode == "Strongest Enemy") {
            int h = 0;
            for (const auto& e : enemies)
            {
                if (!e->isAlive())
                    continue;

                float dist = sqrt(
                    pow(e->getPosition().x - shape.getPosition().x, 2) +
                    pow(e->getPosition().y - shape.getPosition().y, 2));

                if (dist <= range) {
                    if (e->gethealth() > h) {
                        enemy = e;
                        h = e->gethealth();
                    }
                }
            }
        }
        else if (targetingMode == "Fastest Enemy") {
            int f = 0;
            for (const auto& e : enemies)
            {
                if (!e->isAlive())
                    continue;

                float dist = sqrt(
                    pow(e->getPosition().x - shape.getPosition().x, 2) +
                    pow(e->getPosition().y - shape.getPosition().y, 2));

                if (dist <= range) {
                    if (e->gethealth() > f) {
                        enemy = e;
                        f = e->gethealth();
                    }
                }
            }
        }
        return enemy;
    }

    // Function to handle mode button click
    void toggleModeOptions() {
        showModeOptions = !showModeOptions;
        showUpgradeOptions = false; // Close upgrade options if open
    }

    // Function to set targeting mode
    void setTargetingMode(string mode) {
        targetingMode = mode;
        showModeOptions = false; // Close mode options after selection
    }

    // Function to check which mode button was clicked
    string checkModeClick(Vector2f mousePos) {
        if (!showModeOptions) return "";

        FloatRect firstBtnRect(pos.x * GRID_SIZE - 25, pos.y * GRID_SIZE + GRID_SIZE, 150, 30);
        FloatRect strongBtnRect(pos.x * GRID_SIZE - 25, pos.y * GRID_SIZE + GRID_SIZE + 35, 150, 30);
        FloatRect fastBtnRect(pos.x * GRID_SIZE - 25, pos.y * GRID_SIZE + GRID_SIZE + 70, 150, 30);

        if (firstBtnRect.contains(mousePos)) return "First Enemy";
        if (strongBtnRect.contains(mousePos)) return "Strongest Enemy";
        if (fastBtnRect.contains(mousePos)) return "Fastest Enemy";

        return "";
    }
};

class Shop
{
public:
    RectangleShape shopTower;
    bool isDragging;
    Text waveText;
    Shop()
    {
        shopTower.setSize(Vector2f(GRID_SIZE * PIXEL, GRID_SIZE * PIXEL));
        shopTower.setFillColor(Color::Red);
        shopTower.setPosition(52 * GRID_SIZE, 35); // shop location
        isDragging = false;

        static Font font;
        if (font.loadFromFile("C:/Users/Dell/OneDrive/Desktop/Hatim/DSA/DS_Proj/Clash_Regular.otf")) {

            waveText.setFont(font);
            waveText.setString("SHOP");
            waveText.setCharacterSize(24);
            waveText.setFillColor(Color::White);
            waveText.setStyle(Text::Bold);
            float shopX = 26 * GRID_SIZE;  // Shop's left edge
            float textWidth = waveText.getLocalBounds().width;

            waveText.setPosition(shopX - textWidth + 590, 5);

        }
    }
    void draw(RenderWindow& window) {
        window.draw(shopTower); window.draw(waveText);
    }
};

// ========== Map ==========
class Map {
    int rows, cols;
    vector<vector<int>> grid;
    vector< RectangleShape> tiles;
    sf::Texture mapTexture;
    sf::Sprite mapSprite;
    bool mapSpriteLoaded = false;
public:
    Map(int r, int c) : rows(r), cols(c) {
        grid.assign(rows, vector<int>(cols, 0));
        // Manually mark the path based on your image
        grid = {
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},

            {1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},

            {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
            {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
            {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
            {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},

            {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},

            {1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 2, 2, 2, 2, 1, 1, 1, 1},
            {1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 2, 2, 2, 2, 1, 1, 1, 1},
            {1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 2, 2, 2, 2, 1, 1, 1, 1},
            {1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 2, 2, 2, 2, 1, 1, 1, 1},

            {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},

            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},

            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},

            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
        };

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                RectangleShape rect(Vector2f(GRID_SIZE, GRID_SIZE));
                rect.setPosition(j * GRID_SIZE, i * GRID_SIZE);
                if (grid[i][j] == 0) rect.setFillColor(Color(150, 150, 150)); // path
                else if (grid[i][j] == 2) rect.setFillColor(Color(0, 0, 0)); // special path
                else rect.setFillColor(Color(0, 200, 0)); // grass
                tiles.push_back(rect);
            }
        }

        if (mapTexture.loadFromFile("C:/Users/Dell/OneDrive/Desktop/Hatim/DSA/DS_Proj/DSAmap-1-Recovered.png")) {
            mapSprite.setTexture(mapTexture);
            mapSprite.setPosition(0, 0); // Make sure it lines up with tiles!
            mapSpriteLoaded = true;
        }
    }

    void draw(RenderWindow& window) {
        if (mapSpriteLoaded) {
            window.draw(mapSprite);
        }
        //for (auto& t : tiles) window.draw(t);
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
    bool pathFound = false;
    if (movement == "UP") {
        if (gameMap->isPath(start->pos.x, start->pos.y - PIXEL)) {
            pathFound = true;
            PathNode* next = new PathNode({ start->pos.x, start->pos.y - 1 });
            start->front = next;
            PathNode* next2 = new PathNode({ start->pos.x, start->pos.y - 2 });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x, start->pos.y - 3 });
            next2->front = next3;
            PathNode* next4 = new PathNode({ start->pos.x, start->pos.y - PIXEL });
            next3->front = next4;
            buildPathNetwork(next4, movement, gameMap);
        }
        if (gameMap->isPath(start->pos.x - PIXEL, start->pos.y)) {
            pathFound = true;
            PathNode* next = new PathNode({ start->pos.x - 1, start->pos.y });
            start->left = next;
            PathNode* next2 = new PathNode({ start->pos.x - 2, start->pos.y });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x - 3, start->pos.y });
            next2->front = next3;
            PathNode* next4 = new PathNode({ start->pos.x - PIXEL, start->pos.y });
            next3->front = next4;
            buildPathNetwork(next4, "LEFT", gameMap);
        }
        if (gameMap->isPath(start->pos.x + PIXEL, start->pos.y) &&
            (start->pos.x != 6 || start->pos.y != 14) &&
            (start->pos.x != 18 || start->pos.y != 22)) {
            pathFound = true;
            PathNode* next = new PathNode({ start->pos.x + 1, start->pos.y });
            start->right = next;
            PathNode* next2 = new PathNode({ start->pos.x + 2, start->pos.y });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x + 3, start->pos.y });
            next2->front = next3;
            PathNode* next4 = new PathNode({ start->pos.x + PIXEL, start->pos.y });
            next3->front = next4;
            buildPathNetwork(next4, "RIGHT", gameMap);
        }
    }
    if (movement == "LEFT") {
        if (gameMap->isPath(start->pos.x, start->pos.y - PIXEL)) {
            pathFound = true;
            PathNode* next = new PathNode({ start->pos.x, start->pos.y - 1 });
            start->front = next;
            PathNode* next2 = new PathNode({ start->pos.x, start->pos.y - 2 });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x, start->pos.y - 3 });
            next2->front = next3;
            PathNode* next4 = new PathNode({ start->pos.x, start->pos.y - PIXEL });
            next3->front = next4;
            buildPathNetwork(next4, "UP", gameMap);
        }
        if (gameMap->isPath(start->pos.x - PIXEL, start->pos.y)) {
            pathFound = true;
            PathNode* next = new PathNode({ start->pos.x - 1, start->pos.y });
            start->left = next;
            PathNode* next2 = new PathNode({ start->pos.x - 2, start->pos.y });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x - 3, start->pos.y });
            next2->front = next3;
            PathNode* next4 = new PathNode({ start->pos.x - PIXEL, start->pos.y });
            next3->front = next4;
            buildPathNetwork(next4, movement, gameMap);
        }
        if (gameMap->isPath(start->pos.x, start->pos.y + PIXEL) &&
            (start->pos.x != 6 || start->pos.y != 14) &&
            (start->pos.x != 18 || start->pos.y != 22)) {
            pathFound = true;
            PathNode* next = new PathNode({ start->pos.x, start->pos.y + 1 });
            start->right = next;
            PathNode* next2 = new PathNode({ start->pos.x, start->pos.y + 2 });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x, start->pos.y + 3 });
            next2->front = next3;
            PathNode* next4 = new PathNode({ start->pos.x, start->pos.y + PIXEL });
            next3->front = next4;
            buildPathNetwork(next4, "DOWN", gameMap);
        }
    }
    if (movement == "RIGHT") {
        if (gameMap->isPath(start->pos.x, start->pos.y - PIXEL)) {
            pathFound = true;
            PathNode* next = new PathNode({ start->pos.x, start->pos.y - 1 });
            start->front = next;
            PathNode* next2 = new PathNode({ start->pos.x, start->pos.y - 2 });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x, start->pos.y - 3 });
            next2->front = next3;
            PathNode* next4 = new PathNode({ start->pos.x, start->pos.y - PIXEL });
            next3->front = next4;
            buildPathNetwork(next4, "UP", gameMap);
        }
        if (gameMap->isPath(start->pos.x, start->pos.y + PIXEL)) {
            pathFound = true;
            PathNode* next = new PathNode({ start->pos.x, start->pos.y + 1 });
            start->right = next;
            PathNode* next2 = new PathNode({ start->pos.x, start->pos.y + 2 });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x, start->pos.y + 3 });
            next2->front = next3;
            PathNode* next4 = new PathNode({ start->pos.x, start->pos.y + PIXEL });
            next3->front = next4;
            buildPathNetwork(next4, "DOWN", gameMap);
        }
        if (gameMap->isPath(start->pos.x + PIXEL, start->pos.y)) {
            pathFound = true;
            PathNode* next = new PathNode({ start->pos.x + 1, start->pos.y });
            start->right = next;
            PathNode* next2 = new PathNode({ start->pos.x + 2, start->pos.y });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x + 3, start->pos.y });
            next2->front = next3;
            PathNode* next4 = new PathNode({ start->pos.x + PIXEL, start->pos.y });
            next3->front = next4;
            buildPathNetwork(next4, movement, gameMap);
        }
    }
    if (movement == "DOWN") {
        if (gameMap->isPath(start->pos.x, start->pos.y + PIXEL)) {
            pathFound = true;
            PathNode* next = new PathNode({ start->pos.x, start->pos.y + 1 });
            start->right = next;
            PathNode* next2 = new PathNode({ start->pos.x, start->pos.y + 2 });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x, start->pos.y + 3 });
            next2->front = next3;
            PathNode* next4 = new PathNode({ start->pos.x, start->pos.y + PIXEL });
            next3->front = next4;
            buildPathNetwork(next4, movement, gameMap);
        }
        if (gameMap->isPath(start->pos.x - PIXEL, start->pos.y)) {
            pathFound = true;
            PathNode* next = new PathNode({ start->pos.x - 1, start->pos.y });
            start->left = next;
            PathNode* next2 = new PathNode({ start->pos.x - 2, start->pos.y });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x - 3, start->pos.y });
            next2->front = next3;
            PathNode* next4 = new PathNode({ start->pos.x - PIXEL, start->pos.y });
            next3->front = next4;
            buildPathNetwork(next4, "LEFT", gameMap);
        }
        if (gameMap->isPath(start->pos.x + PIXEL, start->pos.y)) {
            pathFound = true;
            PathNode* next = new PathNode({ start->pos.x + 1, start->pos.y });
            start->right = next;
            PathNode* next2 = new PathNode({ start->pos.x + 2, start->pos.y });
            next->front = next2;
            PathNode* next3 = new PathNode({ start->pos.x + 3, start->pos.y });
            next2->front = next3;
            PathNode* next4 = new PathNode({ start->pos.x + PIXEL, start->pos.y });
            next3->front = next4;
            buildPathNetwork(next4, "RIGHT", gameMap);
        }
    }
    if (!pathFound) {
        PathNode* end = new PathNode({ start->pos.x - 1, start->pos.y });
        start->front = end;
        PathNode* end2 = new PathNode({ start->pos.x - 2, start->pos.y });
        end->front = end2;
    }
}

bool Tower::upgrade(UpgradeNode* targetNode, Player player)
{
    if ((current->left == targetNode || current->right == targetNode) && player.getMoney() >= targetNode->UpgradeCost)
    {
        damage += targetNode->damageBoost;
        range += targetNode->rangeBoost;
        fireRate -= targetNode->fireRateBoost;
        speed += targetNode->speedBoost;
        current = targetNode;

        rangeCircle.setRadius(range);
        rangeCircle.setOrigin(range, range);

        player.decreaseMoney(targetNode->UpgradeCost);

        cout << "Upgraded to " << current->name << endl;
        return true;
    }
    return false;
}

int findCheapestNode(vector<PathCost*>& list)
{
    int cheapestIndex = -1;
    int cheapestCost = 999999;

    for (int i = 0; i < list.size(); i++)
    {
        if (list[i]->cost < cheapestCost)
        {
            cheapestCost = list[i]->cost;
            cheapestIndex = i;
        }
    }
    return cheapestIndex;
}

// Check if a node was already visited
bool wasVisited(PathNode* node, vector<PathNode*>& visitedList)
{
    for (int i = 0; i < visitedList.size(); i++)
    {
        if (visitedList[i] == node)
        {
            return true;
        }
    }
    return false;
}

// Find parent of a node
PathNode* findParent(PathNode* node, vector<PathCost*>& parentList)
{
    for (int i = 0; i < parentList.size(); i++)
    {
        if (parentList[i]->node == node)
        {
            return parentList[i]->parent;
        }
    }
    return nullptr;
}

stack<PathNode*> dijkstraPath(PathNode* start, Coordinate goal, vector<Tower*>& towers)
{
    // Lists to keep track of things
    vector<PathCost*> toExplore;  // Nodes we need to check
    vector<PathNode*> visited;    // Nodes we already checked
    vector<PathCost*> parentInfo; // Remember how we got to each node

    // Step 1: Start at the beginning
    PathCost* startInfo = new PathCost(start, 0, nullptr);
    toExplore.push_back(startInfo);

    PathCost* parentStart = new PathCost(start, 0, nullptr);
    parentInfo.push_back(parentStart);

    PathNode* goalNode = nullptr;

    // Step 2: Keep exploring until we find the goal or run out of nodes
    while (toExplore.size() > 0)
    {

        // Find the cheapest node to explore next
        int cheapestIndex = findCheapestNode(toExplore);
        PathCost* current = toExplore[cheapestIndex];

        // Remove it from the list
        toExplore.erase(toExplore.begin() + cheapestIndex);

        // Skip if we already visited this node
        if (wasVisited(current->node, visited))
        {
            delete current;
            continue;
        }

        // Mark as visited
        visited.push_back(current->node);

        // Did we reach the goal?
        if (current->node->pos.x == goal.x && current->node->pos.y == goal.y)
        {
            goalNode = current->node;
            delete current;
            break; // Found it! Stop searching
        }

        // Look at all neighbors (front, left, right)
        PathNode* neighbors[3];
        neighbors[0] = current->node->front;
        neighbors[1] = current->node->left;
        neighbors[2] = current->node->right;

        for (int i = 0; i < 3; i++)
        {
            PathNode* neighbor = neighbors[i];

            // Skip if no neighbor in this direction
            if (neighbor == nullptr)
                continue;

            // Skip if already visited
            if (wasVisited(neighbor, visited))
                continue;

            // Calculate cost to move to this neighbor
            int moveCost = neighbor->cost; // Usually 1

            // Add penalty if any tower can see this neighbor
            for (int t = 0; t < towers.size(); t++)
            {
                Tower* tower = towers[t];

                // Calculate distance from neighbor to tower
                float dx = neighbor->pos.x * GRID_SIZE - tower->shape.getPosition().x;
                float dy = neighbor->pos.y * GRID_SIZE - tower->shape.getPosition().y;
                float distance = sqrt(dx * dx + dy * dy);

                // If tower can see this neighbor, add danger cost
                if (distance <= tower->range)
                {
                    moveCost = moveCost + 50; // Dangerous!
                }
            }

            // Total cost = cost to reach current + cost to reach neighbor
            int totalCost = current->cost + moveCost;

            // Add neighbor to explore list
            PathCost* neighborInfo = new PathCost(neighbor, totalCost, current->node);
            toExplore.push_back(neighborInfo);

            // Remember parent (only if not already recorded)
            bool alreadyHasParent = false;
            for (int p = 0; p < parentInfo.size(); p++)
            {
                if (parentInfo[p]->node == neighbor)
                {
                    alreadyHasParent = true;
                    break;
                }
            }

            if (!alreadyHasParent)
            {
                PathCost* parentRecord = new PathCost(neighbor, totalCost, current->node);
                parentInfo.push_back(parentRecord);
            }
        }

        delete current;
    }

    // Step 3: Build the path from goal back to start
    stack<PathNode*> path;

    if (goalNode != nullptr)
    {
        PathNode* current = goalNode;

        while (current != nullptr)
        {
            path.push(current);
            current = findParent(current, parentInfo);
        }
    }

    // Step 4: Clean up memory
    for (int i = 0; i < toExplore.size(); i++)
    {
        delete toExplore[i];
    }

    for (int i = 0; i < parentInfo.size(); i++)
    {
        delete parentInfo[i];
    }

    return path;
}

// ========== Game Manager ==========
class GameManager
{
    Map* gameMap;
    Shop shop;
    Tower dragTower;
    vector<Tower*> towers;
    vector<Enemy*> enemies;
    PathNode* pathHead;
    Clock clock;
    float enemyMoveTimer;
    bool isGameover;

    vector<int> wavePattern;
    int maxEnemies = 0;
    int enemiesSpawned = 0;
    float spawnTimer = 0.0f;
    float spawnDelay = 0.7f;
    float wavePauseTimer = 0.0f;
    float wavePauseDuration = 0.002f;
    bool waveActive = true;
    int currentWave = 1;

    Player player;

public:
    int getWaveCount()
    {
        return currentWave;
    }
    GameManager()
    {
        gameMap = new Map(36, 52);
        enemyMoveTimer = 0;

        // Build the path network
        PathNode* start = new PathNode({ 42, 35 });
        PathNode* second = new PathNode({ 42, 34 });
        start->front = second;
        buildPathNetwork(second, "UP", gameMap);

        pathHead = start;
    }

    bool isGameOver() const
    {
        return isGameover;
    }

    void checkShopDrag(Vector2f coord, bool clicked)
    {
        if (shop.isDragging && !clicked)
        {
            shop.isDragging = false;
            Vector2f coord = dragTower.shape.getPosition();
            Vector2i pos = Vector2i(round(coord.x / GRID_SIZE), round(coord.y / GRID_SIZE));
            if (gameMap->isBuildable(pos.x, pos.y) && player.getMoney() > dragTower.getTowerCost())
            {
                for (auto& t : towers)
                {
                    if (t->pos == pos)
                    {
                        return; // Can't build on another tower
                    }
                }
                player.decreaseMoney(dragTower.getTowerCost());
                towers.push_back(new Tower(pos));
                updateSmartEnemyPaths();
            }
        }
        if (shop.shopTower.getGlobalBounds().contains(coord))
        {
            shop.isDragging = true;
        }
    }

    void checkModeClick(sf::Vector2f mousePos) {
        for (auto& t : towers) {
            if (t->isSelected) {
                // Check if clicking mode button
                FloatRect modeBtnRect(t->pos.x * GRID_SIZE, t->pos.y * GRID_SIZE - 35, 100, 30);
                if (modeBtnRect.contains(mousePos)) {
                    t->showModeOptions = !t->showModeOptions;  // Toggle mode options
                    t->showUpgradeOptions = false;  // Close upgrade options if open
                    return;  // Return early
                }
                if (t->showModeOptions) {
                    string mode = t->checkModeClick(mousePos);
                    if (!mode.empty()) {
                        t->setTargetingMode(mode);
                        t->showModeOptions = false;  // Hide options after selection
                        return;
                    }
                }
            }
        }
    }

    void updateSmartEnemyPaths()
    {
        Coordinate goal = { 0, 10 };

        for (auto& e : enemies)
        {
            if (e->isAlive())
            {
                stack<PathNode*> path = dijkstraPath(e->getCurrentNode(), goal, towers);
                if (!path.empty())
                {
                    e->setSmartPath(path);
                }
            }
        }
    }
    void checkTowerClick(Vector2f mousePos)
    {
        bool clickedOnTower = false;
        bool clickedOnUpgradeUI = false;

        // Check if clicking on any tower
        for (auto& t : towers)
        {
            if (t->shape.getGlobalBounds().contains(mousePos))
            {
                clickedOnTower = true;
            }

            // Check if clicking on upgrade button OR mode button
            if (t->isSelected)
            {
                // Check upgrade button
                FloatRect upgradeBtnRect(t->pos.x * GRID_SIZE, t->pos.y * GRID_SIZE - 70, 100, 30);
                if (upgradeBtnRect.contains(mousePos))
                {
                    clickedOnUpgradeUI = true;
                }

                // Check mode button (ADD THIS)
                FloatRect modeBtnRect(t->pos.x * GRID_SIZE, t->pos.y * GRID_SIZE - 35, 100, 30);
                if (modeBtnRect.contains(mousePos))
                {
                    clickedOnUpgradeUI = true;
                }

                // Check if clicking on upgrade options
                if (t->showUpgradeOptions)
                {
                    UpgradeNode* options[2];
                    int n = t->getAvailableUpgrades(options);
                    for (int i = 0; i < n; i++)
                    {
                        FloatRect btnRect(t->pos.x * GRID_SIZE, t->pos.y * GRID_SIZE + GRID_SIZE + i * 45, 100, 40);
                        if (btnRect.contains(mousePos)) {
                            clickedOnUpgradeUI = true;
                        }
                    }
                }

                // Check if clicking on mode options (ADD THIS)
                if (t->showModeOptions)
                {
                    FloatRect firstBtnRect(t->pos.x * GRID_SIZE - 25, t->pos.y * GRID_SIZE + GRID_SIZE, 150, 30);
                    FloatRect strongBtnRect(t->pos.x * GRID_SIZE - 25, t->pos.y * GRID_SIZE + GRID_SIZE + 35, 150, 30);
                    FloatRect fastBtnRect(t->pos.x * GRID_SIZE - 25, t->pos.y * GRID_SIZE + GRID_SIZE + 70, 150, 30);

                    if (firstBtnRect.contains(mousePos) ||
                        strongBtnRect.contains(mousePos) ||
                        fastBtnRect.contains(mousePos)) {
                        clickedOnUpgradeUI = true;
                    }
                }
            }
        }

        // If clicking outside towers and upgrade UI, deselect everything
        if (!clickedOnTower && !clickedOnUpgradeUI)
        {
            for (auto& t : towers)
            {
                t->isSelected = false;
                t->showUpgradeOptions = false;
                t->showModeOptions = false; // Also reset mode options
            }
        }
        // If clicking on a tower, select it
        else if (clickedOnTower && !clickedOnUpgradeUI)
        {
            // First deselect all
            for (auto& t : towers)
            {
                t->isSelected = false;
                t->showUpgradeOptions = false;
                t->showModeOptions = false; // Also reset mode options
            }

            // Then select the clicked tower
            for (auto& t : towers)
            {
                if (t->shape.getGlobalBounds().contains(mousePos))
                {
                    t->isSelected = true;
                    break;
                }
            }
        }
    }

    void checkUpgradeClick(Vector2f mousePos)
    {
        for (auto& t : towers)
        {
            if (t->isSelected)
            {
                FloatRect upgradeBtnRect(t->pos.x * GRID_SIZE, t->pos.y * GRID_SIZE - 70, 100, 30);
                if (upgradeBtnRect.contains(mousePos)) {
                    t->showUpgradeOptions = !t->showUpgradeOptions;  // Toggle upgrade options
                    t->showModeOptions = false;  // Close mode options if open
                    return;  // Return early
                }
                if (t->showUpgradeOptions)
                {
                    UpgradeNode* options[2];
                    int n = t->getAvailableUpgrades(options);

                    for (int i = 0; i < n; i++)
                    {
                        FloatRect btnRect(t->pos.x * GRID_SIZE, t->pos.y * GRID_SIZE + GRID_SIZE + i * 45, 100, 40);
                        if (btnRect.contains(mousePos))
                        {
                            t->upgrade(options[i], player);
                            t->showUpgradeOptions = false; // Hide options after upgrading
                        }
                    }
                }
            }
        }
    }

    void update(Vector2f mousePos, float dt)
    {

        vector<Enemy*> reachedEnd;

        enemies.erase(
            remove_if(enemies.begin(), enemies.end(),
                [&reachedEnd](Enemy*& e)
                {
                    if (e->reachedEndOfPath() && e->isAlive())
                    {
                        reachedEnd.push_back(e);
                        return true;
                    }
                    return !e->isAlive();
                }),
            enemies.end());

        for (auto& e : reachedEnd)
        {
            player.updateHealth(*e);
            delete e;
        }

        if (player.getHealth() <= 0)
        {
            isGameover = true;
            player.setHealth(0);
            return;  // stop all further game updates
        }

        if (!isGameover)
        {
            for (auto& e : enemies)
                e->update(dt);
        }

        // -------- SPAWN LOGIC with CHECKS --------
        if (waveActive && maxEnemies > 0 && enemiesSpawned < maxEnemies)
        {
            spawnTimer += dt;
            while (spawnTimer >= spawnDelay && enemiesSpawned < maxEnemies)
            {

                if (enemiesSpawned < wavePattern.size())
                {
                    int type = wavePattern[enemiesSpawned];

                    if (type == 0)
                        enemies.push_back(new RedEnemy(pathHead, true));
                    else if (type == 1)
                        enemies.push_back(new BlueEnemy(pathHead, true));
                    else if (type == 2)
                        enemies.push_back(new GreenEnemy(pathHead, true));

                    updateSmartEnemyPaths();
                    enemiesSpawned++;
                }
                spawnTimer -= spawnDelay;
            }
        }

        // -------- WAVE CLEARED CHECK --------
        if (waveActive && enemiesSpawned == maxEnemies && enemies.empty())
        {
            waveActive = false;
            wavePauseTimer = 0.0f;
        }

        // -------- PAUSE TIMER & NEXT WAVE --------
        if (!waveActive)
        {
            wavePauseTimer += dt;
            if (wavePauseTimer >= wavePauseDuration)
            {
                startNextWave();
            }
        }

        // Update towers
        for (auto& t : towers)
            t->update(dt, enemies, player);

        // Handle dragging
        if (shop.isDragging)
        {
            dragTower.shape.setPosition(mousePos.x - dragTower.shape.getRadius(),
                mousePos.y - dragTower.shape.getRadius());
        }
    }

    void startNextWave()
    {

        if (currentWave > 5)
        {

            waveActive = false;
            return;
        }

        wavePattern.clear();

        for (int i = 0; i < 7; ++i)
            wavePattern.push_back(0);

        if (currentWave == 2)
        {
            for (int i = 0; i < 5; ++i)
                wavePattern.push_back(1);
        }

        if (currentWave == 3)
        {
            // for (int i = 0; i < 5; ++i)
            //   wavePattern.push_back(0);
            for (int i = 0; i < 5; ++i)
                wavePattern.push_back(1);
            for (int i = 0; i < 10; ++i)
                wavePattern.push_back(2);
        }

        if (currentWave >= 4)
        {
            // for (int i = 0; i < 5; ++i)
            //   wavePattern.push_back(0);
            for (int i = 0; i < 10; ++i)
                wavePattern.push_back(1);
            for (int i = 0; i < 10; ++i)
                wavePattern.push_back(2);
        }

        maxEnemies = static_cast<int>(wavePattern.size());
        enemiesSpawned = 0;
        spawnTimer = 0.0f;
        waveActive = true;
        wavePauseTimer = 0.0f;
        player.increaseMoney(currentWave);
        currentWave++; // increment wave for the next start
    }

    void draw(RenderWindow& window)
    {
        gameMap->draw(window);
        shop.draw(window);
        for (auto& t : towers)
            t->draw(window);
        for (auto& e : enemies)
            e->draw(window);
        if (shop.isDragging)
        {
            dragTower.draw(window);
        }
        player.draw(window);

        static Font font;
        if (font.loadFromFile("C:/Users/Dell/OneDrive/Desktop/Hatim/DSA/DS_Proj/Clash_Regular.otf")) {
            Text waveText;
            waveText.setFont(font);
            waveText.setString("Wave: " + to_string(currentWave));
            waveText.setCharacterSize(24);
            waveText.setFillColor(Color::White);
            waveText.setStyle(Text::Bold);
            float shopX = 26 * GRID_SIZE;  // Shop's left edge
            float textWidth = waveText.getLocalBounds().width;

            waveText.setPosition(shopX - textWidth + 500, 5);

            window.draw(waveText);
        }

        if (isGameover)
        {
            RectangleShape overlay;
            overlay.setSize(Vector2f(window.getSize()));
            overlay.setFillColor(Color(0, 0, 0, 150));
            window.draw(overlay);

            static Font font;
            font.loadFromFile("C:/Users/Dell/OneDrive/Desktop/Hatim/DSA/DS_Proj/Clash_Regular.otf");

            Text txt;
            txt.setFont(font);
            txt.setString("GAME OVER!");
            txt.setCharacterSize(120);
            txt.setFillColor(Color::White);
            txt.setOutlineThickness(5);
            txt.setOutlineColor(Color::Black);

            FloatRect bounds = txt.getLocalBounds();
            txt.setOrigin(bounds.width / 2, bounds.height / 2);
            txt.setPosition(window.getSize().x / 2, window.getSize().y / 2);

            window.draw(txt);
        }
    }

    friend class Player;
};

// ========== MAIN ==========
int main()
{
    RenderWindow window(VideoMode(52 * GRID_SIZE + PIXEL * GRID_SIZE, 36 * GRID_SIZE), "Tower Defense Game");
    window.setFramerateLimit(60);

    GameManager game;
    Clock clock;

    while (window.isOpen())
    {
        Event event;
        float dt = clock.restart().asSeconds();
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();

            // Mouse press: begin drag if clicked on the shop item
            if (event.type == Event::MouseButtonPressed &&
                event.mouseButton.button == Mouse::Left)
            {
                Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
                game.checkUpgradeClick(mousePos);
                game.checkModeClick(mousePos);
                game.checkTowerClick(mousePos);
                game.checkShopDrag(mousePos, true);
            }

            // Mouse release: drop tower
            if (event.type == Event::MouseButtonReleased &&
                event.mouseButton.button == Mouse::Left)
            {
                game.checkShopDrag({ 0, 0 }, false);
            }
        }

        game.update(window.mapPixelToCoords(Mouse::getPosition(window)), dt);

        window.clear();
        game.draw(window);
        window.display();
    }

    return 0;
}