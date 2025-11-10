#include <SFML/Graphics.hpp>

int main() {
    // Create the main window
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Setup Test");

    // Create a circle
    sf::CircleShape circle(50);
    circle.setFillColor(sf::Color::Cyan);
    circle.setPosition(100, 100);

    // Run the program as long as the window is open
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            // Close window when requested
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Move the circle slightly to show animation works
        circle.move(0.1f, 0);

        // Clear the screen
        window.clear(sf::Color::Black);

        // Draw the circle
        window.draw(circle);

        // Display the contents of the window
        window.display();
    }

    return 0;
}
